#include "io_stub.h"
#include "config.h"
#include <stdio.h>

/*
 *  - prompt: (e) enter, (c) clear, (q) quit
 *  - if enter: prompt for hex code (like 0x015)
 *  - return matching event
 */
input_event_t io_stub_read_event(uint16_t *out_code)
{
    char choice = 0;

    printf("\nChoose: [e]=ENTER  [c]=CLEAR  [q]=QUIT > ");
    fflush(stdout);

    if (scanf(" %c", &choice) != 1) {
        return INPUT_NONE;
    }

    if (choice == 'q' || choice == 'Q') {
        return INPUT_QUIT;
    }

    if (choice == 'c' || choice == 'C') {
        return INPUT_CLEAR;
    }

    if (choice == 'e' || choice == 'E') {
        unsigned int code = 0;
        printf("Enter code as hex (example 0x015) > ");
        fflush(stdout);

        if (scanf("%x", &code) != 1) {
            return INPUT_NONE;
        }

        if (out_code) {
            *out_code = (uint16_t)(code & SWITCH_MASK);
        }
        return INPUT_ENTER;
    }

    return INPUT_NONE;
}

void io_stub_show_status_locked(int wrong_attempts)
{
    printf("[STATE] LOCKED | wrong_attempts=%d/%d\n", wrong_attempts, MAX_WRONG_ATTEMPTS);
}

void io_stub_show_status_unlocked(void)
{
    printf("[STATE] UNLOCKED ✅\n");
}

void io_stub_show_wrong_attempt(void)
{
    printf("[RESULT] WRONG CODE ❌\n");
}

void io_stub_show_lockout(int seconds_remaining)
{
    printf("[LOCKOUT] Too many attempts. Try again in %d seconds.\n", seconds_remaining);
}

void io_stub_show_cleared(void)
{
    printf("[ACTION] CLEAR pressed. Attempts reset.\n");
}
