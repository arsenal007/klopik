/*
 * kenwood.h
 *
 *  Created on: 13 дек. 2017 г.
 *      Author: Vasyl
 */

#ifndef KENWOOD_H_
#define KENWOOD_H_

extern void cat_kenwood_init( void );
extern void cat_kenwood(void );
extern uint8_t cat_kenwood_new_freq_is_active(void);
extern uint8_t cat_kenwood_tx_is_active(void);
extern uint8_t cat_kenwood_rx_is_active(void);
extern uint32_t cat_kenwood_get_new_freq_vfo(void);

#define BAUD 57600
#endif /* KENWOOD_H_ */
