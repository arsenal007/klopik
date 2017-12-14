#ifndef MAIN_H_
#define MAIN_H_



#define LED_TEST_PORT PORTC
#define LED_TEST_DDR  DDRC
#define LED_TEST_BIT  PC0


#define SET_BIT(x) |= (1<<x)
#define CLR_BIT(x) &=~(1<<x)
#define INV_BIT(x) ^=(1<<x)
#define TEST_BIT(x) & (1<<x)

#define BUFLEN 64

#define _2(x)           ((uint32_t)1<<(x))      // Take power of 2

#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)

#define TX_EPA_OUT_PORT PORTC
#define TX_EPA_OUT_DDR DDRC
#define TX_EPA_OUT_BIT PC2

#define TX_SN74HC4053_PORT PORTD
#define TX_SN74HC4053_DDR DDRD
#define TX_SN74HC4053_BIT PD4

#define TX_PA_GAIN_PORT PORTD
#define TX_PA_GAIN_DDR DDRD
#define TX_PA_GAIN_BIT PD5


#define TX_LPA_PORT PORTC
#define TX_LPA_DDR DDRC
#define TX_LPA_BIT PC3

typedef struct var_t var_t;

struct var_t {
                uint8_t         Si5351Address;
                uint32_t        pll_freq;
                uint32_t        XtalFreqSi5351;                               // crystal frequency[MHz] (8.24bits)
                uint32_t        StartRigFreq;                                 // Running frequency[MHz] (11.21bits)
                uint32_t        LSB_freq;                                          // Upper and lower Freqency of crystal filter
                uint32_t        USB_freq;
} ;

extern var_t R;
extern EEMEM   var_t           E;
extern uint32_t get_vfo(void);
#endif

