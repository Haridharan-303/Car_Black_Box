/* Car black box definition */
#include "main.h"
/* display dashboard function */

unsigned char clock_reg[3]; // HH MM SS
char time[7];  // "HH:MM:SS"
char log[11];  // HHMMSSEVSP
char log_pos = 0, sec;
unsigned char return_time;
char *menu[] = {"View log","Clear log","Download log","Reset Time","Change pass"};
static char menu_pos = 0;
char new_password[4];
char re_password[4];

/* Get time from RTC */
static void get_time()
{
    clock_reg[0] = read_ds1307(HOUR_ADDR); // HH -> BCD 
    clock_reg[1] = read_ds1307(MIN_ADDR); // MM -> BCD 
    clock_reg[2] = read_ds1307(SEC_ADDR); // SS -> BCD 
    
    // HH -> bcd to ascii
    time[0] = ((clock_reg[0] >> 4) & 0x03) + '0';
    time[1] = (clock_reg[0] & 0x0F) + '0';
    // MM 
    time[2] = ((clock_reg[1] >> 4) & 0x07) + '0';
    time[3] = (clock_reg[1] & 0x0F) + '0';
    // SS
    time[4] = ((clock_reg[2] >> 4) & 0x07) + '0';
    time[5] = (clock_reg[2] & 0x0F) + '0';
    time[6] = '\0';
}

/* Display time to dashboard screen */
void display_time()
{
    get_time();
    // HH:MM:SS
    clcd_putch(time[0], LINE2(2));      //HH
    clcd_putch(time[1], LINE2(3));
    clcd_putch(':', LINE2(4));
    clcd_putch(time[2], LINE2(5));      //MM
    clcd_putch(time[3], LINE2(6));
    clcd_putch(':', LINE2(7));
    clcd_putch(time[4], LINE2(8));      //SS
    clcd_putch(time[5], LINE2(9));
}

/* display dashboard function */
void display_dashboard(unsigned char event[], unsigned char speed)
{
    clcd_print("TIME     E  SP", LINE1(2));
    
    // display Time
    display_time();
    // display event
    
    clcd_print(event , LINE2(11));
    
    // display speed
    // int to char
    clcd_putch((speed / 10) + '0', LINE2(14));
    clcd_putch((speed % 10) + '0', LINE2(15));
    
}

/* store the event to EEPROM */
void store_event()
{
    char addr;
    if(log_pos == 10)
    {
        log_pos = 0;
    }
    addr = 0x05 + (log_pos *10);    //5 15 25
    write_str_external_eeprom(addr, log);
    log_pos++;
    
}

/* copy the current event and time */
void log_event(unsigned char event[], unsigned char speed)
{
    get_time();
    strncpy(log, time, 6);           // store time HHMMSS -> [6] bytes
    strncpy(&log[6], event, 2);      // store event [2] bytes
    
    log[8] = (speed / 10) + '0';     // store 1st digit of speed 98/10 -> 9
    log[9] = (speed % 10) + '0';     // store 2nd digit of speed 98%10 -> 8
    log[10] = '\0';                  // store '\0' to terminate the log string
    
    store_event();                   // store event in the External_EEPROM  
}

