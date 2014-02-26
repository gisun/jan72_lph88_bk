#include "device.h"

#include <inttypes.h>

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>

#include "init.h"
#include "s65_lcd.h"

#include "menu.h"
#include "ecu.h"
#include "timers.h"

#include "vars.h"

uint16_t textcolor;
uint16_t bordercolor;
uint16_t bgcolor;

uint8_t test_connection[] = {'&', 0};

#define def_MainMenuSize 6
MENU menu_main[def_MainMenuSize];

#define def_tripMenuSize 6
MENU menu_trip[def_tripMenuSize];

#define def_ErrorsMenuSize 1
MENU menu_errors[]={
    {   0,   0,  83,  60, (void *)sub_main}
};

#define def_AdcMenuSize 1
MENU menu_adc[]={
    {   0,   0,  83,  60, (void *)sub_main}
};

#define def_AccelMenuSize 1
MENU menu_accel[]={
    {   0,   0,  83,  60, (void *)sub_main}
};

#define def_ConfigMenuSize 1
MENU menu_config[]={
    {   0,   0,  83,  60, (void *)sub_main}
};

#define def_SystemMenuSize 2
MENU menu_system[def_SystemMenuSize];



PMENU pmenu;
uint8_t menu_size;
uint8_t cur_menu;

WORKER worker;

void worker_dummy(void){
    ;
};

void worker_t0_init(uint8_t dt_t0, void * sub){
    timer0_stop();
    worker.t0_dtime = dt_t0;
    worker.sub_worker = sub;
    worker.update = 0;
    worker.update_sm = 0;
    timer0_start();
}

void worker_test(void){
	uint8_t msec, sec, min, hour;
	uint32_t tmp;

	msec = t0_timer % 100;	//1/100 of sec
	tmp = t0_timer / 100;
	sec = tmp % 60;		//60 sec 1 min
	tmp = tmp / 60;
	min = tmp % 60;		//60 min 1 hour
	tmp = tmp / 60;
	hour = tmp % 24;	//24 hour 1 day
	sprintf(convert, "%2.2d:%2.2d:%2.2d.%2.2d", hour, min, sec, msec);
	s65_drawText( 60, 0, convert, 1, RGB(0x1E,0x2E,0x1E), RGB(0x00,0x00,0x0E));
}


void add_btn(uint8_t idx, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, PMENU menu, void * sub, uint16_t border, uint16_t bg){
    s65_drawRect( x1, y1, x2, y2, border);
    s65_fillRect( x1+1, y1+1, x2-1, y2-1, bg);

    menu[idx].x1 = x1; menu[idx].y1 = y1;
    menu[idx].x2 = x2; menu[idx].y2 = y2;

    menu[idx].sub_menu = sub;
}

void sub_main(void){
    bgcolor = RGB(0x00, 0x00, 0x0E);
    s65_clear(bgcolor);

    bgcolor = RGB(0x00, 0x00, 0x1E);
    textcolor = RGB(0x0F, 0x1F,0x0F);
    bordercolor = RGB(0x00, 0x0,0x00);

    add_btn(0, 8, 8, 83, 44, menu_main, &sub_trip, bordercolor, bgcolor);
    s65_drawText( 28, 21, "Старт", 1, textcolor, bgcolor);

    add_btn(1, 93, 8, 168, 44, menu_main, &sub_errors, bordercolor, bgcolor);
    s65_drawText( 108, 21, "Ошибки", 1, textcolor, bgcolor);

    add_btn(2, 8, 56, 83, 92, menu_main, &sub_adc, bordercolor, bgcolor);
    s65_drawText( 36, 69, "АЦП", 1, textcolor, bgcolor);

    add_btn(3, 93, 56, 168, 92, menu_main, &sub_accel, bordercolor, bgcolor);
    s65_drawText( 96, 69, "Ускорение", 1, textcolor, bgcolor);

    textcolor = RGB(0x1E, 0x00, 0x00);

    add_btn(4, 8, 104, 83, 124, menu_main, &sub_config, bordercolor, bgcolor);
    s65_drawText( 11, 110, "Настройка", 1, textcolor, bgcolor);

    add_btn(5, 93, 104, 168, 124, menu_main, &sub_system, bordercolor, bgcolor);
    s65_drawText( 106, 110, "Система", 1, textcolor, bgcolor);

    pmenu = (PMENU)&menu_main;
    menu_size = def_MainMenuSize;

    worker_t0_init(10, &worker_dummy);
}


uint8_t worker_trip_mode;

#define def_trip_RPM_MODE		0
#define def_trip_VCC_MODE		1
#define def_trip_TEMP_MODE		2
#define def_trip_THROTTLE_MODE		3
#define def_trip_INJ_MODE		4
#define def_trip_FUEL_MODE		5

