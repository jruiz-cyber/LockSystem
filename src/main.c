#include "lock_logic.h"
#include "io_stub.h"
#include <stdio.h>
#include <time.h>
#include <unistd.h>


int main(void)
{
    lock_ctx_t ctx;
    lock_init(&ctx);

    printf("=== Digital Lock (HPS C Program - Stub IO) ===\n");
    printf("Passwords are defined in include/config.h\n");

    while (1) {
        time_t now = time(NULL);

        // Displays the countdown of the lokcout and blocks input 
        int remaining = 0;
        if (lock_update_lockout(&ctx, now, &remaining)) {
            io_stub_show_lockout(remaining);
            sleep(1);
            continue;
        }

        // Current lock state before accepting input 
        if (ctx.state == LOCK_STATE_UNLOCKED) {
            io_stub_show_status_unlocked();
        } else {
            io_stub_show_status_locked(ctx.wrong_attempts);
        }

        uint16_t code = 0;
        input_event_t event = io_stub_read_event(&code);

        if (event == INPUT_QUIT) {
            printf("Exiting program.\n");
            break;
        }

        if (event == INPUT_CLEAR) {
            lock_handle_clear(&ctx);
            io_stub_show_cleared();
            continue;
        }

        if (event == INPUT_ENTER) {
            now = time(NULL);

            bool was_locked_out = ctx.lockout_active;
            lock_handle_submit(&ctx, code, now);

            if (ctx.lockout_active && !was_locked_out) {
                lock_update_lockout(&ctx, time(NULL), &remaining);
                io_stub_show_lockout(remaining);
                continue;
            }

            if (ctx.state == LOCK_STATE_UNLOCKED) {
                io_stub_show_status_unlocked();
            } else {
                io_stub_show_wrong_attempt();
            }
        }
    }

    return 0;
}
