#include "config.h"
#include "hw_io.h"
#include "lock_logic.h"

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static volatile sig_atomic_t keep_running = 1;

static void handle_signal(int sig)
{
    (void)sig;
    keep_running = 0;
}

static void reset_entry_buffer(uint8_t sequence[PASSWORD_LENGTH], int *index, time_t *last_press_time)
{
    memset(sequence, 0, PASSWORD_LENGTH);
    *index = 0;
    *last_press_time = 0;
}

static void print_button_label(uint8_t code)
{
    switch (code) {
        case BUTTON0_CODE:
            printf("Button 0 pressed -> 0001\n");
            break;
        case BUTTON1_CODE:
            printf("Button 1 pressed -> 0010\n");
            break;
        case BUTTON2_CODE:
            printf("Button 2 pressed -> 0100\n");
            break;
        case BUTTON3_CODE:
            printf("Button 3 pressed -> 1000\n");
            break;
        default:
            break;
    }
}

static void print_code_binary(uint8_t code)
{
    for (int bit = 3; bit >= 0; bit--) {
        putchar((code & (1u << bit)) ? '1' : '0');
    }
}

static void print_sequence(const uint8_t sequence[PASSWORD_LENGTH])
{
    printf("Sequence entered: ");
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
        print_code_binary(sequence[i]);
        if (i < PASSWORD_LENGTH - 1) {
            printf(" ");
        }
    }
    printf("\n");
}

int main(void)
{
    hw_io_t hw = { .fd = -1, .lw_virtual_base = NULL, .key_ptr = NULL, .sw_ptr = NULL, .ledr_ptr = NULL, .previous_keys = 0xF };
    lock_ctx_t ctx;
    uint8_t entered_sequence[PASSWORD_LENGTH] = {0};
    int entry_index = 0;
    time_t last_press_time = 0;
    bool last_lockout_state = false;
    bool last_unlocked_state = false;
    bool last_enable_state = false;
    bool last_clear_state = false;

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    if (!hw_init(&hw)) {
        fprintf(stderr, "Failed to initialize HPS-to-FPGA IO. Run on the DE10 board with root privileges.\n");
        return 1;
    }

    lock_init(&ctx);

    printf("=== Digital Lock (HPS hardware version) ===\n");
    printf("SW0 = enable entry\n");
    printf("SW1 = clear current partial entry\n");
    printf("KEY0 -> 0001, KEY1 -> 0010, KEY2 -> 0100, KEY3 -> 1000\n");
    printf("Press Ctrl+C to exit.\n\n");

    while (keep_running) {
        time_t now = time(NULL);
        uint32_t switches = hw_read_switches(&hw);
        bool entry_enabled = (switches & SW_ENABLE_MASK) != 0;
        bool clear_active = (switches & SW_CLEAR_MASK) != 0;
        int seconds_remaining = 0;

        if (lock_update_lockout(&ctx, now, &seconds_remaining)) {
            if (!last_lockout_state || seconds_remaining % 5 == 0) {
                printf("[LOCKOUT] Too many wrong attempts. Try again in %d seconds.\n", seconds_remaining);
            }
            hw_set_leds(&hw, 0x200);
            reset_entry_buffer(entered_sequence, &entry_index, &last_press_time);
            last_lockout_state = true;
            hw_sleep_ms(100);
            continue;
        }
        last_lockout_state = false;

        if (ctx.state == LOCK_STATE_UNLOCKED && !last_unlocked_state) {
            printf("[STATE] UNLOCKED\n");
            hw_set_leds(&hw, 0x3FF);
            last_unlocked_state = true;
        } else if (ctx.state == LOCK_STATE_LOCKED && last_unlocked_state) {
            printf("[STATE] LOCKED\n");
            last_unlocked_state = false;
        }

        if (!entry_enabled) {
            if (last_enable_state) {
                printf("[INFO] SW0 disabled. Entry paused and partial input cleared.\n");
            }
            reset_entry_buffer(entered_sequence, &entry_index, &last_press_time);
            if (ctx.state == LOCK_STATE_LOCKED) {
                hw_set_leds(&hw, 0x000);
            }
            last_enable_state = false;
            hw_sleep_ms(25);
            continue;
        }

        if (!last_enable_state) {
            printf("[INFO] SW0 enabled. Ready for button input.\n");
        }
        last_enable_state = true;

        if (clear_active && !last_clear_state) {
            reset_entry_buffer(entered_sequence, &entry_index, &last_press_time);
            if (ctx.state == LOCK_STATE_LOCKED) {
                hw_set_leds(&hw, 0x000);
            }
            printf("[ACTION] SW1 clear pressed. Current entry erased.\n");
        }
        last_clear_state = clear_active;

        if (entry_index > 0 && last_press_time != 0 && difftime(now, last_press_time) >= ENTRY_TIMEOUT_SEC) {
            printf("[TIMEOUT] No button pressed for %d seconds. Counting as a wrong attempt.\n", ENTRY_TIMEOUT_SEC);
            ctx.state = LOCK_STATE_LOCKED;
            ctx.wrong_attempts++;
            if (ctx.wrong_attempts >= MAX_WRONG_ATTEMPTS) {
                ctx.lockout_active = true;
                ctx.lockout_end_time = now + LOCKOUT_SECONDS;
                printf("[LOCKOUT] Maximum wrong attempts reached.\n");
            } else {
                printf("[RESULT] Wrong attempts = %d/%d\n", ctx.wrong_attempts, MAX_WRONG_ATTEMPTS);
            }
            reset_entry_buffer(entered_sequence, &entry_index, &last_press_time);
            hw_set_leds(&hw, 0x000);
            hw_sleep_ms(100);
            continue;
        }

        if (ctx.state == LOCK_STATE_LOCKED) {
            uint8_t button_code = hw_get_button_code(&hw);
            if (button_code != 0) {
                print_button_label(button_code);
                entered_sequence[entry_index] = button_code;
                entry_index++;
                last_press_time = now;
                hw_set_leds(&hw, button_code);
                printf("[INPUT] Stored %d of %d entries. Wrong attempts = %d/%d\n",
                       entry_index, PASSWORD_LENGTH, ctx.wrong_attempts, MAX_WRONG_ATTEMPTS);

                if (entry_index == PASSWORD_LENGTH) {
                    print_sequence(entered_sequence);
                    lock_handle_sequence(&ctx, entered_sequence, now);

                    if (ctx.lockout_active) {
                        printf("[LOCKOUT] Maximum wrong attempts reached.\n");
                        hw_set_leds(&hw, 0x200);
                    } else if (ctx.state == LOCK_STATE_UNLOCKED) {
                        printf("[RESULT] Password correct. System unlocked.\n");
                        hw_set_leds(&hw, 0x3FF);
                    } else {
                        printf("[RESULT] Password incorrect. Wrong attempts = %d/%d\n",
                               ctx.wrong_attempts, MAX_WRONG_ATTEMPTS);
                        hw_set_leds(&hw, 0x000);
                    }

                    reset_entry_buffer(entered_sequence, &entry_index, &last_press_time);
                }

                hw_sleep_ms(180);
            }
        }

        hw_sleep_ms(20);
    }

    hw_set_leds(&hw, 0x000);
    hw_close(&hw);
    printf("Program ended.\n");
    return 0;
}
