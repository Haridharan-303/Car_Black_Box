/* 
 * File:   ds1307.h
 */

#ifndef EXTERNAL_EEPROM
#define	EXTERNAL_EEPROM

#define EXT_SLAVE_WRITE             0xA0
#define EXT_SLAVE_READ              0xA1

#define SEC_ADDR                0x00
#define MIN_ADDR                0x01
#define HOUR_ADDR               0x02
#define DAY_ADDR                0x03
#define DATE_ADDR               0x04
#define MONTH_ADDR              0x05
#define YEAR_ADDR               0x06
#define CNTL_ADDR               0x07


void init_AT24C04(void);
unsigned char read_external_eeprom(unsigned char addr);
void write_external_eeprom(unsigned char addr, unsigned char data);
void write_str_external_eeprom(unsigned char addr, char *data);

#endif	/* EXTERNAL_EEPROM */

