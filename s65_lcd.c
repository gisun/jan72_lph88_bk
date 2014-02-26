#include "device.h"

#include <inttypes.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include <util/delay.h>

#include "s65_lcd.h"

#include "fonts.h"


void s65_LED_ON(){
    PORTB |= _BV(LCD_LED);
}

void s65_LED_OFF(){
    PORTB &= ~_BV(LCD_LED);
}

void S65_RST_DISABLE(){
    PORTD |= _BV(LCD_RESET);  // release reset
}

void S65_RST_ENABLE(){
    PORTD &= ~_BV(LCD_RESET); // reset display
}

void S65_CS_DISABLE() {
    PORTD |= _BV(LCD_CS);
};

void S65_CS_ENABLE() {
    PORTD &= ~_BV(LCD_CS);
};

void S65_RS_DISABLE() {
    PORTD |= _BV(LCD_RS);
};

void S65_RS_ENABLE() {
    PORTD &= ~_BV(LCD_RS);
};

void s65_writeSPI(uint8_t data) {
    SPDR = data;
    while(!(SPSR & (1<<SPIF)));
}

#ifdef LCD_LPH88
#include "s65_lph88_drv.c"
#endif

#ifdef LCD_LS020
#include "s65_ls020_drv.c"
#endif



void s65_clear(uint16_t color) {
    uint16_t size;

    s65_setArea(0, 0, (S65_WIDTH-1), (S65_HEIGHT-1));

    s65_drawStart();
    for(size=(S65_WIDTH*S65_HEIGHT); size!=0; size--) s65_draw(color);
    s65_drawStop();
}

void s65_drawPixel(uint8_t x0, uint8_t y0, uint16_t color) {
    if((x0 >= S65_WIDTH) || (y0 >= S65_HEIGHT)) return;

    s65_setCursor(x0, y0);

    s65_drawStart();
    s65_draw(color);
    s65_drawStop();
}

void s65_drawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color) {
    int16_t dx, dy, dx2, dy2, err, stepx, stepy;

    //horizontal or vertical line
    if((x0 == x1) || (y0 == y1)) {
	s65_fillRect(x0, y0, x1, y1, color);
    } else {
	//calculate direction
	dx = x1 - x0;
	dy = y1 - y0;
	if(dx < 0) { dx = -dx; stepx = -1; } else { stepx = +1; }
	if(dy < 0) { dy = -dy; stepy = -1; } else { stepy = +1; }
	dx2 = dx << 1;
	dy2 = dy << 1;
	//draw line
	s65_setArea(0, 0, (S65_WIDTH-1), (S65_HEIGHT-1));
	s65_drawPixel(x0, y0, color);
	if(dx > dy) {
	    err = dy2 - dx;
	    while(x0 != x1) {
		if(err >= 0) {
		    y0  += stepy;
		    err -= dx2;
		}
		x0  += stepx;
		err += dy2;
		s65_drawPixel(x0, y0, color);
	    }
	} else {
	    err = dx2 - dy;
	    while(y0 != y1) {
		if(err >= 0) {
		    x0  += stepx;
		    err -= dy2;
		}
		y0  += stepy;
		err += dx2;
		s65_drawPixel(x0, y0, color);
	    }
	}
    }
}

void s65_fillRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color) {
    uint16_t size;
    uint8_t tmp;

    if(x0 > x1) {
	tmp = x0;
	x0  = x1;
	x1  = tmp;
    }
    if(y0 > y1) {
	tmp = y0;
	y0  = y1;
	y1  = tmp;
    }

    if((x1 >= S65_WIDTH) || (y1 >= S65_HEIGHT)) return;

    s65_setArea(x0, y0, x1, y1);

    s65_drawStart();
    for(size=((1+(x1-x0))*(1+(y1-y0))); size!=0; size--) s65_draw(color);
    s65_drawStop();
}

void s65_drawRect(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint16_t color) {
    s65_fillRect(x0, y0, x0, y1, color);
    s65_fillRect(x0, y1, x1, y1, color);
    s65_fillRect(x1, y0, x1, y1, color);
    s65_fillRect(x0, y0, x1, y0, color);
}

