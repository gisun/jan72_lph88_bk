void s65_drawStart(void) {
    S65_RS_ENABLE(); //data
    S65_CS_ENABLE();
    return;
}

void s65_draw(uint16_t color) {
    s65_writeSPI(color>>8);
    s65_writeSPI(color);
    return;
}

void s65_drawStop(void) {
    S65_CS_DISABLE();
    return;
}

void s65_setArea(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    s65_writeCmd(0xEF, 0x90);
#ifdef S65_ROTATE
#ifdef S65_MIRROR
    s65_writeCmd(0x08, (S65_WIDTH-1)-x0);  //set x0
    s65_writeCmd(0x09, (S65_WIDTH-1)-x1);  //set x1
    s65_writeCmd(0x0A, (S65_HEIGHT-1)-y0); //set y0
    s65_writeCmd(0x0B, (S65_HEIGHT-1)-y1); //set y1
    s65_writeCmd(0x06, (S65_WIDTH-1)-x0);  //set x cursor pos
    s65_writeCmd(0x07, (S65_HEIGHT-1)-y0); //set y cursor pos
#else
    s65_writeCmd(0x08, x0);                //set x0
    s65_writeCmd(0x09, x1);                //set x1
    s65_writeCmd(0x0A, y0);                //set y0
    s65_writeCmd(0x0B, y1);                //set y1
    s65_writeCmd(0x06, x0);                //set y cursor pos
    s65_writeCmd(0x07, y0);                //set x cursor pos
#endif
#else
#ifdef S65_MIRROR
    s65_writeCmd(0x08, (S65_HEIGHT-1)-y0); //set y0
    s65_writeCmd(0x09, (S65_HEIGHT-1)-y1); //set y1
    s65_writeCmd(0x0A, x0);                //set x0
    s65_writeCmd(0x0B, x1);                //set x1
    s65_writeCmd(0x06, (S65_HEIGHT-1)-y0); //set y cursor pos
    s65_writeCmd(0x07, x0);                //set x cursor pos
#else
    s65_writeCmd(0x08, y0);                //set y0
    s65_writeCmd(0x09, y1);                //set y1
    s65_writeCmd(0x0A, (S65_WIDTH-1)-x0);  //set x0
    s65_writeCmd(0x0B, (S65_WIDTH-1)-x1);  //set x1
    s65_writeCmd(0x06, y0);                //set y cursor pos
    s65_writeCmd(0x07, (S65_WIDTH-1)-x0);  //set x cursor pos
#endif
#endif
    return;
}

void s65_setCursor(uint8_t x, uint8_t y) {
    s65_writeCmd(0xEF, 0x90);
#ifdef S65_ROTATE
#ifdef S65_MIRROR
    s65_writeCmd(0x06, (S65_WIDTH-1)-x);  //set x cursor pos
    s65_writeCmd(0x07, (S65_HEIGHT-1)-y); //set y cursor pos
#else
    s65_writeCmd(0x06, x);                //set y cursor pos
    s65_writeCmd(0x07, y);                //set x cursor pos
#endif
#else
#ifdef S65_MIRROR
    s65_writeCmd(0x06, (S65_HEIGHT-1)-y); //set y cursor pos
    s65_writeCmd(0x07, x);                //set x cursor pos
#else
    s65_writeCmd(0x06, y);                //set y cursor pos
    s65_writeCmd(0x07, (S65_WIDTH-1)-x);  //set x cursor pos
#endif
#endif
    return;
}

void s65_scrollP(uint8_t start, uint8_t width, uint8_t value) {
    //partial scroll
    s65_writeCmd(0xEF, 0x90);
    s65_writeCmd(0x0F, start);
    s65_writeCmd(0x10, width);
    s65_writeCmd(0x11, value);
    return;
}

void s65_scroll(uint8_t value) {
    //full scroll
    s65_writeCmd(0xEF, 0x90);
    s65_writeCmd(0x11, value);
    return;
}


void s65_on(void) {
    //display ON
    s65_writeCmd(0xEF, 0x90);
    s65_writeCmd(0x00, 0x00);
    return;
}


void s65_off(uint8_t bw) {
    //display OFF with black/white background
    s65_writeCmd(0xEF, 0x90);
    if(bw == 0) s65_writeCmd(0x00, 0x40); //black background
    else s65_writeCmd(0x00, 0x80); //white background
    return;
}

