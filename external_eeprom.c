#include <xc.h>
#include "i2c.h"
#include "external_eeprom.h"

void init_AT24C04(void)
{
    unsigned char dummy;
   
    dummy = read_external_eeprom(SEC_ADDR);
    dummy = dummy & 0x7F;
    write_external_eeprom(SEC_ADDR, dummy);// ch = 0
}

unsigned char read_external_eeprom(unsigned char addr)
{
    unsigned char data;
    
    i2c_start();
    i2c_write(EXT_SLAVE_WRITE);
    i2c_write(addr);
    i2c_rep_start();
    i2c_write(EXT_SLAVE_READ);
    data = i2c_read(0);
    i2c_stop();
    
    return data;
}

void write_external_eeprom(unsigned char addr, unsigned char data) // SEc_ADDR, data
{
    i2c_start();
    i2c_write(EXT_SLAVE_WRITE);
    i2c_write(addr);
    i2c_write(data);
    i2c_stop();
}

void write_str_external_eeprom(unsigned char addr, char *data)
{
    while(*data != 0)
    {
        write_external_eeprom(addr, *data);
        data++;
        addr++;
    }
}
