#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// Wrong attempts before lockout
#define MAX_WRONG_ATTEMPTS   (3)

// Lockout duration 
#define LOCKOUT_SECONDS      (30)

// Switches
#define SWITCH_MASK          ((uint16_t)0x03FF)

//Valid Passwords
static const uint16_t VALID_PASSWORDS[] = {
    0x015,  
    0x1A3,  
    0x2D0   
};

#define VALID_PASSWORD_COUNT (sizeof(VALID_PASSWORDS)/sizeof(VALID_PASSWORDS[0]))

#endif
