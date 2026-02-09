#ifndef LOCK_LOGIC_H
#define LOCK_LOGIC_H

#include <stdbool.h>
#include <stdint.h>
#include <time.h>

typedef enum {
    LOCK_STATE_LOCKED = 0,
    LOCK_STATE_UNLOCKED
} lock_state_t;

typedef struct {
    lock_state_t state;
    int wrong_attempts;

    bool lockout_active;
    time_t lockout_end_time; //seconds epoch
} lock_ctx_t;

// Initialize lock
void lock_init(lock_ctx_t *ctx);

// True if correct password was typed in
bool lock_is_code_valid(uint16_t code);

// Pressing enter
void lock_handle_submit(lock_ctx_t *ctx, uint16_t code, time_t now);

// Clear
void lock_handle_clear(lock_ctx_t *ctx);

// Checks whether or not it is still locked
bool lock_update_lockout(lock_ctx_t *ctx, time_t now, int *seconds_remaining);

#endif
