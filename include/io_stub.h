#ifndef IO_STUB_H
#define IO_STUB_H

#include <stdbool.h>
#include <stdint.h>


typedef enum {
    INPUT_NONE = 0,
    INPUT_ENTER,
    INPUT_CLEAR,
    INPUT_QUIT
} input_event_t;


input_event_t io_stub_read_event(uint16_t *out_code);

// Terminal output
void io_stub_show_status_locked(int wrong_attempts);
void io_stub_show_status_unlocked(void);
void io_stub_show_wrong_attempt(void);
void io_stub_show_lockout(int seconds_remaining);
void io_stub_show_cleared(void);

#endif
