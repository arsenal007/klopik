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
#include "i2c_master.h"
#include "si5351.h"
#include "kenwood.h"

#define LED_TEST_PORT PORTC
#define LED_TEST_DDR  DDRC
#define LED_TEST_BIT  PC0

#define BAND_10M_PORT PORTB
#define BAND_10M_DDR  DDRB
#define BAND_10M_BIT  PB2

#define BAND_20M_PORT PORTD
#define BAND_20M_DDR  DDRD
#define BAND_20M_BIT  PD6

#define BAND_40M_PORT PORTD
#define BAND_40M_DDR  DDRD
#define BAND_40M_BIT  PD5

#define BAND_80M_PORT PORTB
#define BAND_80M_DDR  DDRB
#define BAND_80M_BIT  PB0

#define TX_TRX_PORT PORTD
#define TX_TRX_DDR DDRD
#define TX_TRX_BIT PD4


#define F_MIN        1000000L               // Нижний предел частоты
#define F_MAX        30000000L              // Верхний предел частоты



#ifndef BAUD
#define BAUD 57600
#endif

//#define SET_PORT(PORT,BIT,VALUE) PORT |= (VALUE << BIT);

#define SET_BIT(x) |= (1<<x)
#define CLR_BIT(x) &=~(1<<x)
#define INV_BIT(x) ^=(1<<x)
#define TEST_BIT(x) & (1<<x)

#define FALSE 0
#define TRUE (!FALSE)

#define BUFLEN 64

#define _2(x)           ((uint32_t)1<<(x))      // Take power of 2


typedef union {
        uint16_t                w;
        struct {
                uint8_t         b0;
                uint8_t         b1;
        };
} sint16_t;

typedef union {
        uint32_t                dw;
        struct {
                sint16_t        w0;
                sint16_t        w1;
        };
} sint32_t;

EEMEM   var_t           E;				//
var_t           R                                                                      // Variables in ram
		               =                                                      // Variables in flash rom
{               .Si5351Address			= 0xC0
,		.pll_freq			= 600000000	// PLL frequency [MHz]
,       .XtalFreqSi5351                 = 30000000	// Crystal frequency [MHz]
,       .StartRigFreq                   = 7100000ULL	// Start receiving frequency[MHz]
,		.LSB_freq				= 8863000ULL
,		.USB_freq				= 8867000ULL
};

static uint32_t bfo;
static uint32_t vfo;

uint32_t get_vfo(void) {
	return vfo - bfo;
}

void blink_LED(int n) {
        for (int i = 0; i < n; i++) {
                _delay_ms(50);
                LED_TEST_PORT INV_BIT(LED_TEST_BIT);
                _delay_ms(50);
                LED_TEST_PORT INV_BIT(LED_TEST_BIT);
        };
};

inline static void set_bfo(uint32_t f) {
                si5351_set_freq(f, R.pll_freq, SI5351_CLK0);
};

inline static void set_vfo(uint32_t f) {
		vfo = f;
                si5351_set_freq(vfo, R.pll_freq, SI5351_CLK1);
};


// **********************************************************/
// INIT
// **********************************************************/
void init(void) {
	if (eeprom_read_byte(&E.Si5351Address) == 0xFF)
                eeprom_write_block(&R, &E, sizeof(E));  // Initialize eeprom to "factory defaults".
        else
                eeprom_read_block(&R, &E, sizeof(E));   // Load the persistend data from eeprom.

	cat_kenwood_init();
//	nokia_lcd_init();
//	nokia_lcd_clear();
//        nokia_lcd_write_string("Klopik init",1);
//        nokia_lcd_set_cursor(0, 10);
//        nokia_lcd_write_string("DONE!", 3);
//        nokia_lcd_render();

// band output pin init
//	blink_LED(10);
//	LED_TEST_PORT SET_BIT(LED_TEST_BIT);
	TX_TRX_DDR   SET_BIT(TX_TRX_BIT);
	TX_TRX_PORT  CLR_BIT(TX_TRX_BIT);

        LED_TEST_DDR SET_BIT(LED_TEST_BIT);
        LED_TEST_PORT CLR_BIT(LED_TEST_BIT);
	blink_LED(10);
//        uart_init(UART_BAUD_SELECT(BAUD,F_CPU));
        i2c_init();
	//i2c_scan();

        si5351_init(R.XtalFreqSi5351);
	si5351_clock_enable(SI5351_CLK0, TRUE);
	si5351_clock_enable(SI5351_CLK1, TRUE);
	//si5351_clock_enable(SI5351_CLK2, 1);
	si5351_drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
	si5351_drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA);
// init  LED set as output
	//LED_TEST_DDR SET_BIT(LED_TEST_BIT);
        //LED_TEST_PORT CLR_BIT(LED_R.LSBTEST_BIT);
	//blink_LED(10);
	si5351_set_pll(R.pll_freq, SI5351_PLLA);
	si5351_set_pll(R.pll_freq, SI5351_PLLB);
	bfo = R.LSB_freq;
	set_bfo(bfo);
	vfo = R.StartRigFreq + bfo;
	set_vfo(vfo);

// init  LED set as output
//        LED_TEST_DDR SET_BIT(LED_TEST_BIT);
//        LED_TEST_PORT CLR_BIT(LED_TEST_BIT);
// band output pin init

        sei();
}



/**********************************************************
 * External variables
 **********************************************************/

/**********************************************************
 * Global variables
 **********************************************************/


/**********************************************************
 * Main function
 **********************************************************/
int main(void){
	//char message[BUFLEN];
	init();
	//frequency = 5100000;
	//si570_set_frequency(frequency);
	//int messageIndex = 0;
	for(;;){
		si5351_update_status();
		if (dev_status.SYS_INIT == 1) {
			uart_puts_P("Initialising Si5351, you shouldn't see many of these! ;\n");

		};
		cat_kenwood();
		if ( cat_kenwood_new_freq_is_active() ) {
			uint32_t new_vfo = cat_kenwood_get_new_freq_vfo() ;
			if ((new_vfo >= 10000000ULL) && (bfo != R.USB_freq))    {
				bfo = R.USB_freq;
				set_bfo( bfo );
			        uart_puts_P("We've switched from LSB to USB");
			} else if ((new_vfo < 10000000ULL) && (bfo != R.LSB_freq))  {
				bfo = R.LSB_freq;
			        set_bfo( bfo );
				uart_puts_P("We've switched from USB to LSB");
			}
			new_vfo +=  bfo;

			set_vfo(new_vfo);
                } else if ( cat_kenwood_rx_is_active() ) {
                        TX_TRX_PORT         CLR_BIT(TX_TRX_BIT);
                } else if ( cat_kenwood_tx_is_active() ) {
                        TX_TRX_PORT         SET_BIT(TX_TRX_BIT);
                };

	};
	return 0;
};



