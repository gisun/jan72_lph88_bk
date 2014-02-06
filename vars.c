#include "device.h"

#include <inttypes.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "init.h"
#include "menu.h"
#include "timers.h"

#include "vars.h"


volatile uint8_t need_reset;
volatile uint16_t kline_delay;


volatile float ecu_vcc;
volatile uint8_t ecu_temp;
volatile uint8_t ecu_throttle;

volatile uint8_t ecu_speed;
volatile int ecu_rpm;

volatile float ecu_inj;
volatile float ecu_oil;

volatile float ecu_adc_maf;
volatile float ecu_adc_lambda;

volatile uint8_t t_msec;// 0,1 s
volatile uint8_t t_sec;// 1 sec 0..60
volatile uint8_t t_min;// 1 min 0..59
volatile uint8_t t_hour;// 1 hour 0..23

uint8_t convert[32];

