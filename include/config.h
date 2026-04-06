#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

/* Lock settings used throughout the program. */
#define PASSWORD_LENGTH     4
#define MAX_ATTEMPTS        3
#define LOCKOUT_SECONDS     30

/* Button codes that match the DE10 pushbuttons. */
#define BUTTON_KEY0         0x1
#define BUTTON_KEY1         0x2
#define BUTTON_KEY2         0x4
#define BUTTON_KEY3         0x8

/* Password sequence: KEY0, KEY2, KEY1, KEY3. */
static const uint8_t PASSWORD[PASSWORD_LENGTH] = {
    BUTTON_KEY0,
    BUTTON_KEY2,
    BUTTON_KEY1,
    BUTTON_KEY3
};

#endif
