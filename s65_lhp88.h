#ifndef _DISP1_H
#define _DISP1_H

#if defined(S65_ROTATE)
# define S65_WIDTH            (132)
# define S65_HEIGHT           (176)
#else
# define S65_WIDTH            (176)
# define S65_HEIGHT           (132)
#endif

#define RGB(r,g,b)           (((r<<11)&0xF800)|((g<<5)&0x07E0)|(b&0x001F)) //5 red | 6 green | 5 blue

void s65_LED_ON();
void s65_LED_OFF();

void s65_init(void);
void s65_clear(uint16_t color);
void s65_drawPixel(uint8_t x0, uint8_t y0, uint16_t color);
void s65_drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);
void s65_drawRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);
void s65_fillRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color);
void s65_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint16_t color);
void s65_fillCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint16_t color);

uint8_t s65_drawChar(uint8_t x, uint8_t y, char c, uint8_t size, uint16_t color, uint16_t bg_color);
uint8_t s65_drawText(uint8_t x, uint8_t y, char *s, uint8_t size, uint16_t color, uint16_t bg_color);


uint8_t s65_drawTextPGM(uint8_t x, uint8_t y, PGM_P s, uint8_t size, uint16_t color, uint16_t bg_color);
uint8_t s65_drawMLText(uint8_t x, uint8_t y, char *s, uint8_t size, uint16_t color, uint16_t bg_color);
uint8_t s65_drawMLTextPGM(uint8_t x, uint8_t y, PGM_P s, uint8_t size, uint16_t color, uint16_t bg_color);



#endif
