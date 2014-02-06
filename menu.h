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
    uint8_t dtime8;
    uint8_t dtime16;
    void (*sub_worker)(void);
    uint8_t update;
    uint8_t update_sm;
} WORKER, *PWORKER;

void worker_t0_init(uint8_t dt8, uint16_t dt16, void * sub);
void add_btn(uint8_t idx, uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, PMENU menu, void * sub, uint16_t border, uint16_t bg);

void sub_main(void);
void sub_trace(void);
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
