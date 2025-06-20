/*
 * File:   main.c
 * Author: harid
 *
 * Created on 20 December, 2024, 9:41 AM
 */


#include "main.h"

#pragma config WDTE = OFF

static void init_config(void) 
{
    // initialize i2c
    init_i2c(100000);
    // initialize RTC
    init_ds1307();
    // initialize ADC
    init_adc();
    // initialize dkp
    init_digital_keypad();
    // initialize clcd
    init_clcd();
    // init external_eeprom
    init_AT24C04();   
    // initialize timer0
    init_timer2();
    /* Peripheral Interrupt Enable Bit (For Timer2) */
    PEIE = 1;
    /* Enable all the Global Interrupts */
    GIE = 1;
    /* initialize UART*/
    init_uart(9600);
}

void main(void) 
{
    unsigned char control_flag = DASHBOARD_SCREEN, key;
    unsigned char reset_flag = 0, menu_pos;
    unsigned char event[3] = "ON";
    unsigned char speed = 0;
    char *gear[] = {"GN","GR","G1","G2","G3","G4"};
                    //0   1    2    3     4    5
    unsigned char gr = 0;

    init_config();
    log_event(event, speed);
    write_str_external_eeprom(0x00, "1111");    // Initially store the password to EEPROM
    
    while (1) 
    {
        speed = read_adc() / 10.3;      // 0 to 1023 -> 0 - 99
        
        key = read_digital_keypad();    // Read key
        for(unsigned int i = 200; i--; );   // reduce key bouncing
        
        if(key == SW1)      // Dashboard screen function
        {
            strcpy(event, "CO");
            log_event(event, speed);
        }
        else if(key == SW2 && gr < 6)   // gear shift up
        {
            strcpy(event, gear[gr]);
            gr++;
            log_event(event, speed);
        }
        else if(key == SW3 && gr > 0)   // gear shift down
        {
            gr--;
            strcpy(event, gear[gr]);
            log_event(event, speed);
        }
        
        else if((key == SW4 || key == SW5) && control_flag == DASHBOARD_SCREEN)     // Login screen
        {
            control_flag = LOGIN_SCREEN;
            clear_screen();
            __delay_us(500);
            clcd_print("Enter Password", LINE1(1));
            clcd_write(LINE2(5), INST_MODE);
            clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
            __delay_us(100);
            reset_flag = RESET_PASSWORD;
            TMR2ON = 1;
        }
        else if(key == LPSW4 && (control_flag == VIEW_LOG || control_flag == DOWNLOAD_LOG))    // return to main_menu 
        {
            clear_screen();
            control_flag = MAIN_MENU_SCREEN;
            reset_flag = RESET_MENU;
        }
        
        else if(key == LPSW4 && control_flag == MAIN_MENU_SCREEN)   // enter to selected main_menu
        {  
            TMR2ON = 0;
            switch(menu_pos)
            {
                case 0:                             // view log
                    control_flag = VIEW_LOG;
                    reset_flag = RESET_VIEW_LOG;
                    clear_screen();
                    log_event("VL", speed);
                    break;
                case 1:                             // clear log
                    control_flag = CLEAR_LOG;
                    clear_screen();
                    break;
                case 2:                             // download log
                    log_event("DL", speed);
                    clear_screen();
                    control_flag = DOWNLOAD_LOG;
                    break;
                case 3:                             // reset time
                    log_event("CT", speed);
                    clear_screen();
                    control_flag = SET_TIME;
                    reset_flag = RESET_TIME;
                    break;
                case 4:                             // change password
                    log_event("CP", speed);
                    clear_screen();
                    control_flag = CHANGE_PASSWORD;
                    reset_flag = RESET_PASSWORD;
                    clcd_print("Enter New Pass", LINE1(1));
                    clcd_write(LINE2(5), INST_MODE);
                    clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
                    __delay_us(100);
                    break;
            }
        }
        else if(key == LPSW5 && (control_flag == VIEW_LOG || control_flag == MAIN_MENU_SCREEN || control_flag == DOWNLOAD_LOG)) // long press SW5 return to dashboard screen
        {
            clear_screen();
            control_flag = DASHBOARD_SCREEN;
        }
        
        switch(control_flag)        //switch case to select function
        {
            case DASHBOARD_SCREEN:      // dashboard screen
                display_dashboard(event, speed);
                break;
            case LOGIN_SCREEN:          // login screen
                switch(login(key, reset_flag))
                {
                    case RETURN_BACK:
                        control_flag = DASHBOARD_SCREEN;
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        clear_screen();
                        TMR2ON = 0;
                        break;
                    case LOGIN_SUCCESS:
                        control_flag = MAIN_MENU_SCREEN;
                        reset_flag = RESET_MENU;
                        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
                        __delay_us(100);
                        clear_screen();
                        continue;   
                }
                break;
            case MAIN_MENU_SCREEN:      // main_manu screen
                TMR2ON = 1;
                    if((menu_pos = menu_screen(key, reset_flag)) == RETURN_BACK)        // return back when in-activity is present
                    {
                        clear_screen();
                        control_flag = DASHBOARD_SCREEN;
                        TMR2ON = 0;
                    }
                break;
            case VIEW_LOG:              // view the log in lcd
                view_log(key, reset_flag);
                break;  
            case CLEAR_LOG:             // clear the log history
                if(clear_log() == SUCCESS)
                {
                    clear_screen();
                    log_event("CL", speed);
                    control_flag = MAIN_MENU_SCREEN;
                    reset_flag = RESET_MENU;
                    continue;
                }
                break;
            case DOWNLOAD_LOG:          // download log to pc
                if(download_log() == SUCCESS)
                {
                    clear_screen();
                    control_flag = MAIN_MENU_SCREEN;
                    reset_flag = RESET_MENU;
                    continue;
                }
                break;
            case SET_TIME:              // reset the time
                if(change_time(key, reset_flag) == SUCCESS)
                {
                    clear_screen();
                    control_flag = MAIN_MENU_SCREEN;
                    reset_flag = RESET_MENU;
                    continue;
                }
                break;
            case CHANGE_PASSWORD:       // change password and store to EEPROM
                    if(change_password(key, reset_flag) == SUCCESS)
                    {
                        clear_screen();
                        control_flag = MAIN_MENU_SCREEN;
                        reset_flag = RESET_MENU;
                        continue;
                    }
                    break;
        }
        reset_flag = RESET_NOTHING;     // reset nothing flag
    }
    return;
}