void s65_drawCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint16_t color) {
    int16_t err, x, y;

    err = -radius;
    x   = radius;
    y   = 0;

    s65_setArea(0, 0, (S65_WIDTH-1), (S65_HEIGHT-1));

    while(x >= y) {
	s65_drawPixel(x0 + x, y0 + y, color);
	s65_drawPixel(x0 - x, y0 + y, color);
	s65_drawPixel(x0 + x, y0 - y, color);
	s65_drawPixel(x0 - x, y0 - y, color);
	s65_drawPixel(x0 + y, y0 + x, color);
	s65_drawPixel(x0 - y, y0 + x, color);
	s65_drawPixel(x0 + y, y0 - x, color);
	s65_drawPixel(x0 - y, y0 - x, color);

	err += y;
	y++;
	err += y;
	if(err >= 0) {
	    x--;
	    err -= x;
	    err -= x;
	}
    }
}

void s65_fillCircle(uint8_t x0, uint8_t y0, uint8_t radius, uint16_t color) {
    int16_t err, x, y;

    err = -radius;
    x   = radius;
    y   = 0;

    s65_setArea(0, 0, (S65_WIDTH-1), (S65_HEIGHT-1));

    while(x >= y) {
	s65_drawLine(x0 - x, y0 + y, x0 + x, y0 + y, color);
	s65_drawLine(x0 - x, y0 - y, x0 + x, y0 - y, color);
	s65_drawLine(x0 - y, y0 + x, x0 + y, y0 + x, color);
	s65_drawLine(x0 - y, y0 - x, x0 + y, y0 - x, color);

	err += y;
	y++;
	err += y;
	if(err >= 0) {
	    x--;
	    err -= x;
	    err -= x;
	}
    }
}


uint8_t s65_drawChar(uint8_t x, uint8_t y, char c, uint8_t size, uint16_t color, uint16_t bg_color) {
    uint8_t ret;

#if FONT_WIDTH <= 8
    uint8_t data, mask;
#elif FONT_WIDTH <= 16
    uint16_t data, mask;
#elif FONT_WIDTH <= 32
    uint32_t data, mask;
#endif
    uint8_t i, j, width, height;
    const prog_uint8_t *ptr;

    i      = (uint8_t)c;
#if FONT_WIDTH <= 8
    ptr    = &font_PGM[(i-FONT_START)*(8*FONT_HEIGHT/8)];
#elif FONT_WIDTH <= 16
    ptr    = &font_PGM[(i-FONT_START)*(16*FONT_HEIGHT/8)];
#elif FONT_WIDTH <= 32
    ptr    = &font_PGM[(i-FONT_START)*(32*FONT_HEIGHT/8)];
#endif

    width  = FONT_WIDTH;
    height = FONT_HEIGHT;

    if(size <= 1) {
	ret = x+width;
	if(ret > S65_WIDTH) return S65_WIDTH+1;

	s65_setArea(x, y, (x+width-1), (y+height-1));

	s65_drawStart();
	for(; height!=0; height--) {
#if FONT_WIDTH <= 8
	    data = pgm_read_byte(ptr); ptr+=1;
#elif FONT_WIDTH <= 16
	    data = pgm_read_word(ptr); ptr+=2;
#elif FONT_WIDTH <= 32
	    data = pgm_read_dword(ptr); ptr+=4;
#endif
	    for(mask=(1<<(width-1)); mask!=0; mask>>=1) {
		if(data & mask) s65_draw(color);
		else s65_draw(bg_color);
	    }
	}
	s65_drawStop();
    } else {
	ret = x+(width*size);
	if(ret > S65_WIDTH) return S65_WIDTH+1;

	s65_setArea(x, y, (x+(width*size)-1), (y+(height*size)-1));

	s65_drawStart();
	for(; height!=0; height--) {
#if FONT_WIDTH <= 8
	    data = pgm_read_byte(ptr); ptr+=1;
#elif FONT_WIDTH <= 16
	    data = pgm_read_word(ptr); ptr+=2;
#elif FONT_WIDTH <= 32
	    data = pgm_read_dword(ptr); ptr+=4;
#endif
	    for(i=size; i!=0; i--) {
		for(mask=(1<<(width-1)); mask!=0; mask>>=1) {
		    if(data & mask) {
			for(j=size; j!=0; j--) {
			    s65_draw(color);
			}
		    } else {
			for(j=size; j!=0; j--) {
			    s65_draw(bg_color);
			}
		    }
		}
	    }
	}
	s65_drawStop();
    }
    return ret;
}

