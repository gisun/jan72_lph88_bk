#ifndef _ECU_H
#define _ECU_H

#define idx_FMT         0
#define idx_TGT         1
#define idx_SRC         2

#define idx_LEN         3

#define idx_SLD3        3
#define idx_SLD4        4


#define addr_ECU	0x10
#define addr_TST	0xF1
#define addr_IMO	0xC0

#define BUFFER_SIZE	255
#define def_KLINE_DELAY 100

void timer0_start(void);
void timer0_stop(void);

void ecu_SendCmd (prog_uint8_t *command, uint8_t length);

uint8_t ecu_connect();

void ecu_get_rli_ass(void);
void ecu_parse_rli_ass(void);
void ecu_get_rli_ft(void);
void ecu_parse_rli_ft(void);

extern volatile uint8_t pkt_idx;

extern volatile uint8_t buff_idx;
extern volatile uint8_t buff_size;
extern volatile uint8_t buff_pkt_ready;

extern volatile uint8_t buffer[];

#endif


