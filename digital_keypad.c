#include <xc.h>
#include "digital_keypad.h"

#pragma config WDTE = OFF // Watchdog Timer Enable bit (WDT disabled)

void init_digital_keypad(void) {
    /* Set Keypad Port as input */
    KEYPAD_PORT_DDR = KEYPAD_PORT_DDR | INPUT_LINES;
}

unsigned char read_digital_keypad() {
    static char once = 0;
    static int longpressed = 0;
    static unsigned char pre_key = ALL_RELEASED;
    static unsigned char long_press_flag = 0; // Flag to ensure long press is returned only once

    unsigned char key = KEYPAD_PORT & INPUT_LINES;

    if (key != ALL_RELEASED && !once) {
        // Key press detected for the first time
        once = 1;
        longpressed = 0;         // Reset long press counter
        long_press_flag = 0;     // Reset the flag for new key press
        pre_key = key;
    }

    if (once && key == pre_key) {
        // Key is being held
        longpressed++;
        if (longpressed >= 20 && !long_press_flag) { // Threshold for long press
            long_press_flag = 1; // Set flag to ensure single return for long press
            if (pre_key == SW4) {
                return LPSW4; // Long press detected for SW4
            } else if (pre_key == SW5) {
                return LPSW5; // Long press detected for SW5
            }    else if(pre_key == SW6){
                    return LPSW6;
            }
        }
    } else if (key == ALL_RELEASED && once) {
        // Key released
        if (longpressed < 20 && !long_press_flag) {
            // Short press detected
            once = 0; // Reset for new key detection
            return pre_key;
        }
        // Reset after long press or short press
        once = 0;
        longpressed = 0;
        long_press_flag = 0;
    }

    return ALL_RELEASED;
}



