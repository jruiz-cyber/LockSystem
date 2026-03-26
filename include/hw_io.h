#ifndef HW_IO_H
#define HW_IO_H

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    int fd;
    void *lw_virtual_base;
    volatile uint32_t *key_ptr;
    volatile uint32_t *sw_ptr;
    volatile uint32_t *ledr_ptr;
    uint32_t previous_keys;
} hw_io_t;

bool hw_init(hw_io_t *hw);
void hw_close(hw_io_t *hw);
uint32_t hw_read_switches(const hw_io_t *hw);
uint32_t hw_read_keys_raw(const hw_io_t *hw);
uint8_t hw_get_button_code(hw_io_t *hw);
void hw_set_leds(const hw_io_t *hw, uint32_t value);
void hw_sleep_ms(unsigned int ms);

#endif
