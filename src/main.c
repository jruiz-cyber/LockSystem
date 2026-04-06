#include <stdio.h>
#include <unistd.h>

#include "config.h"
#include "hw_io.h"
#include "lock_logic.h"

/* Print the readable key name that matches the one-hot button code. */
static void print_button_name(unsigned char code)
{
    switch (code) {
        case BUTTON_KEY0: printf("KEY0"); break;
        case BUTTON_KEY1: printf("KEY1"); break;
        case BUTTON_KEY2: printf("KEY2"); break;
        case BUTTON_KEY3: printf("KEY3"); break;
        default:          printf("UNKNOWN"); break;
    }
}

int main(void)
{
    /* State stores the current lock status, entered buttons, and attempts used. */
    LockState state;
    /* prev_sw1 is used to detect a new press on SW1 instead of repeated triggers. */
    int prev_sw1 = 0;

    /* Start the lock state in its default locked condition. */
    lock_init(&state);

    /* Map the board registers so the HPS can read switches/keys and write LEDs. */
    if (hw_init() != 0)
        return 1;

    /* Turn off all LEDs at startup. */
    hw_write_leds(0);

    printf("=== Digital Lock System (No PIO Version) ===\n");
    printf("HPS reads raw board registers directly.\n");
    printf("SW0 = enable entry\n");
    printf("SW1 = clear entry or relock after unlock\n");

    while (1) {
        /* Read the switch register and separate the two switch meanings. */
        uint32_t sw = hw_read_switches();
        int sw0_enable = (sw & 0x1) ? 1 : 0;
        int sw1 = (sw & 0x2) ? 1 : 0;

        /* A rising edge on SW1 either relocks the system or clears the current entry. */
        if (sw1 && !prev_sw1) {
            if (state.unlocked) {
                lock_relock(&state);
                hw_set_led0(0);
                printf("System relocked.\n");
            } else {
                lock_clear_entry(&state);
                printf("Current entry cleared.\n");
            }
        }
        /* Save the current SW1 state for the next loop pass. */
        prev_sw1 = sw1;

        /* When locked out, LED1 stays on and the program counts down the delay. */
        if (state.lockout_active) {
            hw_set_led0(0);
            hw_set_led1(1);

            printf("Too many wrong attempts. Lockout started for %d seconds.\n", LOCKOUT_SECONDS);
            while (state.lockout_active) {
                int remaining;
                /* Print the remaining lockout time once per second. */
                for (remaining = LOCKOUT_SECONDS; remaining > 0; --remaining) {
                    printf("LOCKED OUT: %d seconds remaining\n", remaining);
                    sleep(1);
                }
                state.lockout_active = 0;
            }

            /* After the lockout ends, reset the attempts counter and clear LED1. */
            state.attempts_used = 0;
            hw_set_led1(0);
            printf("Lockout finished. You have three tries again.\n");
            continue;
        }

        /* Only accept button input when the system is locked and SW0 enables entry. */
        if (!state.unlocked && sw0_enable) {
            /* Read only a new key press */
            uint8_t button = hw_get_new_button_press();

            if (button != 0) {
                printf("Button entered: ");
                print_button_name(button);
                printf("\n");

                /* Add the button to the current sequence. */
                if (lock_add_button(&state, button)) {
                    /* When four buttons have been entered, compare against the password. */
                    if (lock_check_password(&state)) {
                        lock_unlock(&state);
                        hw_set_led0(1);
                        hw_set_led1(0);
                        printf("Unlocked\n");
                    } else {
                        state.attempts_used++;
                        printf("Wrong password. Attempts used: %d/%d\n",
                               state.attempts_used, MAX_ATTEMPTS);
                        /* Clear the wrong entry so the next attempt starts fresh. */
                        lock_clear_entry(&state);

                        /* Enter lockout once the maximum number of tries is reached. */
                        if (state.attempts_used >= MAX_ATTEMPTS)
                            lock_start_lockout(&state);
                    }
                } else {
                    /* Show progress while fewer than four buttons have been entered. */
                    printf("Entry count: %d/%d\n", state.entry_count, PASSWORD_LENGTH);
                }
            }
        }

        /* Small delay to reduce CPU usage while polling the hardware. */
        usleep(20000);
    }

    hw_cleanup();
    return 0;
}
