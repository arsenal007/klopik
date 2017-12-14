/*
 * i2c_master.h
 *
 *  Created on: 13 дек. 2017 г.
 *      Author: Vasyl
 */

#ifndef I2C_MASTER_H_
#define I2C_MASTER_H_

#ifndef F_CPU
        #error F_CPU is undefined, TWI may not work correctly without this parametr
#endif

#define I2C_READ 0x01
#define I2C_WRITE 0x00

extern void i2c_init(void);
extern uint8_t i2c_start(uint8_t address);
extern uint8_t i2c_write(uint8_t data);
extern uint8_t i2c_read_ack(void);
extern uint8_t i2c_read_nack(void);
extern uint8_t i2c_transmit(uint8_t address, uint8_t* data, uint16_t length);
extern uint8_t i2c_receive(uint8_t address, uint8_t* data, uint16_t length);
extern uint8_t i2c_writeReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length);
extern uint8_t i2c_write1Reg(uint8_t devaddr, uint8_t regaddr, uint8_t data);
extern uint8_t i2c_readReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length);
extern uint8_t i2c_read1Reg(uint8_t devaddr, uint8_t regaddr, uint8_t* data);
extern void i2c_stop(void);
extern void i2c_scan(void);

#define F_SCL 100000UL // SCL frequency
#define Prescaler 1
#define TWBR_val ((((F_CPU / F_SCL) / Prescaler) - 16 ) / 2)




#endif /* I2C_MASTER_H_ */
