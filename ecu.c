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

ISR(USART_UDRE_vect) {
    int i;

    if(buff_idx < buff_size) UDR0 = buffer[buff_idx++];
    else {
	buff_idx = 0;
	buff_size = 0;
	buff_pkt_ready = 0;
	pkt_idx = 0;

	kline_delay = 0;

	for(i = 0 ; i < 100 ; i++) {
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
	_delay_ms(500);
	ecu_SendCmd( startCommunication, 1);
	_delay_ms(200);
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
		break;
	    case 3:
		s65_clear(bgcolor);
		s65_drawText( 20, 50, "KLine err", 2, RGB(0x1E, 0x00,0x00), bgcolor);
		break;
	    default:
		s65_clear(bgcolor);
		s65_drawText( 35, 50, "Timeout", 2, RGB(0x1E, 0x00,0x00), bgcolor);
		break;
	}
	if(sm_state == 0 ){ usart_deinit(); _delay_ms(2000);
	}
    }
    return sm_state;
}


void ecu_get_rli_ass(void){
    ecu_SendCmd(readDataByLocalIdentifier_RLI_ASS, 2);
}

void ecu_parse_rli_ass(void){
    uint32_t tmp;
    float tmp_trip;
    float tmp_fuel;

    // длина пакета данных (4-й байт в посылке) otvet_length = buffer[9];
    // -10

    // напряжение
    ecu_vcc = (5.2 + (float) buffer[20] / 20);

    // вычисляем температуру двигателя
    ecu_temp = buffer[10] - 0x28;

    ecu_throttle = buffer[12];					// положение дроссельной заслонки

    //обороты
    if (ecu_throttle != 0){
	 ecu_rpm = (buffer[13]) * 40;				// обороты если НЕ холостой ход
    } else {
	ecu_rpm = (buffer[14]) * 10;				// обороты на холостом ходу
    }

    // скорость
    ecu_speed = buffer[19];

    asm volatile("cli");
    tmp = t0_timer - t0_timer_last;
    t0_timer_last = t0_timer;
    asm volatile("sei");

    ecu_trip += tmp * ecu_speed;

    if(ecu_speed != 0){
	// путевой расход топлива
	ecu_fuel = abs((float) ((buffer[33] << 8) + buffer[32]) / 128);	// L/100km
	ecu_fuel_full += abs((float) ( ((buffer[33] << 8) + buffer[32]) * tmp * ecu_speed / 12800));
    } else {
	// часовой расход топлива
	ecu_fuel = abs((float) ((buffer[31] << 8) + buffer[30]) / 50);		// L/h
	ecu_fuel_full += abs((float) ( ((buffer[31] << 8) + buffer[30]) * tmp / 50));
    }


    // длительность впрыска
    ecu_inj = (float) ((buffer[25] << 8) + buffer[24]) / 125;

/*
    if(ecu_fuel_full == 0) ecu_fuel_full = ecu_fuel_tmp;
    else ecu_fuel_full = (ecu_fuel_full + ecu_fuel_tmp) / 2;
*/

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

void ecu_read_dct(void){
    ecu_SendCmd(readDTCByStatus, 4);
}

void ecu_parse_dct(void){
    //  8 байт запрос + 18 байт ответ (4-й байт в ответе - число байтов в посылке)
    // -10
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
    // !!!!!!!!!!! ответ может быть произвольной длинны !!!!!!!!!!!!!!
    if (counter>12) {                          // парсим  запрос + ответ
	unsigned char cislo_oshibok = 0;
	unsigned char otvet_length = 0;

	cislo_oshibok = buffer[12];              // смотрим число ошибок
	otvet_length = 13 + (3 * cislo_oshibok); // длинна ответа
	if (counter > otvet_length) {            // ждем пока примется весь пакет
	    unsigned char i = 0;
	    unsigned int crc = 0;

	    for (i=8;i<otvet_length;i++) crc = crc + buffer[i];

	    i =  crc & 0xFF;                       // берем 2 младших разряда 
	    if ( buffer[otvet_length] == i ) {     // проверяем контрольную сумму
		sprintf(convert,"%d",cislo_oshibok);
		put_string(150,5,convert,text_parametr,1);
		if (cislo_oshibok != 0) {
		    for (i=0;i < cislo_oshibok;i++) {
			// за раз можежем вывести на экран максимум 15 ошибок (3 столбика по 5 ошибок)
			sprintf(convert,"P%02x%02x",buffer[(13+(i*3))],buffer[(14+(i*3))]);
			if (i < 5) {
			    put_string(5,25+(i*15),convert,0xAFE0,1);
			} else
			    if (i>=5 && i<10) {
				put_string(65,25+((i-5)*15),convert,0xAFE0,1);
			    } else
				if (i>=10 && i<15) put_string(125,25+((i-10)*15),convert,0xAFE0,1);
		    }
		}
	    }
	    counter = 0;                             // сбрасываем counter иначе опять попадем под условие if (counter>12)
	}
    }
*/

void ecu_clear_diag_info(void){
    ecu_SendCmd(clearDiagnosticInformation, 3);
}

void ecu_parse_clear_diag_info(void){

}



/*
		if (push && !press) {
		    if ((x > 61) && (x < 115) && (y > 108) && (y < 124)) {  // если попали на кнопку "сброс"
			draw_rect(53,105,123,127,0x07E0);            // меняем цвет кнопки 
			bgcolor = 0x07E0;
			put_string(65,112,"Reset",text_tablo,1);
			SendCommand(clearDiagnosticInformation,7);   // стираем ошибки
			_delay_ms(200);                               // даём ЭБУ время на стирание ошибок (хз зачем, но мне показалось так лучше срабатывает)
			first_draw = 1;                              // перерисовываем экран
		    } else {
			first_draw = 1;
			mode = Menu;
		    }
		}
		break;
*/

/*
	    case SpeedSample:                              // если режим замера скорости 0-100 км/ч
		if (first_draw == 1) {
		    fill_screen(bgcolor);
		    put_string(15,5,"Acceleration 0-100 km/h",text_tablo,1);
		    put_string(65,25,"0.0",0xAFE0,2);
		    put_string(50,57,"Speed",text_tablo,1); 

		    draw_rect(53,105,123,127,0xF800);
		    bgcolor = 0xF800;
		    put_string(65,112,"Reset",text_tablo,1);

		    first_draw = 0;
		}
		if (StartSend == 1) {
		    SendCommand(readDataByLocalIdentifier_RLI_ASS,6);
		    StartSend = 0;
		} else {
		    if (counter>10) {                          //  если начали принимать ответ
			unsigned char i = 0;
			unsigned int crc = 0;
			unsigned char speed = 0;
			unsigned char otvet_length = 0;
			otvet_length = buffer[9];                // длина пакета данных (4-й байт в посылке)
			if (counter > (otvet_length+10)) {       // ждем пока примется вся посылка
			    for (i=6;i<(otvet_length+10);i++) crc = crc + buffer[i];                 // считаем контрольную сумму
			    i =  crc & 0xFF;                         // берем 2 младших разряда 
			    if ( buffer[(otvet_length+10)] == i ) {  // проверяем контрольную сумму 
				speed = buffer[29];
				if (speed == 0) go = 1;              // если машина остановлена разрешаем замер скорости
				if (speed > 0 && go == 1) {            // начали разгоняться
				    if (zamer_finish) {
					TCCR1B=0x00;
				    } else {
					TCCR1B=0x04;                       // запустили отсчет времени  
					if (speed >= 100) {                // замер разгона до 100 км/ч
					    zamer_finish = 1;
					    TCCR1B=0x00;
					}
				    }
				}
				sprintf(convert,"%.1f",((float) (dsec * 0.1)));  // выводим время
				bgcolor = svetlii_color;
				put_string(65,25,convert,0xAFE0,2);
				sprintf(convert,"%d",speed);                     // показываем текущую скорость
				if (strlen(convert) < 3) put_string(67,75,strcat(convert, " "),0xAFE0,2);
				else put_string(67,75,convert,0xAFE0,2);
			    }
			    counter = 0;                               // сбрасываем counter иначе опять попадем под условие if (counter > (otvet_length+10))
			}
		    }
		}
*/