uint8_t s65_drawText(uint8_t x, uint8_t y, char *s, uint8_t size, uint16_t color, uint16_t bg_color) {
    while(*s != 0) {
	x = s65_drawChar(x, y, *s++, size, color, bg_color);
	if(x > S65_WIDTH) break;
    }

    return x;
}


uint8_t s65_drawTextPGM(uint8_t x, uint8_t y, PGM_P s, uint8_t size, uint16_t color, uint16_t bg_color) {
    char c;

    c = pgm_read_byte(s++);
    while(c != 0) {
	x = s65_drawChar(x, y, c, size, color, bg_color);
	if(x > S65_WIDTH) break;
	c = pgm_read_byte(s++);
    }

    return x;
}


uint8_t s65_drawMLText(uint8_t x, uint8_t y, char *s, uint8_t size, uint16_t color, uint16_t bg_color) {
    uint8_t i, start_x=x, wlen, llen;
    char c;
    char *wstart;

    s65_fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text

    llen   = (S65_WIDTH-x)/8;
    wstart = s;
    while(*s) {
	c = *s++;
	//new line
	if(c == '\n') {
	    s65_fillRect(x, y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
	    x  = start_x;
	    y += (FONT_HEIGHT*size)+2;
	    s65_fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text
	    continue;
	}

	//start of a new word
	if(c == ' ') wstart = s;

	if((c == ' ') && (x == start_x)) {
	    //do nothing
	} else
	    if(c >= FONT_START) {
		i = s65_drawChar(x, y, c, size, color, bg_color);
		//new line
		if(i > S65_WIDTH) {
		    //do not start with space
		    if(c == ' ') {
			s65_fillRect(x, y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
			x  = start_x;
			y += (FONT_HEIGHT*size)+2;
			s65_fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text
		    } else {
			wlen = (s-wstart);
			//word too long
			if(wlen > llen) {
			    s65_fillRect(x, y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
			    x  = start_x;
			    y += (FONT_HEIGHT*size)+2;
			    s65_fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text
			    x = s65_drawChar(x, y, c, size, color, bg_color);
			} else {
			    s65_fillRect(x-(wlen*8), y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
			    x  = start_x;
			    y += (FONT_HEIGHT*size)+2;
			    s65_fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text
			    s = wstart;
			}
		    }
		} else {
		    x = i;
		}
	    }
    }
    s65_fillRect(x, y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
    return x;
}

uint8_t s65_drawMLTextPGM(uint8_t x, uint8_t y, PGM_P s, uint8_t size, uint16_t color, uint16_t bg_color) {
    uint8_t i, start_x=x, wlen, llen;
    char c;
    PGM_P wstart;

    s65_fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text

    llen   = (S65_WIDTH-x)/8;
    wstart = s;

    c = pgm_read_byte(s++);
    while(c != 0) {
	//new line
	if(c == '\n') {
	    s65_fillRect(x, y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
	    x  = start_x;
	    y += (FONT_HEIGHT*size)+2;
	    s65_fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text
	    c = pgm_read_byte(s++);
	    continue;
	}

	//start of a new word
	if(c == ' ') wstart = s;

	if((c == ' ') && (x == start_x)) {
	    //do nothing
	} else if(c >= FONT_START) {
	    i = s65_drawChar(x, y, c, size, color, bg_color);
	    //new line
	    if(i > S65_WIDTH) {
		//do not start with space
		if(c == ' ') {
		    s65_fillRect(x, y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
		    x  = start_x;
		    y += (FONT_HEIGHT*size)+2;
		    s65_fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text
		} else {
		    wlen = (s-wstart);
		    //word too long
		    if(wlen > llen) {
			s65_fillRect(x, y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
			x  = start_x;
			y += (FONT_HEIGHT*size)+2;
			s65_fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text
			x = s65_drawChar(x, y, c, size, color, bg_color);
		    } else {
			s65_fillRect(x-(wlen*8), y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
			x  = start_x;
			y += (FONT_HEIGHT*size)+2;
			s65_fillRect(0, y, x-1, (y+(FONT_HEIGHT*size)), bg_color); //clear before text
			s = wstart;
		    }
		}
	    } else x = i;
	}
	c = pgm_read_byte(s++);
    }
    s65_fillRect(x, y, (S65_WIDTH-1), (y+(FONT_HEIGHT*size)), bg_color); //clear after text
    return x;
}

