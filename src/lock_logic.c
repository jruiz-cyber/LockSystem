#include "lock_logic.h"

void lock_init(lock_ctx_t *ctx)
{
    if (!ctx) {
        return;
    }

    ctx->state = LOCK_STATE_LOCKED;
    ctx->wrong_attempts = 0;
    ctx->lockout_active = false;
    ctx->lockout_end_time = 0;
}

bool lock_is_sequence_valid(const uint8_t sequence[PASSWORD_LENGTH])
{
    if (!sequence) {
        return false;
    }

    for (unsigned int i = 0; i < VALID_PASSWORD_COUNT; i++) {
        bool match = true;
        for (int j = 0; j < PASSWORD_LENGTH; j++) {
            if (sequence[j] != VALID_PASSWORDS[i][j]) {
                match = false;
                break;
            }
        }
        if (match) {
            return true;
        }
    }

    return false;
}

void lock_handle_sequence(lock_ctx_t *ctx, const uint8_t sequence[PASSWORD_LENGTH], time_t now)
{
    if (!ctx || !sequence || ctx->lockout_active) {
        return;
    }

    if (lock_is_sequence_valid(sequence)) {
        ctx->state = LOCK_STATE_UNLOCKED;
        ctx->wrong_attempts = 0;
        return;
    }

    ctx->state = LOCK_STATE_LOCKED;
    ctx->wrong_attempts++;

    if (ctx->wrong_attempts >= MAX_WRONG_ATTEMPTS) {
        ctx->lockout_active = true;
        ctx->lockout_end_time = now + LOCKOUT_SECONDS;
    }
}

bool lock_update_lockout(lock_ctx_t *ctx, time_t now, int *seconds_remaining)
{
    if (!ctx) {
        return false;
    }

    if (!ctx->lockout_active) {
        if (seconds_remaining) {
            *seconds_remaining = 0;
        }
        return false;
    }

    long remaining = (long)(ctx->lockout_end_time - now);
    if (remaining <= 0) {
        ctx->lockout_active = false;
        ctx->wrong_attempts = 0;
        if (seconds_remaining) {
            *seconds_remaining = 0;
        }
        return false;
    }

    if (seconds_remaining) {
        *seconds_remaining = (int)remaining;
    }
    return true;
}
