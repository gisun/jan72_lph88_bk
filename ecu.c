#include "device.h"

#include <inttypes.h>

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>

#include <avr/interrupt.h>

#include "s65_lhp88.h"

#include "init.h"
#include "vars.h"
#include "ecu.h"


/*
uint8_t startCommunication[]                 = {0x81, 0x10,0xF1, 0x81,			0x03};
uint8_t readDataByLocalIdentifier_RLI_ASS[]  = {0x82, 0x10,0xF1, 0x21,0x01,		0xA5};
uint8_t readDTCByStatus[]                    = {0x84, 0x10,0xF1, 0x18,0x00,0x00,0x00,	0x9D};	//считать ошибки систмы управления двигателем и трансмиссии
uint8_t clearDiagnosticInformation[]         = {0x83, 0x10,0xf1, 0x14,0x00,0x00,	0x98};		//стереть ошибки систмы управления двигателем и трансмиссии
uint8_t testerPresent[]                      = {0x82, 0x10,0xf1, 0x3E,0x01,		0xC2};		//сообщить ецу о подключении и настроить канал связи
uint8_t readDataByLocalIdentifier_RLI_FT[]   = {0x82, 0x10,0xF1, 0x21,0x03,		0xA7};
*/

uint8_t startCommunication[]                 = {0x81};

uint8_t readDataByLocalIdentifier_RLI_ASS[]  = {0x21,0x01};
uint8_t readDTCByStatus[]                    = {0x18,0x00,0x00,0x00};		//считать ошибки систмы управления двигателем и трансмиссии
uint8_t clearDiagnosticInformation[]         = {0x14,0x00,0x00};		//стереть ошибки систмы управления двигателем и трансмиссии
uint8_t testerPresent[]                      = {0x3E,0x01};			//сообщить ецу о подключении и настроить канал связи
uint8_t readDataByLocalIdentifier_RLI_FT[]   = {0x21,0x03};


volatile uint8_t buffer[BUFFER_SIZE];

volatile uint8_t crc;

volatile uint8_t pkt_idx;

volatile uint8_t buff_idx;
volatile uint8_t buff_size;
volatile uint8_t buff_pkt_ready;

uint16_t bgcolor;
uint16_t textcolor;

//recive byte vector
ISR(USART_RX_vect) {

    uint8_t status, data;

    status = UCSR0A;
    data = UDR0;

    if ((status & (_BV(FE0) | _BV(UPE0) | _BV(DOR0))) == 0) {
	switch(pkt_idx){
	    case idx_FMT:
			crc = data;
			if((data & 0xC0) == 0x80){
			    buff_size = data & 0x3F;	// FMT header have A1A0L5L4L3L2L1L0 format
							// there  A1A0 anyway can be 0b10
							// and
							// if L5..L0 is not 0 - it mean it show pkt size (from header end till crc byte)
			    pkt_idx++;
			} else pkt_idx = 0;		// wrong FMT byte in pkt. wait next
			break;
	    case idx_TGT:
			crc += data;
			if (data == addr_TST) pkt_idx++;
			else pkt_idx = 0;
			break;
	    case idx_SRC:
			crc += data;
			if (data == addr_ECU) pkt_idx++;
			else pkt_idx = 0;
			break;
	    default:
			if((buff_size == 0) && (pkt_idx == idx_LEN)){
			    buff_size = data;
			    crc += data;
			} else {
			    if(buff_idx == (buff_size)){
				if(data == crc){
				    buff_pkt_ready = 1;
				    UCSR0B =  0;	//setup USART mode
				} else {
				    buff_pkt_ready = 2;	//wrong crc
				    UCSR0B =  0;	//setup USART mode
				}
			    } else {
				crc += data;
				buffer[buff_idx++] = data;
			    }
			}
			break;
	}
    } else {
	//k-line error
	buff_pkt_ready = 3;
	//setup USART mode
	UCSR0B =  0;
    }
    if((buff_pkt_ready > 1) || (kline_delay > def_KLINE_DELAY)) need_reset = 1;
}

//transmite complite vector
ISR(USART_UDRE_vect) {

    int i;

    if(buff_idx < buff_size) {
	UDR0 = buffer[buff_idx++];
    }
    else {
	buff_idx = 0;
	buff_size = 0;
	buff_pkt_ready = 0;
	pkt_idx = 0;

	kline_delay = 0;

	for(i = 0 ; i < 100 ; i++){
	    asm volatile("nop");
	}

	//setup USART mode for get answer
	UCSR0B =  _BV(RXCIE0) | _BV(RXEN0);// 0x98 == RX complete int enable | RX enable | TX enable
    }
}


