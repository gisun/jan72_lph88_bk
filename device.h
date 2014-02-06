#ifndef _DEVICE_H
#define _DEVICE_H

//#define S65_MIRROR
//#define S65_ROTATE

#if defined(S65_ROTATE)
# define S65_WIDTH            (132)
# define S65_HEIGHT           (176)
#else
# define S65_WIDTH            (176)
# define S65_HEIGHT           (132)
#endif

#define LCD_CS		PD7//PB0
#define LCD_RESET	PD6//PB6
#define LCD_RS		PD5//PB7
#define LCD_MOSI	PB3
#define LCD_MISO	PB4
#define LCD_SCK		PB5

#define LCD_LED		PB2

#define F_CPU 16000000


#define ADC_VREF_TYPE 0x40

#endif