/* Login screen - enter password */
unsigned char login(unsigned char key, unsigned char reset_flag)
{
    static char user_password[4];
    static unsigned char i;
    static unsigned char attempts_left;

    
    if(reset_flag == RESET_PASSWORD)        // reset login screen flag
    {
        i = 0;
        attempts_left = 3;
        user_password[0] = '\0';
        user_password[1] = '\0';
        user_password[2] = '\0';
        user_password[3] = '\0';
        key = ALL_RELEASED;         // key all released to avoid collision
        return_time = 5;
    }
    
    // when inactivity for 5-seconds
    if(return_time == 0)
    {
        return RETURN_BACK;
    }
    
    if(key == LPSW4 && i < 4)       // store the 1 and print *
    {
        clcd_putch('*', LINE2(5+i));
        user_password[i++] = '1';
        return_time = 5;
    }
    else if(key == LPSW5 && i < 4)  // store the 0 and print *
    {
        clcd_putch('*', LINE2(5+i));
        user_password[i++] = '0';
        return_time = 5;
    }
     
    if(i == 4)
    {
        user_password[i] = '\0';
        char original_password[4];
        char index;
        
        /* read the original password from EEPROM */
        for(index=0; index<4; index++) 
        {
            original_password[index] = read_external_eeprom(0x00 + index);
        }
        original_password[index] = '\0';
        
        
        clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
        __delay_ms(500);
        clear_screen();
        
        if(strcmp(original_password, user_password) == 0)   // compare the password correct or not
        {   
            //login success
            clcd_print("Login Success", LINE1(1));
            __delay_ms(1000);
            //change to menu screen
            return LOGIN_SUCCESS;
        }
        else        // if password wrong reduce attempt 
        { 
            attempts_left--;
            if(attempts_left == 0)  // if attempts 0 block for 60 seconds
            {
                clear_screen();
                clcd_print("You are Blocked", LINE1(0));
                clcd_print("Wait for ", LINE2(0));
                clcd_print("Secs", LINE2(12));
                sec = 60;
                // wait until sec 0
                while(sec)
                {
                    clcd_putch((sec / 10) + '0', LINE2(9));
                    clcd_putch((sec % 10) + '0', LINE2(10));
                }
                attempts_left = 3;
            }
            else        // reduce attempts and print on CLCD
            {
            clcd_print("Wrong Password", LINE1(0));
            clcd_print("Attempt Left:", LINE2(0));
            clcd_putch(attempts_left + '0', LINE2(14));
            __delay_ms(1000);
            }       
        }
        i = 0;
        clear_screen();
        clcd_print("Enter Password", LINE1(1));
        clcd_write(LINE2(5), INST_MODE);
        clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
        __delay_us(100);
        return_time = 5;
    }
    
}

/* main_menu screen */
unsigned char menu_screen(unsigned char key, unsigned char reset_flag)
{    
    if(reset_flag == RESET_MENU)        // reset the main menu
    {
        menu_pos = 0;
        return_time = 5;
        clear_screen();
    }   
    
    if( key == SW4 && menu_pos < 4)     // up to see the main menu list
    {
        //  incre menu pos
            clear_screen();
            menu_pos++;
            return_time = 5;
    }
    else if( key == SW5 && menu_pos > 0)    // down to see the main menu list
    {
        // decre menu pos
            clear_screen();
            menu_pos--;
            return_time = 5;
    }

    // when inactivity for 5-seconds
    if(return_time == 0)
    {
        return RETURN_BACK;
    }
    
    if(menu_pos < 4)        // display the menu
    {
        clcd_putch('*', LINE1(1));       
        clcd_print(menu[menu_pos], LINE1(3));
        clcd_print(menu[menu_pos+1], LINE2(3));
    }
    else 
    {
        clcd_putch('*', LINE2(1));
        clcd_print(menu[menu_pos-1], LINE1(3));
        clcd_print(menu[menu_pos], LINE2(3));
    }
        
    // return the menu pos    
    return menu_pos;
}

/* view log function */
unsigned char view_log(unsigned char key, unsigned char reset_flag)
{
    static unsigned char display_log[10];
    static unsigned char addr, index = 0;
    
    
    if(reset_flag == RESET_VIEW_LOG)    // reset flag
    {
        index = 0;
    }
    
    if(key == SW4 && index < log_pos - 1)   // increment the view_log
    {
        index++;// Increment  log
    }
    else if(key == SW5 && index > 0)       // decrement the view_log
    {
        index--;// decrement log    
    }
    
    /* read no.of logs from EEPROM */
    addr = 0x05 + (index * 10);    //5 15 25 
    for(int i = 0; i < 10; i++)
    {
        display_log[i] = read_external_eeprom(addr + i);
    }
    
    /* display in CLCD */
    clcd_print("# TIME     E  SP", LINE1(0));   
    clcd_putch(index + '0', LINE2(0));
    clcd_putch(display_log[0], LINE2(2));
    clcd_putch(display_log[1], LINE2(3));
    clcd_putch(':', LINE2(4));
    clcd_putch(display_log[2], LINE2(5));
    clcd_putch(display_log[3], LINE2(6));
    clcd_putch(':', LINE2(7));
    clcd_putch(display_log[4], LINE2(8));
    clcd_putch(display_log[5], LINE2(9));
    clcd_putch(display_log[6], LINE2(11));
    clcd_putch(display_log[7], LINE2(12));
    clcd_putch(display_log[8], LINE2(14));
    clcd_putch(display_log[9], LINE2(15));
    for(unsigned long int i = 0; i < 2000; i++);    // blocking delay
}


