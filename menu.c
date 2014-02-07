#include "device.h"

#include <inttypes.h>

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>

#include "init.h"
#include "s65_lhp88.h"

#include "menu.h"
#include "ecu.h"
#include "timers.h"

#include "vars.h"

uint16_t textcolor;
uint16_t bordercolor;
uint16_t bgcolor;

//uint8_t convert[32];

#define def_MainMenuSize 6
MENU menu_main[def_MainMenuSize];

#define def_TraceMenuSize 6
MENU menu_trace[def_TraceMenuSize];

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

void worker_t0_init(uint8_t dt8, uint16_t dt16, void * sub){
    timer0_stop();
    worker.dtime8 = dt8;
    worker.dtime16 = dt16;
    worker.sub_worker = sub;
    worker.update = 0;
    worker.update_sm = 0;
    timer0_start();
}

void worker_test(void){
//	sprintf(convert,"%2.2d:%2.2d:%2.2d",t_hour,t_min,t_sec);
//	s65_drawText( 100, 0, convert, 1, RGB(0x1E,0x2E,0x1E), RGB(0x00,0x00,0x0E));
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

    add_btn(0, 8, 8, 83, 44, menu_main, &sub_trace, bordercolor, bgcolor);
    s65_drawText( 28, 21, "Trace", 1, textcolor, bgcolor);

    add_btn(1, 93, 8, 168, 44, menu_main, &sub_errors, bordercolor, bgcolor);
    s65_drawText( 108, 21, "Errors", 1, textcolor, bgcolor);

    add_btn(2, 8, 56, 83, 92, menu_main, &sub_adc, bordercolor, bgcolor);
    s65_drawText( 36, 69, "ADC", 1, textcolor, bgcolor);

    add_btn(3, 93, 56, 168, 92, menu_main, &sub_accel, bordercolor, bgcolor);
    s65_drawText( 112, 69, "Accel", 1, textcolor, bgcolor);

    textcolor = RGB(0x1E, 0x00, 0x00);

    add_btn(4, 8, 104, 83, 124, menu_main, &sub_config, bordercolor, bgcolor);
    s65_drawText( 22, 110, "Config", 1, textcolor, bgcolor);

    add_btn(5, 93, 104, 168, 124, menu_main, &sub_system, bordercolor, bgcolor);
    s65_drawText( 106, 110, "System", 1, textcolor, bgcolor);

    pmenu = (PMENU)&menu_main;
    menu_size = def_MainMenuSize;

    worker_t0_init(10,0,&worker_test);
}


uint8_t worker_trace_mode;

#define def_trace_RPM_MODE		0
#define def_trace_VCC_MODE		1
#define def_trace_TEMP_MODE		2
#define def_trace_THROTTLE_MODE		3
#define def_trace_INJ_MODE		4
#define def_trace_OIL_MODE		5

