#include <avr/io.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <util/twi.h>

#include "i2c_master.h"
#include "uart.h"


void i2c_init(void)
{
	TWBR = (uint8_t)TWBR_val;
}

uint8_t i2c_start(uint8_t address)
{
	// reset TWI control register
	TWCR = 0;
	// transmit START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );

	// check if the start condition was successfully transmitted
	if((TWSR & 0xF8) != TW_START){ return 1; }

	// load slave address into data register
	TWDR = address;
	// start transmission of address
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );

	// check if the device has acknowledged the READ / WRITE mode
	uint8_t twst = TW_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;

	return 0;
}

uint8_t i2c_write(uint8_t data)
{
	// load data into data register
	TWDR = data;
	// start transmission of data
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );

	if( (TWSR & 0xF8) != TW_MT_DATA_ACK ){ return 1; }

	return 0;
}

uint8_t i2c_read_ack(void)
{

	// start TWI module and acknowledge data after reception
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	// return received data from TWDR
	return TWDR;
}

uint8_t i2c_read_nack(void)
{

	// start receiving without acknowledging reception
	TWCR = (1<<TWINT) | (1<<TWEN);
	// wait for end of transmission
	while( !(TWCR & (1<<TWINT)) );
	// return received data from TWDR
	return TWDR;
}

uint8_t i2c_transmit(uint8_t address, uint8_t* data, uint16_t length)
{
	if (i2c_start(address | I2C_WRITE)) return 1;

	for (uint16_t i = 0; i < length; i++)
	{
		if (i2c_write(data[i])) return 1;
	}

	i2c_stop();

	return 0;
}

uint8_t i2c_receive(uint8_t address, uint8_t* data, uint16_t length)
{
	if (i2c_start(address | I2C_READ)) return 1;

	for (uint16_t i = 0; i < (length-1); i++)
	{
		data[i] = i2c_read_ack();
	}
	data[(length-1)] = i2c_read_nack();

	i2c_stop();

	return 0;
}

uint8_t i2c_writeReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length)
{
	if (i2c_start(devaddr | I2C_WRITE)) return 1;

	i2c_write(regaddr);

	for (uint16_t i = 0; i < length; i++)
	{
		if (i2c_write(data[i])) return 1;
	}

	i2c_stop();

	return 0;
}

uint8_t i2c_write1Reg(uint8_t devaddr, uint8_t regaddr, uint8_t data)
{
        if (i2c_start(devaddr | I2C_WRITE)) return 1;

        i2c_write(regaddr);

        if (i2c_write(data)) return 1;

        i2c_stop();

        return 0;
}



uint8_t i2c_readReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length)
{
	if (i2c_start(devaddr | I2C_WRITE )) return 1;

	i2c_write(regaddr);

	if (i2c_start(devaddr | I2C_READ)) return 1;

	for (uint16_t i = 0; i < (length-1); i++)
	{
		data[i] = i2c_read_ack();
	}
	data[(length-1)] = i2c_read_nack();

	i2c_stop();

	return 0;
}

uint8_t i2c_read1Reg(uint8_t devaddr, uint8_t regaddr, uint8_t* data)
{
        if (i2c_start(devaddr | I2C_WRITE )) return 1;

        i2c_write(regaddr);

        if (i2c_start(devaddr | I2C_READ)) return 1;

        *data = i2c_read_nack();

        i2c_stop();

        return 0;
}



void i2c_stop(void)
{
	// transmit STOP condition
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
}


//******************************************************************
//Function  : To scan i2c bus.
//Arguments : none
//Return    : none
//******************************************************************
#if defined(UART_H)
	void i2c_scan(void) {
		uart_puts_P("Scanning for devices on i2c bus  ... \n");
		for(uint8_t s = 0; s <= 0x7F; s++) {
			uint8_t ss = s << 1;
			if (i2c_start(ss | I2C_WRITE )==0) {
				uart_puts_P("Found device at address: ");
				uart_puthex(ss);
				uart_puts_P(";\n");
			}
			i2c_stop();
		}
	}
#else
	void i2c_scan(void){}
#endif