void s65_invert(uint8_t inv) {
    //invert display colors
    s65_writeCmd(0xEF, 0xB0);
    if(inv == 0) s65_writeCmd(0x49, 0x02);
    else s65_writeCmd(0x49, 0x03);
    s65_writeCmd(0xEF, 0x90);
    s65_writeCmd(0x00, 0x00);
    return;
}


void s65_contrast(uint8_t value) {
    //display contrast
    s65_writeCmd(0x24, value);
    s65_writeCmd(0x00, 0x00);
    return;
}


void s65_init(void) {
    //reset
    S65_CS_DISABLE();
    S65_RS_DISABLE();
    S65_RST_ENABLE();
    _delay_ms(50);
    S65_RST_DISABLE();
    _delay_ms(50);

    s65_writeCmd(0xFD, 0xFD);
    s65_writeCmd(0xFD, 0xFD);

    _delay_ms(50);

    //init 1
    s65_writeCmd(0xEF, 0x00);
    s65_writeCmd(0xEE, 0x04);
    s65_writeCmd(0x1B, 0x04);
    s65_writeCmd(0xFE, 0xFE);
    s65_writeCmd(0xFE, 0xFE);
    s65_writeCmd(0xEF, 0x90);
    s65_writeCmd(0x4A, 0x04);
    s65_writeCmd(0x7F, 0x3F);
    s65_writeCmd(0xEE, 0x04);
    s65_writeCmd(0x43, 0x06);

    _delay_ms(7); //important 7ms

    //init 2
    s65_writeCmd(0xEF, 0x90);
    s65_writeCmd(0x09, 0x83);
    s65_writeCmd(0x08, 0x00);
    s65_writeCmd(0x0B, 0xAF);
    s65_writeCmd(0x0A, 0x00);
    s65_writeCmd(0x05, 0x00);
    s65_writeCmd(0x06, 0x00);
    s65_writeCmd(0x07, 0x00);
    s65_writeCmd(0xEF, 0x00);
    s65_writeCmd(0xEE, 0x0C);
    s65_writeCmd(0xEF, 0x90);
    s65_writeCmd(0x00, 0x80);
    s65_writeCmd(0xEF, 0xB0);
    s65_writeCmd(0x49, 0x02);
    s65_writeCmd(0xEF, 0x00);
    s65_writeCmd(0x7F, 0x01);
    s65_writeCmd(0xE1, 0x81);
    s65_writeCmd(0xE2, 0x02);
    s65_writeCmd(0xE2, 0x76);
    s65_writeCmd(0xE1, 0x83);

    _delay_ms(50);

    //display on
    s65_writeCmd(0x80, 0x01);
    s65_writeCmd(0xEF, 0x90);
    s65_writeCmd(0x00, 0x00);

    //display options
    s65_writeCmd(0xEF, 0x90);
#ifdef S65_ROTATE
#ifdef S65_MIRROR
    s65_writeCmd(0x01, 0xC0); //x1->x0, y1->y0
    s65_writeCmd(0x05, 0x00); //0x04=rotate, 0x00=normal
#else
    s65_writeCmd(0x01, 0x00); //x0->x1, y0->y1
    s65_writeCmd(0x05, 0x00); //0x04=rotate, 0x00=normal
#endif
#else
#ifdef S65_MIRROR
    s65_writeCmd(0x01, 0x80); //x0->x1, y1->y0
    s65_writeCmd(0x05, 0x04); //0x04=rotate, 0x00=normal
#else
    s65_writeCmd(0x01, 0x40); //x1->x0, y0->y1
    s65_writeCmd(0x05, 0x04); //0x04=rotate, 0x00=normal
#endif
#endif
    s65_setArea(0, 0, (S65_WIDTH-1), (S65_HEIGHT-1));
    return;
}

void s65_writeData(uint16_t data) {
    S65_RS_ENABLE(); //data
    S65_CS_ENABLE();
    s65_writeSPI(data>>8);
    s65_writeSPI(data);
    S65_CS_DISABLE();
    return;
}

void s65_writeCmd(uint8_t reg, uint8_t param) {
    S65_RS_DISABLE(); //cmd
    S65_CS_ENABLE();
    s65_writeSPI(reg);
    s65_writeSPI(param);
    S65_CS_DISABLE();
    return;
}