void worker_trace(void){

    uint8_t test_connection[] = {248, 0};
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

		    sprintf(convert, "%-4.1fL/100", ecu_full_oil);
		    s65_drawText( 2, 94, convert, 1, RGB(0x1E,0x2E,0x00), bgcolor);
		    sprintf(convert, "%3d kmh", ecu_speed);
		    s65_drawText( 52, 28, convert, 2, RGB(0x00,0x2E,0x00), bgcolor);

		    switch(worker_trace_mode){
			case def_trace_RPM_MODE:
			    sprintf(convert, "+%3.1fV", ecu_vcc);
			    s65_drawText( 3, 6, convert, 1, textcolor, bgcolor);
			    if ((ecu_temp & 0x80) != 0) sprintf(convert, "-%3d%cC", 256 - ecu_temp, 248); // если она ниже нуля
			    else sprintf(convert, "+%3d%cC", ecu_temp, 248);
			    s65_drawText( 64, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%2d%c", ecu_throttle, 248);
			    s65_drawText( 138, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4d", ecu_rpm);
			    s65_drawText( 20, 58, convert, 3, textcolor, bgcolor);
			    sprintf(convert, "%4.1fms", ecu_inj);
			    s65_drawText( 16, 112, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4.1fL", ecu_oil);
			    s65_drawText( 104, 112, convert, 1, textcolor, bgcolor);
			    break;
			case def_trace_VCC_MODE:
			    sprintf(convert, "%4d", ecu_rpm);
			    s65_drawText( 3, 6, convert, 1, textcolor, bgcolor);
			    if ((ecu_temp & 0x80) != 0) sprintf(convert, "-%3d%cC", 256 - ecu_temp, 248); // если она ниже нуля
			    else sprintf(convert, "+%3d%cC", ecu_temp, 248);
			    s65_drawText( 64, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%2d%c", ecu_throttle, 248);
			    s65_drawText( 138, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "+%3.1fV", ecu_vcc);
			    s65_drawText( 20, 58, convert, 3, textcolor, bgcolor);
			    sprintf(convert, "%4.1fms", ecu_inj);
			    s65_drawText( 16, 112, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4.1fL", ecu_oil);
			    s65_drawText( 104, 112, convert, 1, textcolor, bgcolor);
			    break;
			case def_trace_TEMP_MODE:
			    sprintf(convert, "+%3.1fV", ecu_vcc);
			    s65_drawText( 3, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4d", ecu_rpm);
			    s65_drawText( 64, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%2d%c", ecu_throttle, 248);
			    s65_drawText( 138, 6, convert, 1, textcolor, bgcolor);
			    if ((ecu_temp & 0x80) != 0) sprintf(convert, "-%3d%cC", 256 - ecu_temp, 248); // если она ниже нуля
			    else sprintf(convert, "+%3d%cC", ecu_temp, 248);
			    s65_drawText( 20, 58, convert, 3, textcolor, bgcolor);
			    sprintf(convert, "%4.1fms", ecu_inj);
			    s65_drawText( 16, 112, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4.1fL", ecu_oil);
			    s65_drawText( 104, 112, convert, 1, textcolor, bgcolor);
			    break;
			case def_trace_THROTTLE_MODE:
			    sprintf(convert, "+%3.1fV", ecu_vcc);
			    s65_drawText( 3, 6, convert, 1, textcolor, bgcolor);
			    if ((ecu_temp & 0x80) != 0) sprintf(convert, "-%3d%cC", 256 - ecu_temp, 248); // если она ниже нуля
			    else sprintf(convert, "+%3d%cC", ecu_temp, 248);
			    s65_drawText( 64, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4d", ecu_rpm);
			    s65_drawText( 138, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%2d%c", ecu_throttle, 248);
			    s65_drawText( 20, 58, convert, 3, textcolor, bgcolor);
			    sprintf(convert, "%4.1fms", ecu_inj);
			    s65_drawText( 16, 112, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4.1fL", ecu_oil);
			    s65_drawText( 104, 112, convert, 1, textcolor, bgcolor);
			    break;
			case def_trace_INJ_MODE:
			    sprintf(convert, "+%3.1fV", ecu_vcc);
			    s65_drawText( 3, 6, convert, 1, textcolor, bgcolor);
			    if ((ecu_temp & 0x80) != 0) sprintf(convert, "-%3d%cC", 256 - ecu_temp, 248); // если она ниже нуля
			    else sprintf(convert, "+%3d%cC", ecu_temp, 248);
			    s65_drawText( 64, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%2d%c", ecu_throttle, 248);
			    s65_drawText( 138, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4.1fms", ecu_inj);
			    s65_drawText( 20, 58, convert, 3, textcolor, bgcolor);
			    sprintf(convert, "%4d", ecu_rpm);
			    s65_drawText( 16, 112, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4.1fL", ecu_oil);
			    s65_drawText( 104, 112, convert, 1, textcolor, bgcolor);
			    break;
			case def_trace_OIL_MODE:
			    sprintf(convert, "+%3.1fV", ecu_vcc);
			    s65_drawText( 3, 6, convert, 1, textcolor, bgcolor);
			    if ((ecu_temp & 0x80) != 0) sprintf(convert, "-%3d%cC", 256 - ecu_temp, 248); // если она ниже нуля
			    else sprintf(convert, "+%3d%cC", ecu_temp, 248);
			    s65_drawText( 64, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%2d%c", ecu_throttle, 248);
			    s65_drawText( 138, 6, convert, 1, textcolor, bgcolor);
			    sprintf(convert, "%4.1fL", ecu_oil);
			    s65_drawText( 20, 58, convert, 3, textcolor, bgcolor);
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

void sub_trace_init_btn(void){
    bgcolor = RGB(0x00, 0x00, 0x1E);
    s65_clear(bgcolor);

    textcolor = RGB(0x0F, 0x1F,0x0F);
    bordercolor = RGB(0x0F, 0x1F,0x0F);

    //VCC
    add_btn(0, 0, 0, 52, 24, menu_trace, &sub_trace_vcc, bordercolor, bgcolor);

    //Temp
    add_btn(1, 52, 0, 124, 24, menu_trace, &sub_trace_temp, bordercolor, bgcolor);

    //Throttle
    add_btn(2, 124, 0, 175, 24, menu_trace, &sub_trace_throttle, bordercolor, bgcolor);

    //Speed + RPM
    add_btn(3, 0, 24, 175, 106, menu_trace, &sub_main, bordercolor, bgcolor);

    //dT впрыска
    add_btn(4, 0, 106, 87, 131, menu_trace, &sub_trace_inj, bordercolor, bgcolor);

    //расход топлива
    add_btn(5, 87, 106, 175, 131, menu_trace, &sub_trace_oil, bordercolor, bgcolor);
}

void sub_trace_rpm(void){
    sub_trace_init_btn();

    s65_drawText( 2, 26, "RPM", 1, RGB(0x1E,0x2E,0x00), bgcolor);

    worker_trace_mode = def_trace_RPM_MODE;
}

void sub_trace_vcc(void){
    sub_trace_init_btn();

    s65_drawText( 2, 26, "VCC", 1, RGB(0x1E,0x2E,0x00), bgcolor);

    pmenu[0].sub_menu = &sub_trace_rpm;
    worker_trace_mode = def_trace_VCC_MODE;
}

void sub_trace_temp(void){
    sub_trace_init_btn();

    s65_drawText( 2, 26, "TEMP", 1, RGB(0x1E,0x2E,0x00), bgcolor);

    pmenu[1].sub_menu = &sub_trace_rpm;
    worker_trace_mode = def_trace_TEMP_MODE;
}

void sub_trace_throttle(void){
    sub_trace_init_btn();

    s65_drawText( 2, 26, "THR", 1, RGB(0x1E,0x2E,0x00), bgcolor);

    pmenu[2].sub_menu = &sub_trace_rpm;
    worker_trace_mode = def_trace_THROTTLE_MODE;
}
void sub_trace_inj(void){
    sub_trace_init_btn();

    s65_drawText( 2, 26, "INJ", 1, RGB(0x1E,0x2E,0x00), bgcolor);

    pmenu[4].sub_menu = &sub_trace_rpm;
    worker_trace_mode = def_trace_INJ_MODE;
}

void sub_trace_oil(void){
    sub_trace_init_btn();

    s65_drawText( 2, 26, "OIL", 1, RGB(0x1E,0x2E,0x00), bgcolor);

    pmenu[5].sub_menu = &sub_trace_rpm;
    worker_trace_mode = def_trace_OIL_MODE;
}

void sub_trace(void){

    if(is_connected == 0)
	if((is_connected = ecu_connect()) != 1) sub_main();

    if(is_connected == 1) {
	sub_trace_rpm();
	pmenu = (PMENU)&menu_trace;
	menu_size = def_TraceMenuSize;

	ecu_oil = 0;
	ecu_oil_tmp = 0;
	ecu_oil_cnt = 0;

	worker_trace_mode = def_trace_RPM_MODE;
	worker_t0_init( 250, 0, &worker_trace);
    };
}

void sub_errors(void){
    bgcolor = RGB(0x00, 0x00, 0x1E);
    s65_clear(bgcolor);

    textcolor = RGB(0x1E, 0x00, 0x00);

    s65_drawText( 8, 34, "ERRORS", 1, textcolor, bgcolor);

    pmenu = (PMENU)&menu_errors;
    menu_size = def_ErrorsMenuSize;

    worker_t0_init(100,0,&worker_test);
}

void sub_adc(void){
    bgcolor = RGB(0x00, 0x00, 0x1E);
    s65_clear(bgcolor);

    textcolor = RGB(0x1E, 0x00, 0x00);

    s65_drawText( 8, 34, "ADC", 1, textcolor, bgcolor);

    pmenu = (PMENU)&menu_adc;
    menu_size = def_AdcMenuSize;

    worker_t0_init(100,0,&worker_test);
}

void sub_accel(void){
    bgcolor = RGB(0x00, 0x00, 0x1E);
    s65_clear(bgcolor);

    textcolor = RGB(0x1E, 0x00, 0x00);

    s65_drawText( 8, 34, "ACCEL", 1, textcolor, bgcolor);

    pmenu = (PMENU)&menu_accel;
    menu_size = def_AccelMenuSize;

    worker_t0_init(100,0,&worker_test);
}

void sub_config(void){
    bgcolor = RGB(0x00, 0x00, 0x1E);
    s65_clear(bgcolor);

    textcolor = RGB(0x1E, 0x00, 0x00);

    s65_drawText( 8, 34, "CONFIG", 1, textcolor, bgcolor);

    pmenu = (PMENU)&menu_config;
    menu_size = def_ConfigMenuSize;

    worker_t0_init(100,0,&worker_test);
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

    worker_t0_init(100,0,&worker_test);
};