/* clear the log from EEPROM */
unsigned char clear_log()
{
    log_pos = 0;    // reset the log_pos to clear log
    clcd_print("Logs Cleared", LINE1(2));
    clcd_print("Successfully", LINE2(2));
    __delay_ms(2000);

    return SUCCESS;
}

/* download log from EEPROM and print to serial terminal */
unsigned char download_log(void)
{
    unsigned char download_log[10];     // log array
    char address;
    int pos = 0;
    
    puts("Car Black Box\n\r");    //display in serial monitor
    puts("Logs :");
    putchar('\n');
    putchar('\n');
    putchar('\r');
    
    clcd_print("log Downloaded", LINE1(1));     // print confirmation Msg in clcd
    clcd_print("Successfully", LINE2(2));
    
    // read log from EEPROM
    while(pos < log_pos)
    {
        address = 0x05 + (pos * 10);    //5 15 25   
        for(char i = 0; i<10; i++)
        {
            download_log[i] = read_external_eeprom(address + i);     // read log from EEPROM
        }
        __delay_us(100);
        // display all logs in serial monitor
        putchar(pos + '0');         
        putchar(' ');
        putchar(download_log[0]);
        putchar(download_log[1]);
        putchar(':');
        putchar(download_log[2]);
        putchar(download_log[3]);
        putchar(':');
        putchar(download_log[4]);
        putchar(download_log[5]);
        putchar(' ');
        putchar(download_log[6]);
        putchar(download_log[7]);
        putchar(' ');
        putchar(download_log[8]);
        putchar(download_log[9]);
        puts("\n\r");
        for(unsigned long int i = 0; i < 2000; i++);
        pos++;      // increase the position
    }
    
    putchar('\n');
    __delay_ms(3000);
    
    return SUCCESS; //return back to main menu
}


