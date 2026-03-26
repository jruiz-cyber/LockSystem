#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

#define MAX_WRONG_ATTEMPTS   3
#define LOCKOUT_SECONDS      30
#define PASSWORD_LENGTH      4
#define ENTRY_TIMEOUT_SEC    3

#define BUTTON0_CODE         0x1
#define BUTTON1_CODE         0x2
#define BUTTON2_CODE         0x4
#define BUTTON3_CODE         0x8

#define SW_ENABLE_MASK       0x1   /* SW0 */
#define SW_CLEAR_MASK        0x2   /* SW1 */

/*
 * Valid passwords are stored as 4 one-hot button entries.
 * KEY0 -> 0001, KEY1 -> 0010, KEY2 -> 0100, KEY3 -> 1000
 */
static const uint8_t VALID_PASSWORDS[][PASSWORD_LENGTH] = {
    {BUTTON0_CODE, BUTTON1_CODE, BUTTON2_CODE, BUTTON3_CODE},
    {BUTTON3_CODE, BUTTON2_CODE, BUTTON1_CODE, BUTTON0_CODE},
    {BUTTON0_CODE, BUTTON2_CODE, BUTTON0_CODE, BUTTON3_CODE}
};

#define VALID_PASSWORD_COUNT (sizeof(VALID_PASSWORDS) / sizeof(VALID_PASSWORDS[0]))

#endif
