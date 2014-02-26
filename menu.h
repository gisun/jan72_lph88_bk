#ifndef _MENU_H
#define _MENU_H

extern uint16_t textcolor;
extern uint16_t bordercolor;
extern uint16_t bgcolor;

typedef struct menu{
    uint8_t x1,y1,x2,y2;
    void (*sub_menu)(void);
} MENU, *PMENU;

typedef struct worker{
    void (*sub_worker)(void);
    uint8_t update;
    uint8_t update_sm;
    uint8_t t0_dtime;
} WORKER, *PWORKER;

void worker_t0_init(uint8_t dt_t0, void * sub);

void add_btn(uint8_t idx, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, PMENU menu, void * sub, uint16_t border, uint16_t bg);

void sub_main(void);

void sub_trip_init_btn(void);

void sub_trip_rpm(void);
void sub_trip_vcc(void);
void sub_trip_temp(void);
void sub_trip_throttle(void);
void sub_trip_inj(void);
void sub_trip_fuel(void);

void sub_trip(void);
void sub_errors(void);
void sub_adc(void);
void sub_accel(void);
void sub_config(void);
void sub_system(void);

void worker_dummy(void);
void worker_test(void);



extern PMENU pmenu;
extern WORKER worker;
extern uint8_t menu_size;
extern uint8_t cur_menu;

#endif