/* Reset time and update RTC register */
unsigned char change_time(unsigned char key, unsigned char reset_flag)
{
    static unsigned int new_time[3];
    static unsigned char blink_pos, blink = 0, wait = 0;
    
    if(reset_flag == RESET_TIME)        // reset time flag, initially read the current time from RTC
    {
        get_time();
        new_time[0] = (time[0] & 0x0F) * 10 + (time[1] & 0x0F);     // hour field store in integer format
        new_time[1] = (time[2] & 0x0F) * 10 + (time[3] & 0x0F);     // minute field store in integer format
        new_time[2] = (time[4] & 0x0F) * 10 + (time[5] & 0x0F);     // seconds filed store in integer format
        clcd_print("Time (HH:MM:SS)", LINE1(0));
        key = ALL_RELEASED;  // key released to avoid collision
        wait = 0;
        blink_pos = 2;
    }
    
    if(key == LPSW4)    // increment the current field
    {
        new_time[blink_pos]++;
    }
    else if(key == LPSW5)   // toggle field
    {
        if(blink_pos-- == 0)
            blink_pos = 2;
    }
    else if(key == LPSW6)   // update the time and RTC register
    {
        // update to time register
        write_ds1307(HOUR_ADDR, ((new_time[0] / 10) << 4) | (new_time[0] % 10));
        write_ds1307(MIN_ADDR,  ((new_time[1] / 10) << 4) | (new_time[1] % 10));
        write_ds1307(SEC_ADDR,  ((new_time[2] / 10) << 4) | (new_time[2] % 10));
        
        // display message
        clear_screen();
        clcd_print("Time Changed", LINE1(2));
        clcd_print("Successfully", LINE2(2));
        __delay_ms(2000);
        
        return SUCCESS;
    }
    
    if(new_time[0] > 23)    // hour filed increment upto 23
        new_time[0] = 0;
    if(new_time[1] > 59)    // minute filed increment upto 59
        new_time[1] = 0;
    if(new_time[2] > 59)    // second filed increment upto 59
        new_time[2] = 0;
    
    if(wait++ == 150)       // blink the current field
    {
        wait = 0;
        blink = !blink;
        clcd_putch(new_time[0] / 10 + '0', LINE2(0));      //HH
        clcd_putch(new_time[0] % 10 + '0', LINE2(1));
        clcd_putch(':', LINE2(2));
        clcd_putch(new_time[1] / 10 + '0', LINE2(3));      //MM
        clcd_putch(new_time[1] % 10 + '0', LINE2(4));
        clcd_putch(':', LINE2(5));
        clcd_putch(new_time[2] / 10 + '0', LINE2(6));      //SS
        clcd_putch(new_time[2] % 10 + '0', LINE2(7));
        
        if(blink)
        {
            switch(blink_pos)
            {
                case 0:
                    clcd_print("  ", LINE2(0));
                    break;
                case 1:
                    clcd_print("  ", LINE2(3));
                    break;
                case 2:
                    clcd_print("  ", LINE2(6));
                    break;
            }
        }
    }
    return FAILURE;
}


/* change password and store in EEPROM */
unsigned char change_password(unsigned char key, unsigned char reset_flag)
{
    static unsigned char i = 0, flag = 0;
    
    if(reset_flag == RESET_PASSWORD)    //password reset flag
    {
        key = ALL_RELEASED;
        i = 0;
        flag = 0;
    }
    if(flag == 0)   // flag 0  for store the new password
    {
        if(key == LPSW4 && i < 4)   // store 1 or 0 and print * on screen
        {
            clcd_putch('*', LINE2(5+i));
            new_password[i++] = '1';
        }
        else if(key == LPSW5 && i < 4)
        {
            clcd_putch('*', LINE2(5+i));
            new_password[i++] = '0';
        }
    
        if(i == 4)
        {
            clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
            __delay_ms(500);
            clear_screen();
            i = 0;
            flag = 1;
            __delay_ms(100);
            clcd_print("ReEnter Password", LINE1(0));
            clcd_write(LINE2(5), INST_MODE);
            clcd_write(DISP_ON_AND_CURSOR_ON, INST_MODE);
            __delay_us(100);
            key = ALL_RELEASED;
        }
    
    }
    if(flag)        // flag 1 for check confirm password
    {
        if(key == LPSW4 && i < 4)   // store 1 or 0 and print * on screen
        {
            clcd_putch('*', LINE2(5+i));
            re_password[i++] = '1';
        }
        else if(key == LPSW5 && i < 4)
        {
            clcd_putch('*', LINE2(5+i));
            re_password[i++] = '0';
        }
    
        if(i == 4)
        {
            clcd_write(DISP_ON_AND_CURSOR_OFF, INST_MODE);
            __delay_ms(500);
            clear_screen();
            
            if(strncmp(re_password, new_password, 4) == 0)      // check both password
            {
                write_str_external_eeprom(0x00, new_password);  // update password to EEPROM
                
                clcd_print("Password Changed", LINE1(0));
                clcd_print("Successfully", LINE2(2));
                __delay_ms(3000);
                return SUCCESS;
            }
            else 
            {
                clcd_print("Password Change", LINE1(0));    // password matches fails return to main manu
                clcd_print("Failed", LINE2(5));
                __delay_ms(3000);
                
                return SUCCESS;
            }
        }
    }
    return FAILURE;
}


/* clear the display screen */
void clear_screen()
{
    clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
        __delay_us(500);
}