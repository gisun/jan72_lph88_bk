void s65_writeReg(uint8_t reg) {
    S65_CS_ENABLE();
    s65_writeSPI(0x74); //index register
    s65_writeSPI(0x00);
    s65_writeSPI(reg);
    S65_CS_DISABLE();
}

void s65_writeData(uint16_t data) {
    S65_CS_ENABLE();
    s65_writeSPI(0x76); //instruction or RAM data
    s65_writeSPI(data>>8);
    s65_writeSPI(data);
    S65_CS_DISABLE();
}

void s65_writeCmd(uint8_t reg, uint16_t param) {
    s65_writeReg(reg);
    s65_writeData(param);
}

void s65_drawStart(void) {
    s65_writeReg(0x22);
    S65_CS_ENABLE();
    s65_writeSPI(0x76);
}

void s65_draw(uint16_t color) {
    s65_writeSPI(color>>8);
    s65_writeSPI(color);
}

void s65_drawStop(void) {
    S65_CS_DISABLE();
}

void s65_setArea(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
    //set area
#ifdef S65_MIRROR
    s65_writeCmd(0x16, (((S65_HEIGHT-1)-y0)<<8)|(((S65_HEIGHT-1)-y1))); //set y
    s65_writeCmd(0x17, (((S65_WIDTH-1)-x0)<<8)|(((S65_WIDTH-1)-x1)));  //set x
#else
    s65_writeCmd(0x16, (y1<<8)|(y0)); //set y
    s65_writeCmd(0x17, (x1<<8)|(x0)); //set x
#endif

    //set cursor
    s65_setCursor(x0, y0);
}


void s65_setCursor(uint8_t x, uint8_t y) {
#ifdef S65_MIRROR
    s65_writeCmd(0x21, ((((S65_WIDTH-1)-x)<<8)|((S65_HEIGHT-1)-y))); //set cursor pos
#else
    s65_writeCmd(0x21, ((x<<8)|y)); //set cursor pos
#endif
}

void s65_init(void) {
    //reset
    S65_CS_DISABLE();
    S65_RS_DISABLE();
    S65_RST_ENABLE();
    _delay_ms(50);
    S65_RST_DISABLE();
    _delay_ms(50);

    s65_writeCmd(0x07, 0x0000); //display off
    _delay_ms(10);

    //power on sequence
    s65_writeCmd(0x02, 0x0400); //lcd drive control
    s65_writeCmd(0x0C, 0x0001); //power control 3: VC        //step 1
    s65_writeCmd(0x0D, 0x0006); //power control 4: VRH
    s65_writeCmd(0x04, 0x0000); //power control 2: CAD
    s65_writeCmd(0x0D, 0x0616); //power control 4: VRL
    s65_writeCmd(0x0E, 0x0010); //power control 5: VCM
    s65_writeCmd(0x0E, 0x1010); //power control 5: VDV
    s65_writeCmd(0x03, 0x0000); //power control 1: BT        //step 2
    s65_writeCmd(0x03, 0x0000); //power control 1: DC
    s65_writeCmd(0x03, 0x000C); //power control 1: AP
    _delay_ms(40);
    s65_writeCmd(0x0E, 0x2D1F); //power control 5: VCOMG     //step 3
    _delay_ms(40);
    s65_writeCmd(0x0D, 0x0616); //power control 4: PON       //step 4
    _delay_ms(100);

    //display options
#ifdef S65_MIRROR
    s65_writeCmd(0x05, 0x0008); //Entry mode --
#else
    s65_writeCmd(0x05, 0x0038); //Entry mode ++
#endif
    s65_setArea(0, 0, (S65_WIDTH-1), (S65_HEIGHT-1));

    //display on sequence (bit2 = reversed colors)
    s65_writeCmd(0x07, 0x0005); //display control: D0
    s65_writeCmd(0x07, 0x0025); //display control: GON
    s65_writeCmd(0x07, 0x0027); //display control: D1
    s65_writeCmd(0x07, 0x0037); //display control: DTE
    _delay_ms(10);
}