void worker_trip(void){

    textcolor = RGB(0x0F, 0x1F,0x0F);
    bordercolor = RGB(0x0F, 0x1F,0x0F);

    switch(worker.update_sm) {
	case 0:
	    s65_drawText( 164, 28, test_connection, 1, RGB(0x0F, 0x1F, 0x0F), bgcolor);
	    buff_pkt_ready = 0;
	    ecu_get_rli_ass();
	    worker.update_sm++;
	    break;
	case 1:
	    switch(buff_pkt_ready) {
		case 0:
		    s65_drawText( 164, 28, test_connection, 1, RGB(0x0E, 0x00, 0x00), bgcolor);
		    break;
		case 1:
		    s65_drawText( 164, 28, test_connection, 1, RGB(0x00, 0x1E, 0x00), bgcolor);
		    ecu_parse_rli_ass();

		    sprintf(convert, "P:%6.2f/%-6.2f l/km", ecu_fuel_full/360000, ecu_trip/360000);
		    s65_drawText( 2, 94, convert, 1, RGB(0x1E,0x2E,0x00), bgcolor);

		    sprintf(convert, "%3d kmh", ecu_speed);
		    s65_drawText( 52, 28, convert, 2, RGB(0x00,0x2E,0x00), bgcolor);

		    switch(worker_trip_mode){
			case def_trip_RPM_MODE:
			    sprintf(convert, "+%3.1fV", ecu_vcc);
			    s65_drawText( 3, 6, convert, 1, textcolor, bgcolor);
			    if ((ecu_temp & 0x80) != 0) sprintf(convert, "-%3d&C", 256 - ecu_temp); // если она ниже нуля
			    else sprintf(convert, "+%3d&C", ecu_temp);
			    s65_drawText( 64, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%2d&", ecu_throttle);
			    s65_drawText( 138, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4d", ecu_rpm);
			    s65_drawText( 20, 54, convert, 3, textcolor, bgcolor);
			    sprintf(convert, "%4.1fms", ecu_inj);
			    s65_drawText( 16, 112, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4.1fLx", ecu_fuel);
			    s65_drawText( 104, 112, convert, 1, textcolor, bgcolor);
			    break;
			case def_trip_VCC_MODE:
			    sprintf(convert, "%4d", ecu_rpm);
			    s65_drawText( 3, 6, convert, 1, textcolor, bgcolor);
			    if ((ecu_temp & 0x80) != 0) sprintf(convert, "-%3d&C", 256 - ecu_temp); // если она ниже нуля
			    else sprintf(convert, "+%3d&C", ecu_temp);
			    s65_drawText( 64, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%2d&", ecu_throttle);
			    s65_drawText( 138, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "+%3.1fV", ecu_vcc);
			    s65_drawText( 20, 54, convert, 3, textcolor, bgcolor);
			    sprintf(convert, "%4.1fms", ecu_inj);
			    s65_drawText( 16, 112, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4.1fLx", ecu_fuel);
			    s65_drawText( 104, 112, convert, 1, textcolor, bgcolor);
			    break;
			case def_trip_TEMP_MODE:
			    sprintf(convert, "+%3.1fV", ecu_vcc);
			    s65_drawText( 3, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4d", ecu_rpm);
			    s65_drawText( 64, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%2d%c", ecu_throttle, '&');
			    s65_drawText( 138, 6, convert, 1, textcolor, bgcolor);
			    if ((ecu_temp & 0x80) != 0) sprintf(convert, "-%3d&C", 256 - ecu_temp); // если она ниже нуля
			    else sprintf(convert, "+%3d&C", ecu_temp);
			    s65_drawText( 20, 54, convert, 3, textcolor, bgcolor);
			    sprintf(convert, "%4.1fms", ecu_inj);
			    s65_drawText( 16, 112, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4.1fLx", ecu_fuel);
			    s65_drawText( 104, 112, convert, 1, textcolor, bgcolor);
			    break;
			case def_trip_THROTTLE_MODE:
			    sprintf(convert, "+%3.1fV", ecu_vcc);
			    s65_drawText( 3, 6, convert, 1, textcolor, bgcolor);
			    if ((ecu_temp & 0x80) != 0) sprintf(convert, "-%3d&C", 256 - ecu_temp); // если она ниже нуля
			    else sprintf(convert, "+%3d&C", ecu_temp);
			    s65_drawText( 64, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4d", ecu_rpm);
			    s65_drawText( 138, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%2d&", ecu_throttle);
			    s65_drawText( 20, 54, convert, 3, textcolor, bgcolor);
			    sprintf(convert, "%4.1fms", ecu_inj);
			    s65_drawText( 16, 112, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4.1fLx", ecu_fuel);
			    s65_drawText( 104, 112, convert, 1, textcolor, bgcolor);
			    break;
			case def_trip_INJ_MODE:
			    sprintf(convert, "+%3.1fV", ecu_vcc);
			    s65_drawText( 3, 6, convert, 1, textcolor, bgcolor);
			    if ((ecu_temp & 0x80) != 0) sprintf(convert, "-%3d&C", 256 - ecu_temp); // если она ниже нуля
			    else sprintf(convert, "+%3d&C", ecu_temp);
			    s65_drawText( 64, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%2d&", ecu_throttle);
			    s65_drawText( 138, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4.1fms", ecu_inj);
			    s65_drawText( 20, 54, convert, 3, textcolor, bgcolor);
			    sprintf(convert, "%4d", ecu_rpm);
			    s65_drawText( 16, 112, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4.1fLx", ecu_fuel);
			    s65_drawText( 104, 112, convert, 1, textcolor, bgcolor);
			    break;
			case def_trip_FUEL_MODE:
			    sprintf(convert, "+%3.1fV", ecu_vcc);
			    s65_drawText( 3, 6, convert, 1, textcolor, bgcolor);
			    if ((ecu_temp & 0x80) != 0) sprintf(convert, "-%3d&C", 256 - ecu_temp); // если она ниже нуля
			    else sprintf(convert, "+%3d&C", ecu_temp);
			    s65_drawText( 64, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%2d&", ecu_throttle );
			    s65_drawText( 138, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4.1fLx", ecu_fuel);
			    s65_drawText( 20, 54, convert, 3, textcolor, bgcolor);
			    sprintf(convert, "%4.1fms", ecu_inj);
			    s65_drawText( 16, 112, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4d", ecu_rpm);
			    s65_drawText( 104, 112, convert, 1, textcolor, bgcolor);
			    break;
		    }
		    worker.update_sm = 0;
		    break;
		default:
		    //была ошибка - нужен реинит
		    break;
	    };
	    break;
	default:
	    worker.update_sm = 0;
	    break;
    }
}

void sub_trip_init_btn(void){
    bgcolor = RGB(0x00, 0x00, 0x1E);
    s65_clear(bgcolor);

    textcolor = RGB(0x0F, 0x1F,0x0F);
    bordercolor = RGB(0x0F, 0x1F,0x0F);

    add_btn(0, 0, 0, 52, 24, menu_trip, &sub_trip_vcc, bordercolor, bgcolor);		//VCC
    add_btn(1, 52, 0, 124, 24, menu_trip, &sub_trip_temp, bordercolor, bgcolor);	//Temp
    add_btn(2, 124, 0, 175, 24, menu_trip, &sub_trip_throttle, bordercolor, bgcolor);	//Throttle
    add_btn(3, 0, 24, 175, 106, menu_trip, &sub_main, bordercolor, bgcolor);		//Speed + RPM
    add_btn(4, 0, 106, 87, 131, menu_trip, &sub_trip_inj, bordercolor, bgcolor);	//dT впрыска
    add_btn(5, 87, 106, 175, 131, menu_trip, &sub_trip_fuel, bordercolor, bgcolor);	//расход топлива

    if(worker.update_sm != 0){
	_delay_ms(500);
    };
    worker.update_sm = 0;
}

void sub_trip_rpm(void){
    sub_trip_init_btn();

    s65_drawText( 2, 26, "RPM", 1, RGB(0x1E,0x2E,0x00), bgcolor);

    worker_trip_mode = def_trip_RPM_MODE;
}

void sub_trip_vcc(void){
    sub_trip_init_btn();

    s65_drawText( 2, 26, "VCC", 1, RGB(0x1E,0x2E,0x00), bgcolor);

    pmenu[0].sub_menu = &sub_trip_rpm;
    worker_trip_mode = def_trip_VCC_MODE;
}

void sub_trip_temp(void){
    sub_trip_init_btn();

    s65_drawText( 2, 26, "TEMP", 1, RGB(0x1E,0x2E,0x00), bgcolor);

    pmenu[1].sub_menu = &sub_trip_rpm;
    worker_trip_mode = def_trip_TEMP_MODE;
}

void sub_trip_throttle(void){
    sub_trip_init_btn();

    s65_drawText( 2, 26, "THR", 1, RGB(0x1E,0x2E,0x00), bgcolor);

    pmenu[2].sub_menu = &sub_trip_rpm;
    worker_trip_mode = def_trip_THROTTLE_MODE;
}
void sub_trip_inj(void){
    sub_trip_init_btn();

    s65_drawText( 2, 26, "INJ", 1, RGB(0x1E,0x2E,0x00), bgcolor);

    pmenu[4].sub_menu = &sub_trip_rpm;
    worker_trip_mode = def_trip_INJ_MODE;
}

void sub_trip_fuel(void){
    sub_trip_init_btn();

    s65_drawText( 2, 26, "FUEL", 1, RGB(0x1E,0x2E,0x00), bgcolor);

    pmenu[5].sub_menu = &sub_trip_rpm;
    worker_trip_mode = def_trip_FUEL_MODE;
}

void sub_trip(void){

    if(is_connected == 0)
	if((is_connected = ecu_connect()) != 1) sub_main();

    if(is_connected == 1) {
	sub_trip_rpm();
	pmenu = (PMENU)&menu_trip;
	menu_size = def_tripMenuSize;

	ecu_fuel = 0;

	worker_trip_mode = def_trip_RPM_MODE;
	worker_t0_init( 10, &worker_trip);
    };
}

void sub_errors(void){
    bgcolor = RGB(0x00, 0x00, 0x1E);
    s65_clear(bgcolor);

    textcolor = RGB(0x1E, 0x00, 0x00);

    s65_drawText( 8, 34, "Ошибки", 1, textcolor, bgcolor);

    pmenu = (PMENU)&menu_errors;
    menu_size = def_ErrorsMenuSize;

    worker_t0_init( 10, &worker_test);
}

void sub_adc(void){
    bgcolor = RGB(0x00, 0x00, 0x1E);
    s65_clear(bgcolor);

    textcolor = RGB(0x1E, 0x00, 0x00);

/*

    textcolor = RGB(0x0F, 0x1F,0x0F);
    bordercolor = RGB(0x0F, 0x1F,0x0F);

    switch(worker.update_sm) {
	case 0:
	    s65_drawText( 164, 28, test_connection, 1, RGB(0x0F, 0x1F, 0x0F), bgcolor);
	    ecu_get_rli_ass();
	    worker.update_sm++;
	    break;
	case 1:
	    switch(buff_pkt_ready) {
		case 0:
		    //ждем ответ. нужно добавить обработку таймаута
		    s65_drawText( 164, 28, test_connection, 1, RGB(0x0E, 0x00, 0x00), bgcolor);
		    break;
		case 1:
		    s65_drawText( 164, 28, test_connection, 1, RGB(0x00, 0x1E, 0x00), bgcolor);
		    ecu_parse_rli_ass();

		    ecu_fuel_full = 24.98;
		    ecu_trip = 123.45;

		    sprintf(convert, "Trip:%4.1f/%-6.1f l/km", ecu_fuel_full, ecu_trip);
		    s65_drawText( 2, 94, convert, 1, RGB(0x1E,0x2E,0x00), bgcolor);

		    sprintf(convert, "%3d kmh", ecu_speed);
		    s65_drawText( 52, 28, convert, 2, RGB(0x00,0x2E,0x00), bgcolor);

void ecu_get_rli_ft(void);
void ecu_parse_rli_ft(void);

extern volatile float ecu_adc_maf;
extern volatile float ecu_adc_lambda;
*/

    s65_drawText( 8, 34, "ADC", 1, textcolor, bgcolor);

    pmenu = (PMENU)&menu_adc;
    menu_size = def_AdcMenuSize;

    worker_t0_init( 10, &worker_test);
}

void sub_accel(void){
    bgcolor = RGB(0x00, 0x00, 0x1E);
    s65_clear(bgcolor);

    textcolor = RGB(0x1E, 0x00, 0x00);

    s65_drawText( 8, 34, "Accel", 1, textcolor, bgcolor);

    pmenu = (PMENU)&menu_accel;
    menu_size = def_AccelMenuSize;

    worker_t0_init( 10, &worker_test);
}

void sub_config(void){
    bgcolor = RGB(0x00, 0x00, 0x1E);
    s65_clear(bgcolor);

    textcolor = RGB(0x1E, 0x00, 0x00);

    s65_drawText( 8, 34, "CONFIG", 1, textcolor, bgcolor);

    pmenu = (PMENU)&menu_config;
    menu_size = def_ConfigMenuSize;

    worker_t0_init( 10, &worker_test);
};

void sub_system(void){
    bgcolor = RGB(0x00, 0x00, 0x0E);
    s65_clear(bgcolor);

    bgcolor = RGB(0x00, 0x00, 0x1E);
    textcolor = RGB(0x0F, 0x1F,0x0F);
    bordercolor = RGB(0x00, 0x0,0x00);

    add_btn(0, 50, 30, 126, 60, menu_system, &sub_main, bordercolor, bgcolor);
    s65_drawText( 70, 41, "Reset", 1, textcolor, bgcolor);

    add_btn(1, 50, 76, 126, 106, menu_system, &sub_main, bordercolor, bgcolor);
    s65_drawText( 70, 87, "Sleep", 1, textcolor, bgcolor);

    pmenu = (PMENU)&menu_system;
    menu_size = def_SystemMenuSize;

    worker_t0_init( 10, &worker_test);
};
