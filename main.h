/* 
 * File:   main.h
 * Author: harid
 *
 * Created on 20 December, 2024, 9:48 AM
 */

#ifndef MAIN_H
#define	MAIN_H

#include <xc.h>
#include <string.h>
#include "adc.h"
#include "clcd.h"
#include "digital_keypad.h"
#include "ds1307.h"
#include "i2c.h"
#include "car_black_box.h"
#include "external_eeprom.h"
#include "timers.h"
#include "uart.h"

#define DASHBOARD_SCREEN    0x01
#define LOGIN_SCREEN        0x02
#define MAIN_MENU_SCREEN    0x03

#define RESET_PASSWORD      0x11
#define RESET_NOTHING       0x22
#define RETURN_BACK         0x33
#define LOGIN_SUCCESS       0x44
#define RESET_MENU          0x55

#define VIEW_LOG            0x66
#define CLEAR_LOG           0x77
#define DOWNLOAD_LOG        0x88
#define SET_TIME            0x99
#define CHANGE_PASSWORD     0xA0

#define RESET_VIEW_LOG      0x81
#define RESET_TIME          0x82

#define SUCCESS             0xFF
#define FAILURE             0xAA


#endif	/* MAIN_H */

