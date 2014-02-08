#include "device.h"

#include <inttypes.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "init.h"
#include "menu.h"
#include "timers.h"

#include "vars.h"


volatile uint8_t is_connected;

volatile uint8_t need_reset;
volatile uint16_t kline_delay;



volatile float ecu_vcc;
volatile uint8_t ecu_temp;
volatile uint8_t ecu_throttle;

volatile uint8_t ecu_speed;
volatile int ecu_rpm;

volatile float ecu_trip;

volatile uint8_t t0_dtime;
volatile uint32_t t0_timer;

volatile uint32_t t0_timer_last;

volatile float ecu_inj;
volatile float ecu_fuel;

volatile float ecu_fuel_tmp;
volatile uint8_t ecu_fuel_cnt;

volatile float ecu_fuel_full;

volatile float ecu_adc_maf;
volatile float ecu_adc_lambda;

uint8_t convert[32];

