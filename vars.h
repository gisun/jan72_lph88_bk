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

extern volatile float ecu_inj;
extern volatile float ecu_oil;
extern volatile float ecu_oil_tmp;
extern volatile uint8_t ecu_oil_cnt;
extern volatile float ecu_full_oil;

extern volatile float ecu_adc_maf;
extern volatile float ecu_adc_lambda;


extern volatile uint8_t t_msec;// 0,1 s
extern volatile uint8_t t_sec;// 1 sec 0..60
extern volatile uint8_t t_min;// 1 min 0..59
extern volatile uint8_t t_hour;// 1 hour 0..23

extern uint8_t convert[];

#endif
