#ifndef LOCK_LOGIC_H
#define LOCK_LOGIC_H

#include <stdint.h>

/* Holds the current entered sequence and the overall lock status. */
typedef struct
{
    uint8_t entry[4];
    int entry_count;
    int attempts_used;
    int unlocked;
    int lockout_active;
} LockState;

/* Core lock-state helper functions used by main.c. */
void lock_init(LockState *state);
void lock_clear_entry(LockState *state);
int  lock_add_button(LockState *state, uint8_t button_code);
int  lock_check_password(const LockState *state);
void lock_start_lockout(LockState *state);
void lock_unlock(LockState *state);
void lock_relock(LockState *state);

#endif
