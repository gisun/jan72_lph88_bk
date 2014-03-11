void s65_drawStart(void)
{
  s65_writeCmd(0x5C);
  S65_RS_DISABLE(); //data
  S65_CS_ENABLE();

  return;
}


void s65_draw(uint16_t color)
{
  s65_writeSPI(color>>8);
  s65_writeSPI(color);

  return;
}


void s65_drawStop(void)
{
  S65_CS_DISABLE();

  return;
}


void s65_setArea(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
#ifdef S65_MIRROR
  s65_writeCmd(0x15);                    //column address set 
  s65_writeData(0x08+(S65_HEIGHT-1)-y1); //start column
  s65_writeData(0x01);                   //start column
  s65_writeData(0x08+(S65_HEIGHT-1)-y0); //end column
  s65_writeData(0x01);                   //end column
  s65_writeCmd(0x75);                    //page address set 
  s65_writeData(x0);                     //start page
  s65_writeData(x1);                     //end page
#else
  s65_writeCmd(0x15);                    //column address set 
  s65_writeData(0x08+y0);                //start column
  s65_writeData(0x01);                   //start column
  s65_writeData(0x08+y1);                //end column
  s65_writeData(0x01);                   //end column
  s65_writeCmd(0x75);                    //page address set 
  s65_writeData(x0);                     //start page
  s65_writeData(x1);                     //end page
#endif

  return;
}


void s65_setCursor(uint8_t x, uint8_t y)
{
  s65_setArea(x, y, x, y);

  return;
}


void s65_init(void)
{
  //reset
  S65_CS_DISABLE();
  S65_RS_DISABLE();
  S65_RST_ENABLE();
  _delay_ms(50);
  S65_RST_DISABLE();
  _delay_ms(50);

  s65_writeCmd(0xAE); //display off

  s65_writeCmd(0xBC);  //data control
#ifdef S65_MIRROR
  s65_writeData(0x2A); //565 mode, 0x2A=normal, 0x2B=180
#else
  s65_writeData(0x2B); //565 mode, 0x2A=normal, 0x2B=180
#endif

  s65_writeCmd(0xCA);  //display control 
  s65_writeData(0x4C); //P1
  s65_writeData(0x01); //P2
  s65_writeData(0x53); //P3
  s65_writeData(0x00); //P4
  s65_writeData(0x02); //P5
  s65_writeData(0xB4); //P6
  s65_writeData(0xB0); //P7
  s65_writeData(0x02); //P8
  s65_writeData(0x00); //P9

  s65_writeCmd(0x94); //sleep out

  _delay_ms(5);

  s65_writeCmd(0xAF); //display on

  _delay_ms(5);

  s65_setArea(0, 0, (S65_WIDTH-1), (S65_HEIGHT-1));

  return;
}


void s65_writeData(uint8_t data)
{
  S65_RS_DISABLE(); //data
  S65_CS_ENABLE();
  s65_writeSPI(data);
  s65_writeSPI(0x00);
  S65_CS_DISABLE();

  return;
}


void s65_writeCmd(uint8_t cmd)
{
  S65_RS_ENABLE(); //cmd
  S65_CS_ENABLE();
  s65_writeSPI(cmd);
  s65_writeSPI(0x00);
  S65_CS_DISABLE();

  return;
}

