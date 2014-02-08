#ifndef _VARS_H
#define _VARS_H

extern volatile uint8_t is_connected;
extern volatile uint8_t need_reset;

extern volatile uint16_t kline_delay;

extern volatile float ecu_vcc;
extern volatile uint8_t ecu_temp;
extern volatile uint8_t ecu_throttle;

extern volatile uint8_t ecu_speed;
extern volatile int ecu_rpm;

extern volatile float ecu_trip;

extern volatile float ecu_inj;
extern volatile float ecu_fuel;
extern volatile float ecu_fuel_tmp;
extern volatile float ecu_fuel_full;

extern volatile uint8_t ecu_fuel_cnt;


extern volatile float ecu_adc_maf;
extern volatile float ecu_adc_lambda;

extern volatile uint8_t t0_dtime;

extern volatile uint32_t t0_timer;
extern volatile uint32_t t0_timer_last;

extern uint8_t convert[];

#endif
