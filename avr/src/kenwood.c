/*
 * kenwood.c
 *
 *  Created on: 13 дек. 2017 г.
 *      Author: Vasyl
 */


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

#include "main.h"
#include "uart.h"
#include "si5351.h"
#include "i2c_master.h"
#include "atmega-timers.h"
#include "kenwood.h"

/********************************
Variables for important values from CAT,
that can be used in main firmware code.
Science they are static, They are visible only
in this file and trough functions.
**********************************/

// Marker. if CAT firmware have recieved new frequency.
static uint8_t cat_kenwood_new_freq_active;
// Marker.  if CAT firmware have recieved command RIG to transmit
static uint8_t cat_kenwood_tx_active;
// Marker.  if CAT firmware have recieved command RIG to recieve.
static uint8_t cat_kenwood_rx_active;
// Value. New value of frequency.
static uint32_t cat_kenwood_new_freq_vfo;

static uint8_t cat_kenwood_active;

static uint16_t timer_activity;

//static uint16_t timer_LED;

/*void timer_cat( void){
        timer_LED++;
	timer_activity++;
        if ((timer_LED == 20 ) && (cat_kenwood_active)) {
		timer_LED = 0;
		LED_TEST_PORT INV_BIT(LED_TEST_BIT);
                _delay_ms(10);
                LED_TEST_PORT INV_BIT(LED_TEST_BIT);
        } else if ( timer_activity == 60 * 60 ) {
		timer_activity = 0;
		cat_kenwood_active = 0;
        };
};*/



void cat_kenwood_init( void  ) {
	uart_init(UART_BAUD_SELECT(BAUD,F_CPU));
	//timer1(TIMER1_PRESCALER_1024, 15625U, timer_cat);
}

static int messageIndex = 0;
static char message[64];

