#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "address_map_arm.h"
#include "hw_io.h"

/* File descriptor for /dev/mem so the HPS can access board registers. */
static int fd = -1;
/* Base virtual address returned by mmap for the lightweight bridge. */
static void *lw_virtual = NULL;

/* These pointers hold the mapped addresses for LEDs, switches, and keys. */
static volatile uint32_t *ledr_ptr = NULL;
static volatile uint32_t *sw_ptr   = NULL;
static volatile uint32_t *key_ptr  = NULL;

/* led_state keeps a software copy of the LED register value. */
static uint32_t led_state = 0;
/* Used to detect only newly pressed keys instead of held keys. */
static uint32_t prev_keys_pressed = 0;

int hw_init(void)
{
    /* Open physical memory so the program can map the DE10 board registers. */
    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd < 0) {
        perror("open(/dev/mem)");
        return -1;
    }

    /* Map the lightweight bridge that contains the standard board addresses. */
    lw_virtual = mmap(NULL, LW_BRIDGE_SPAN, PROT_READ | PROT_WRITE, MAP_SHARED, fd, LW_BRIDGE_BASE);
    if (lw_virtual == MAP_FAILED) {
        perror("mmap");
        close(fd);
        fd = -1;
        return -1;
    }

    /* Offset each pointer to the correct LED, switch, and key register. */
    ledr_ptr = (volatile uint32_t *)((char *)lw_virtual + LEDR_BASE);
    sw_ptr   = (volatile uint32_t *)((char *)lw_virtual + SW_BASE);
    key_ptr  = (volatile uint32_t *)((char *)lw_virtual + KEY_BASE);

    /* Start with LEDs off and no remembered key presses. */
    *ledr_ptr = 0;
    led_state = 0;
    prev_keys_pressed = 0;
    return 0;
}

void hw_cleanup(void)
{
    /* Turn LEDs off before leaving the program. */
    if (ledr_ptr)
        *ledr_ptr = 0;

    /* Unmap the lightweight bridge memory region. */
    if (lw_virtual && lw_virtual != MAP_FAILED) {
        munmap(lw_virtual, LW_BRIDGE_SPAN);
        lw_virtual = NULL;
    }

    /* Close /dev/mem after the mapped access is no longer needed. */
    if (fd >= 0) {
        close(fd);
        fd = -1;
    }
}

uint32_t hw_read_switches(void)
{
    /* Read only SW0 and SW1 from the switch register. */
    return (*sw_ptr) & 0x3;
}

uint32_t hw_read_keys_raw(void)
{
    /* Read the four pushbuttons as they are in the key register. */
    return (*key_ptr) & 0xF;
}

static uint32_t hw_read_keys_pressed(void)
{
    /* Keys are active-low, so invert the raw bits to get pressed = 1. */
    return (~hw_read_keys_raw()) & 0xF;
}

uint8_t hw_get_new_button_press(void)
{
    uint32_t current_pressed = hw_read_keys_pressed();
    /* new_press keeps only buttons that were not pressed in the previous read. */
    uint32_t new_press = current_pressed & ~prev_keys_pressed;

    /* Save the current key state for edge detection on the next call. */
    prev_keys_pressed = current_pressed;

    /* Return the corresponding button code for the newly detected button press. */
    if (new_press & 0x1) return 0x1;
    if (new_press & 0x2) return 0x2;
    if (new_press & 0x4) return 0x4;
    if (new_press & 0x8) return 0x8;
    return 0;
}

void hw_write_leds(uint32_t value)
{
    /* Limit the value to the ten board LEDs, then write it to the LED register. */
    led_state = value & 0x3FF;
    *ledr_ptr = led_state;
}

void hw_set_led0(int on)
{
    /* Set or clear LED0 while leaving the other LED bits unchanged. */
    if (on) led_state |= 0x1;
    else    led_state &= ~0x1;
    *ledr_ptr = led_state;
}

void hw_set_led1(int on)
{
    /* Set or clear LED1 while leaving the other LED bits unchanged. */
    if (on) led_state |= 0x2;
    else    led_state &= ~0x2;
    *ledr_ptr = led_state;
}