void ecu_SendCmd (uint8_t *data, uint8_t length) {
    uint8_t i;

    if((length < 64) && (length > 0)) {
	buffer[0] = 0x80 | length; crc = buffer[0];
	buffer[1] = addr_ECU; crc += buffer[1];	//Target
	buffer[2] = addr_TST; crc += buffer[2];	//Source
	//Data
	for(i = 0 ; i < length ; i++){ buffer[i + 3] = data[i]; crc += data[i]; }
	buffer[i + 3] = crc; buff_idx = 0; buff_size = length + 3 + 1;
    } else {
	buffer[0] = 0x80; crc = buffer[0];	//long pkt. 4th byte is size, 1t 0x80
	buffer[1] = addr_ECU; crc += buffer[1];	//Target
	buffer[2] = addr_TST; crc += buffer[2];	//Source
	buffer[3] = length; crc += buffer[3];	//Len
	//Data
	for(i = 0 ; i < length ; i++){ buffer[i + 4] = data[i]; crc += data[i]; }
	buffer[i + 4] = crc; buff_idx = 0; buff_size = length + 4 + 1;
    }
    UCSR0B = _BV(UDRIE0) | _BV(TXEN0);//TX int enable |  TX enable
}

uint8_t ecu_connect(void){

    uint8_t j;

    uint8_t sm_state = 0;

    buff_pkt_ready = 0;
    sm_state = 0;

    bgcolor = RGB(0x00, 0x00, 0x0E);
    textcolor = RGB(0x0F, 0x1F,0x0F);

    for(j = 0 ; j < 5 ; j++){

	s65_clear(bgcolor);
	s65_drawText( 20, 50, "Checking", 2, textcolor, bgcolor);

	usart_init();

	_delay_ms(300);

	ecu_SendCmd( startCommunication, 1);

	_delay_ms(100);

	// парсим ответ
	switch(buff_pkt_ready){
	    case 1:
		s65_clear(bgcolor);

		if ( (buffer[0] == 0xC1)) {
		    // положительный ответ startCommunication
		    s65_drawText( 20, 50, "Connected", 2, textcolor, bgcolor);
		    sm_state = 1;
		    j = 5;
		} else s65_drawText( 20, 50, "PKT error", 2,  RGB(0x1E, 0x00,0x00), bgcolor);
		break;
	    case 2:
		s65_clear(bgcolor);
		s65_drawText( 20, 50, "CRC error", 2, RGB(0x1E, 0x00,0x00), bgcolor);
//		sprintf(convert, "%2.2X %2.2X %2.2X %2.2X %2.2X", buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);
		break;
	    case 3:
		s65_clear(bgcolor);
		s65_drawText( 20, 50, "KLine err", 2, RGB(0x1E, 0x00,0x00), bgcolor);
//		sprintf(convert, "%2.2X %2.2X %2.2X %2.2X %2.2X", buffer[0],buffer[1],buffer[2],buffer[3],buffer[4]);
		break;
	    default:
		s65_clear(bgcolor);
		s65_drawText( 35, 50, "Timeout", 2, RGB(0x1E, 0x00,0x00), bgcolor);
		break;
	}
	if(sm_state == 0 ){
//	    s65_drawText( 60, 50, convert, 1, RGB(0x1E, 0x00,0x00), bgcolor);
	    usart_deinit();
	    _delay_ms(3000);
	}
    }

    return sm_state;
}


void ecu_get_rli_ass(void){
    ecu_SendCmd(readDataByLocalIdentifier_RLI_ASS, 2);
}

void ecu_parse_rli_ass(void){
    // длина пакета данных (4-й байт в посылке) otvet_length = buffer[9];
    // -10

    // напряжение
    ecu_vcc = (5.2 + (float) buffer[20] / 20);

    // вычисляем температуру двигателя
    ecu_temp = buffer[10] - 0x28;
    if ((ecu_temp & 0x80) != 0) ecu_temp = 256 - ecu_temp;	// если она ниже нуля
    ecu_throttle = buffer[12];					// положение дроссельной заслонки

    //обороты
    if (ecu_throttle != 0) ecu_rpm = (buffer[13]) * 40;		// обороты если НЕ холостой ход
    else ecu_rpm = (buffer[14]) * 10;				// обороты на холостом ходу

    // скорость
    ecu_speed = buffer[19];

    // длительность впрыска
    ecu_inj = (float) ((buffer[25] << 8) + buffer[24]) / 125;

    // путевой расход топлива
    ecu_oil = abs((float) ((buffer[33] << 8) + buffer[32]) / 128);
}

void ecu_get_rli_ft(void){
    ecu_SendCmd(readDataByLocalIdentifier_RLI_FT, 2);
}

void ecu_parse_rli_ft(void){
    //  6 байт запрос + 18 байт ответ (4-й байт в ответе - число байтов в посылке)
    // -10

    ecu_adc_maf = (float) (buffer[4] * 5) / 256;	// напряжение ДМРВ
    ecu_adc_lambda = (float) (buffer[6] * 5) / 256;	// напряжение на датчике кислорода
}