void cat_kenwood(void ) {
	cat_kenwood_new_freq_active = 0;
	cat_kenwood_tx_active = 0;
	cat_kenwood_rx_active = 0;
	cat_kenwood_new_freq_vfo = 0;
	while ( uart_available() > 0 ) {
			int c = uart_getc();
			//char HighByte = (char)c >> 8;
			char LowByte = (char)c;
			//uart_puts(&c);
			if (LowByte == ';') {
			        message[messageIndex]='\0';
				if ( (strncmp(message,"F",1)==0) && (messageIndex==13)) { // command FA00007000000; Setup a new frequency
					cat_kenwood_new_freq_vfo = strtoul(&message[2], NULL, 10);
					cat_kenwood_new_freq_active = 1;
					timer_activity = 0;
					cat_kenwood_active = 1;
					//for(int i = 4;i < 15; i++) vfo = (vfo * 10ULL) + (long)(message[i]-'0');
					//SetFreq(4 * vfo);
				} else if ((strncmp(message,"F",1)==0) && (messageIndex==2)) {
					char buff[16];
					//uint32_t vfo = floor((double)R.Freq  * 1/( CONV_CONST));
					sprintf(buff,"%011lu;\n", get_vfo() );
					message[2]='\0';
					strcat(message,buff);
					uart_puts(message);
					timer_activity = 0;
					cat_kenwood_active = 1;
				} else if ( (strncmp(message,"RX",2)==0) && (messageIndex==2)) {
					cat_kenwood_tx_active = 1;
					timer_activity = 0;
					cat_kenwood_active = 1;
				} else if ( (strncmp(message,"TX",2)==0) && (messageIndex==2)) {
					cat_kenwood_rx_active = 1;
					timer_activity = 0;
					cat_kenwood_active = 1;
				} else if ( (strncmp(message,"TY",2)==0) && (messageIndex==2) ) {
                                        char buff[16];
					uint8_t value = 1;
                                        sprintf(buff,"%03u;\n", value);
                                        message[2]='\0';
                                        strcat(message,buff);
                                        uart_puts(message);
					timer_activity = 0;
					cat_kenwood_active = 1;
				} else if ( (strncmp(message,"IF",2)==0) && (messageIndex==2) ) {
					char buff[16];
					sprintf(buff,"%011lu", get_vfo() );
					strcat(message,buff);
					memset(message+13, ' ', 5);
				        //if (trx.RIT) {
				//            ltoazp(CAT_buf+18,trx.RIT_Value,5);
				//            CAT_buf[23] = '1';
				//          } else {
					memset(message+18, '0', 6);
          			//		}
				        memset(message+24, '0', 4);
					message[28] = '0' + ( cat_kenwood_tx_active & 0x01 );
					uint8_t sideband = 0;
				        message[29] = '1' + sideband;
				        message[30] = '0' + 0; //VFO A/B
				        message[31] = '0';
				        message[32] = '0' + 0; //(trx.state.Split & 1);
          				memset(message+33, '0', 3);
				        message[36] = ' ';
				        message[37] = ';';
					message[38] = 0;
					uart_puts(message);
#ifdef I2C_MASTER_H
				} else if ( (strncmp(message,"I2C_SCAN",8)==0) && (messageIndex==8)) {
                                        i2c_scan();
#endif
#ifdef SI5351_H_
				} else if ( (strncmp(message, "SI5351_STATUS", strlen("SI5351_STATUS") )==0) && (messageIndex==strlen("SI5351_STATUS")) ) {
                                        si5351_update_status();
                                        uart_puts_P("SYS_INIT: ");
                                        uart_puthex(dev_status.SYS_INIT);
                                        uart_puts_P(";\n");
                                        uart_puts_P("LOL_A: ");
                                        uart_puthex(dev_status.LOL_A);
                                        uart_puts_P(";\n");
                                        uart_puts_P("LOL_B: ");
                                        uart_puthex(dev_status.LOL_B);
                                        uart_puts_P(";\n");
				} else if ( (strncmp(message,"SI5351_ADDRESS",strlen("SI5351_ADDRESS"))==0) && (messageIndex==strlen("SI5351_ADDRESS")) ) {
                                        uart_puts_P( "SI5351_ADDRESS" );
                                        uart_puthex( R.Si5351Address );
                                        uart_puts_P(";\n");
				} else if ( (strncmp(message,"SI5351_ADDRESS", strlen("SI5351_ADDRESS"))==0) && (messageIndex==strlen("SI5351_ADDRESS")+4) ) {
					R.Si5351Address = strtoul(&message[strlen("SI5351_ADDRESS")], NULL, 16);
                                        eeprom_write_block(&R.Si5351Address, &E.Si5351Address, sizeof(R.Si5351Address));
				} else if ( (strncmp(message,"SI5351_PLLFREQ",strlen("SI5351_PLLFREQ"))==0) && (messageIndex==strlen("SI5351_PLLFREQ")) ) {
					char buff[16];
                                        uart_puts_P("SI5351_PLLFREQ");
					sprintf(buff,"%011lu;\n", R.pll_freq );
					uart_puts(buff);
				} else if ( (strncmp(message,"SI5351_PLLFREQ", strlen("SI5351_PLLFREQ"))==0) && (messageIndex==strlen("SI5351_PLLFREQ")+11) ) {
                                        R.pll_freq = strtoul(&message[strlen("SI5351_PLLFREQ")], NULL, 10);
					eeprom_write_block(&R.pll_freq, &E.pll_freq, sizeof(R.pll_freq));
					// reset si5351
				} else if ( (strncmp(message,"SI5351_XTAL",strlen("SI5351_XTAL"))==0) && (messageIndex==strlen("SI5351_XTAL")) ) {
                                        char buff[16];
                                        uart_puts_P("SI5351_XTAL");
                                        sprintf(buff,"%011lu;\n", R.XtalFreqSi5351 );
					uart_puts(buff);
				} else if ( (strncmp(message,"SI5351_XTAL", strlen("SI5351_XTAL"))==0) && (messageIndex==strlen("SI5351_XTAL")+11) ) {
                                        R.pll_freq = strtoul(&message[strlen("SI5351_XTAL")], NULL, 10);
                                        eeprom_write_block(&R.XtalFreqSi5351, &E.XtalFreqSi5351, sizeof(R.XtalFreqSi5351));
					// reset si5351
				} else if ( (strncmp(message,"SI5351_LSB",strlen("SI5351_LSB"))==0) && (messageIndex==strlen("SI5351_LSB")) ) {
                                        char buff[16];
                                        uart_puts_P("SI5351_LSB");
                                        sprintf(buff,"%011lu;\n", R.LSB_freq);
                                        uart_puts(buff);
                                } else if ( (strncmp(message,"SI5351_LSB", strlen("SI5351_LSB"))==0) && (messageIndex==strlen("SI5351_LSB")+11) ) {
                                        R.LSB_freq = strtoul(&message[strlen("SI5351_LSB")], NULL, 10);
                                        eeprom_write_block(&R.LSB_freq, &E.LSB_freq, sizeof(R.LSB_freq));
                                        // reset si5351
				} else if ( (strncmp(message,"SI5351_USB",strlen("SI5351_USB"))==0) && (messageIndex==strlen("SI5351_USB")) ) {
                                        char buff[16];
                                        uart_puts_P("SI5351_USB");
                                        sprintf(buff,"%011lu;\n", R.USB_freq);
                                        uart_puts(buff);
                                } else if ( (strncmp(message,"SI5351_USB", strlen("SI5351_USB"))==0) && (messageIndex==strlen("SI5351_USB")+11) ) {
                                        R.USB_freq = strtoul(&message[strlen("SI5351_USB")], NULL, 10);
                                        eeprom_write_block(&R.USB_freq, &E.USB_freq, sizeof(R.USB_freq));
                                        // reset si5351

#endif
 				} else {
					uart_puts_P("?;\n");
				};
				messageIndex = 0;
			} else {
				message[messageIndex++]=LowByte;
				//uart_puts(message);
			};
		};
};

uint8_t cat_kenwood_new_freq_is_active(void) {
	return cat_kenwood_new_freq_active;
};

uint8_t cat_kenwood_tx_is_active(void) {
	return cat_kenwood_tx_active;
};

uint8_t cat_kenwood_rx_is_active(void) {
        return cat_kenwood_rx_active;
};

uint32_t cat_kenwood_get_new_freq_vfo(void) {
        return cat_kenwood_new_freq_vfo;
};

