#include "hw_io.h"
#include "address_map_arm.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <unistd.h>

bool hw_init(hw_io_t *hw)
{
    if (!hw) {
        return false;
    }

    hw->fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (hw->fd == -1) {
        perror("open(/dev/mem)");
        return false;
    }

    hw->lw_virtual_base = mmap(NULL, LW_BRIDGE_SPAN, PROT_READ | PROT_WRITE,
                               MAP_SHARED, hw->fd, LW_BRIDGE_BASE);
    if (hw->lw_virtual_base == MAP_FAILED) {
        perror("mmap");
        close(hw->fd);
        hw->fd = -1;
        return false;
    }

    hw->ledr_ptr = (volatile uint32_t *)((uint8_t *)hw->lw_virtual_base + LEDR_BASE);
    hw->sw_ptr   = (volatile uint32_t *)((uint8_t *)hw->lw_virtual_base + SW_BASE);
    hw->key_ptr  = (volatile uint32_t *)((uint8_t *)hw->lw_virtual_base + KEY_BASE);
    hw->previous_keys = *(hw->key_ptr);

    *(hw->ledr_ptr) = 0;
    return true;
}

void hw_close(hw_io_t *hw)
{
    if (!hw) {
        return;
    }

    if (hw->lw_virtual_base && hw->lw_virtual_base != MAP_FAILED) {
        munmap(hw->lw_virtual_base, LW_BRIDGE_SPAN);
        hw->lw_virtual_base = NULL;
    }

    if (hw->fd >= 0) {
        close(hw->fd);
        hw->fd = -1;
    }
}

uint32_t hw_read_switches(const hw_io_t *hw)
{
    return hw ? *(hw->sw_ptr) : 0;
}

uint32_t hw_read_keys_raw(const hw_io_t *hw)
{
    return hw ? *(hw->key_ptr) : 0xF;
}

uint8_t hw_get_button_code(hw_io_t *hw)
{
    if (!hw) {
        return 0;
    }

    uint32_t current_keys = *(hw->key_ptr);
    uint32_t current_pressed = (~current_keys) & 0xF;
    uint32_t previous_pressed = (~hw->previous_keys) & 0xF;
    uint32_t new_press = current_pressed & (~previous_pressed);

    hw->previous_keys = current_keys;

    if (new_press & 0x1) return 0x1;
    if (new_press & 0x2) return 0x2;
    if (new_press & 0x4) return 0x4;
    if (new_press & 0x8) return 0x8;

    return 0;
}

void hw_set_leds(const hw_io_t *hw, uint32_t value)
{
    if (hw) {
        *(hw->ledr_ptr) = value;
    }
}

void hw_sleep_ms(unsigned int ms)
{
    struct timeval tv;
    tv.tv_sec = ms / 1000U;
    tv.tv_usec = (int)((ms % 1000U) * 1000U);
    select(0, NULL, NULL, NULL, &tv);
}
