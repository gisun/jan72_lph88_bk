#include "device.h"

#include <inttypes.h>

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/interrupt.h>


#include "init.h"
#include "menu.h"
#include "timers.h"

#include "vars.h"

// Timer 0 overflow interrupt service routine
ISR(TIMER0_OVF_vect) {
    // Reinitialize Timer 0 value
    TCNT0 = 0x06;

    kline_delay++;

    if(worker.dtime8 != 0)
	if(++t_msec == worker.dtime8){
	    worker.update = 1;
	    t_msec = 0;
	    if(++t_sec == 60){
		t_sec=0;
		if(++t_min == 60){
		    t_min = 0;
		    if(++t_hour==24)t_hour = 0;
		};
	    };
	};
}

// Timer1 overflow interrupt service routine
ISR(TIMER1_OVF_vect) {
    // Reinitialize Timer1 value
    TCNT1H=0xE796 >> 8;           // таймер срабатывает каждые 0,1 сек
    TCNT1L=0xE796 & 0xff;
}

void timer0_start(void){
    TIMSK0 |= _BV(0);
}

void timer0_stop(void){
    TIMSK0 &= ~_BV(0);
}

void timer1_start(void){
    TIMSK1 |= _BV(0);
}

void timer1_stop(void){
    TIMSK1 &= ~_BV(0);
}


