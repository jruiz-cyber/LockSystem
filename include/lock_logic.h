#ifndef LOCK_LOGIC_H
#define LOCK_LOGIC_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "config.h"

typedef enum {
    LOCK_STATE_LOCKED = 0,
    LOCK_STATE_UNLOCKED
} lock_state_t;

typedef struct {
    lock_state_t state;
    int wrong_attempts;
    bool lockout_active;
    time_t lockout_end_time;
} lock_ctx_t;

void lock_init(lock_ctx_t *ctx);
bool lock_is_sequence_valid(const uint8_t sequence[PASSWORD_LENGTH]);
void lock_handle_sequence(lock_ctx_t *ctx, const uint8_t sequence[PASSWORD_LENGTH], time_t now);
bool lock_update_lockout(lock_ctx_t *ctx, time_t now, int *seconds_remaining);

#endif
