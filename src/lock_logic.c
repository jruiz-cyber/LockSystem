#include "config.h"
#include "lock_logic.h"

void lock_init(LockState *state)
{
    /* Reset the stored entry, attempts, and status flags at startup. */
    state->entry_count = 0;
    state->attempts_used = 0;
    state->unlocked = 0;
    state->lockout_active = 0;

    /* Clear every entry slot so no old button codes remain. */
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
        state->entry[i] = 0;
    }
}

void lock_clear_entry(LockState *state)
{
    /* Start the current button sequence over from the beginning. */
    state->entry_count = 0;
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
        state->entry[i] = 0;
    }
}

int lock_add_button(LockState *state, uint8_t button_code)
{
    /* Ignore extra input once the full password length has been reached. */
    if (state->entry_count >= PASSWORD_LENGTH) {
        return 0;
    }

    /* Store the new button code in the next available entry position. */
    state->entry[state->entry_count] = button_code;
    state->entry_count++;

    /* Return 1 only when the full sequence has been entered. */
    return (state->entry_count == PASSWORD_LENGTH);
}

int lock_check_password(const LockState *state)
{
    /* Compare each entered button against the expected password pattern. */
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
        if (state->entry[i] != PASSWORD[i]) {
            return 0;
        }
    }
    return 1;
}

void lock_start_lockout(LockState *state)
{
    /* Enable lockout, force the system locked, and clear partial input. */
    state->lockout_active = 1;
    state->unlocked = 0;
    lock_clear_entry(state);
}

void lock_unlock(LockState *state)
{
    /* Mark the system unlocked and reset counters for the next use. */
    state->unlocked = 1;
    state->attempts_used = 0;
    state->lockout_active = 0;
    lock_clear_entry(state);
}

void lock_relock(LockState *state)
{
    /* Return the system to the locked state and clear any current entry. */
    state->unlocked = 0;
    lock_clear_entry(state);
}
