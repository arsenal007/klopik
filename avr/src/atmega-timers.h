/*
 * atmega-timers.h
 *
 *  Created on: 13 дек. 2017 г.
 *      Author: Vasyl
 */

#ifndef ATMEGA_TIMERS_H_
#define ATMEGA_TIMERS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <avr/io.h>
#include <avr/interrupt.h>

// Comment/delete those lines to disable interrupt definition for that timer
// so you can define your own ISR functions without conflicts.
#define ENABLE_TIMER0
#define ENABLE_TIMER1
#define ENABLE_TIMER2

#define TIMER0_PRESCALER_NONE 0
#define TIMER0_PRESCALER_1 1
#define TIMER0_PRESCALER_8 2
#define TIMER0_PRESCALER_64 3
#define TIMER0_PRESCALER_256 4
#define TIMER0_PRESCALER_1024 5

#define TIMER1_PRESCALER_NONE 0
#define TIMER1_PRESCALER_1 1
#define TIMER1_PRESCALER_8 2
#define TIMER1_PRESCALER_64 3
#define TIMER1_PRESCALER_256 4
#define TIMER1_PRESCALER_1024 5

#define TIMER2_PRESCALER_NONE 0
#define TIMER2_PRESCALER_1 1
#define TIMER2_PRESCALER_8 2
#define TIMER2_PRESCALER_32 3
#define TIMER2_PRESCALER_64 4
#define TIMER2_PRESCALER_128 5
#define TIMER2_PRESCALER_256 6
#define TIMER2_PRESCALER_1024 7


void timer0(uint8_t prescaler, uint8_t ticks, void (*f)());
void timer1(uint8_t prescaler, uint16_t ticks, void (*f)());
void timer2(uint8_t prescaler, uint8_t ticks, void (*f)());
void timer0_stop();
void timer1_stop();
void timer2_stop();

void wait0(uint8_t prescaler, uint8_t ticks);
void wait1(uint8_t prescaler, uint16_t ticks);
void wait2(uint8_t prescaler, uint8_t ticks);

#ifdef __cplusplus
}
#endif

#endif /* ATMEGA_TIMERS_H_ */
