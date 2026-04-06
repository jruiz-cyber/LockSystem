#ifndef HW_IO_H
#define HW_IO_H

#include <stdint.h>

/* Set up direct HPS access to the standard DE10 hardware registers. */
int hw_init(void);
/* Release mapped hardware access before the program exits. */
void hw_cleanup(void);

/* Read the current switch bits from the board. */
uint32_t hw_read_switches(void);
/* Read the raw key register value. */
uint32_t hw_read_keys_raw(void);
/* Return only a newly detected button press as a one-hot code. */
uint8_t  hw_get_new_button_press(void);

/* Control individual LEDs or write a full LED value. */
void hw_set_led0(int on);
void hw_set_led1(int on);
void hw_write_leds(uint32_t value);

#endif
