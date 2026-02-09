#include "lock_logic.h"
#include "config.h"

/*
 * Initializes the lock context to a safe default state by
 * setting the system to locked, clearing any recorded wrong attempts,
 * and ensuring that no lockout is active when the program starts.
 */
void lock_init(lock_ctx_t *ctx)
{
    if (!ctx) return;

    ctx->state = LOCK_STATE_LOCKED;
    ctx->wrong_attempts = 0;
    ctx->lockout_active = false;
    ctx->lockout_end_time = 0;
}

/*
 * Checks whether a submitted code matches any of the
 * predefined valid passwords by masking the input to the switch width
 * and comparing it against the stored password list.
 */
bool lock_is_code_valid(uint16_t code)
{
    code &= SWITCH_MASK;

    for (unsigned i = 0; i < VALID_PASSWORD_COUNT; i++) {
        if (code == (VALID_PASSWORDS[i] & SWITCH_MASK)) {
            return true;
        }
    }
    return false;
}

/*
 * Processes a password submission event. If the system is
 * currently locked out, the submission is ignored. Otherwise, the code
 * is validated and either unlocks the system or increments the wrong
 * attempt counter. When the maximum number of wrong attempts is reached,
 * a timed lockout is activated.
 */
void lock_handle_submit(lock_ctx_t *ctx, uint16_t code, time_t now)
{
    if (!ctx) return;

    if (ctx->lockout_active) {
        /* Ignore submissions during lockout */
        return;
    }

    if (lock_is_code_valid(code)) {
        ctx->state = LOCK_STATE_UNLOCKED;
        ctx->wrong_attempts = 0;
    } else {
        ctx->state = LOCK_STATE_LOCKED;
        ctx->wrong_attempts++;

        if (ctx->wrong_attempts >= MAX_WRONG_ATTEMPTS) {
            ctx->lockout_active = true;
            ctx->lockout_end_time = now + LOCKOUT_SECONDS;
        }
    }
}

/*
 * Resets the wrong-attempt counter and returns the system
 * to a locked state.
 */
void lock_handle_clear(lock_ctx_t *ctx)
{
    if (!ctx) return;

    ctx->wrong_attempts = 0;
    ctx->state = LOCK_STATE_LOCKED;
}

/*
 * Updates the lockout timer by comparing the current time
 * against the stored lockout expiration time. If the lockout has expired,
 * normal operation is restored and the attempt counter is reset.
 */
bool lock_update_lockout(lock_ctx_t *ctx, time_t now, int *seconds_remaining)
{
    if (!ctx) return false;

    if (!ctx->lockout_active) {
        if (seconds_remaining) *seconds_remaining = 0;
        return false;
    }

    long remaining = (long)(ctx->lockout_end_time - now);

    if (remaining <= 0) {
        ctx->lockout_active = false;
        ctx->wrong_attempts = 0;
        if (seconds_remaining) *seconds_remaining = 0;
        return false;
    }

    if (seconds_remaining) *seconds_remaining = (int)remaining;
    return true;
}
