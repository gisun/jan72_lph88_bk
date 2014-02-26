#include <inttypes.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include "s65_lcd.h"
#include "device.h"

void usart_init(void){
    // USART initialization
    // Communication Parameters: 8 Data, 1 Stop, No Parity
    // USART Receiver: On
    // USART Transmitter: On
    // USART Mode: Asynchronous
    // USART Baud Rate: 10400

    UCSR0A = 0x00;

    //setup USART mode
    //_BV(RXCIE0) | _BV(RXEN0) | _BV(TXEN0);	// 0x98 == RX complete int enable | RX enable | TX enable

    UCSR0B =  0x00;

    //8N1
    UCSR0C = _BV(UCSZ00) | _BV(UCSZ01);	//0x86;	//

    //BAUD == 10400
    UBRR0H = 0x00;
    UBRR0L = 0x5F;
}


void usart_deinit(void){
    // USART initialization
    // Communication Parameters: 8 Data, 1 Stop, No Parity
    // USART Receiver: On
    // USART Transmitter: On
    // USART Mode: Asynchronous
    // USART Baud Rate: 10400
    UCSR0A=0x00;
    UCSR0B=0x00;	// RX complete int enable | RX enable TX enable
    UCSR0C=00;		//0x86;	//
    UBRR0H=0x00;
    UBRR0L=0x00;
}


void port_init() {
//    uint8_t tmp;
    asm volatile("cli");

    PORTD = 0x00;
    DDRD = 0x00;

    PORTD &= ~_BV(LCD_RESET);
    DDRD |= _BV(LCD_RESET);

    PORTB |= _BV(LCD_MOSI);
    DDRB |= _BV(LCD_MOSI);

    PORTD &= ~_BV(LCD_CS);
    DDRD |= _BV(LCD_CS);

    PORTB &= ~_BV(LCD_SCK);
    DDRB |= _BV(LCD_SCK);

    PORTB |= _BV(LCD_MISO);

    PORTD |= _BV(LCD_RS);  // not used from LPH display
    DDRD |= _BV(LCD_RS);

    //LED
    PORTB |= _BV(LCD_LED);
    DDRB |= _BV(LCD_LED);

    // setup SPI Interface
    SPCR = _BV(MSTR) | _BV(SPE) | _BV(SPR0);
    SPSR = 1;  // double speed bit

    // Timer/Counter 0 initialization
    // Clock source: System Clock
    // Clock value: 250,000 kHz
    // Mode: Normal top=0xFF
    // OC0 output: Disconnected
    TCCR0A=0x00;
    TCCR0B=0x03;
    TCNT0=0x06;
    OCR0A=0x00;

    // Timer/Counter 1 initialization
    // Clock source: System Clock
    // Clock value: 62,500 kHz
    // Mode: Normal top=0xFFFF
    // OC1A output: Discon.
    // OC1B output: Discon.
    // Noise Canceler: Off
    // Input Capture on Falling Edge
    // Timer1 Overflow Interrupt: On
    // Input Capture Interrupt: Off
    // Compare A Match Interrupt: Off
    // Compare B Match Interrupt: Off
    TCCR1A=0x00;
    TCCR1B=0x01;
    //TCCR1B=0x04;
    TCNT1H=0xE7;
    TCNT1L=0x96;
    ICR1H=0x00;
    ICR1L=0x00;
    OCR1AH=0x00;
    OCR1AL=0x00;
    OCR1BH=0x00;
    OCR1BL=0x00;
/*
    // Timer/Counter 2 initialization
    // Clock source: System Clock
    // Clock value: Timer2 Stopped
    // Mode: Normal top=0xFF
    // OC2 output: Disconnected
    ASSR=0x00;
    TCCR2A=0x00;
    TCNT2=0x00;
    OCR2A=0x00;

    // External Interrupt(s) initialization
    // INT0: Off
    // INT1: Off
    // INT2: Off
    MCUCR=0x00;
    MCUSR=0x00;
*/


    // Analog Comparator initialization
    // Analog Comparator: Off
    // Analog Comparator Input Capture by Timer/Counter 1: Off
    ACSR=0x80;
    ADCSRB = 0x00;

//    SFIOR=0x00;

    // ADC initialization
    // ADC Clock frequency: 125,000 kHz
    // ADC Voltage Reference: AVCC pin
    // ADC Auto Trigger Source: ADC Stopped
    ADMUX=ADC_VREF_TYPE & 0xff;
    ADCSRA=0x87;

    // TWI initialization
    // TWI disabled
    TWCR=0x00;

    // Global enable interrupts
    asm volatile("sei");
}


