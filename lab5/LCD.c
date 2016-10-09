#include "LCD.H"

void drawRedSquare(void)
{
  setArea(OUTSQSC, OUTSQEC, 0x1B, 0x66);           //27,102
  writeCmd(0x2C);
  drawSizeColor(SQUAREAREA, 0xFF, 0x00, 0x00);      //red
}

void drawGreenSquare(void)
{
  setArea(OUTSQSC, OUTSQEC, 0x7A, 0xC5);            //122,197
  writeCmd(0x2C);
  drawSizeColor(SQUAREAREA, 0x00, 0xFF, 0x00);      //green
}

void drawYelSquare(void)
{
  setArea(OUTSQSC, OUTSQEC, 0xD9, 0x124);           //217, 292
  writeCmd(0x2C);
  drawSizeColor(SQUAREAREA, 0xFF, 0xFF, 0x00);      //yellow
}

void drawRedEmpty(void)
{  
  setArea(INSQSC, INSQEC, 0x22, 0x5F);                  // 34, 95
  drawSizeColor(INNERSQAREA, 0x00, 0x00, 0x01);      //black
}

void drawGreenEmpty(void)
{
  setArea(INSQSC, INSQEC, 0x81, 0xBE);              // 129, 190
  drawSizeColor(INNERSQAREA, 0x00, 0x00, 0x01);      //black
}

void drawYelEmpty(void)
{
  setArea(INSQSC, INSQEC, 0xE0, 0x11D);               // 224, 285
  drawSizeColor(INNERSQAREA, 0x00, 0x00, 0x01);      //black
}


void drawSizeColor(unsigned int Area, unsigned char RED, unsigned char GREEN, unsigned char BLUE)
{
	unsigned int k;
	writeCmd(0x2C);
	for(k = 0; k < Area; k++)
	{
		RGB(RED, GREEN, BLUE);
	}
}

void writeCmd(unsigned char input)
{
	PA[0x3FC] &= 0xEF;					// D/CX = 0 -> command
	PA[0x3FC] &= 0xDF;					// WRX = 0 -> going to write
	PE[0x3FC] = input & 0x3;
	PD[0x3FC] = input & 0xF;
	PC[0x3FC] = input & 0xF0;
	PA[0x3FC] |= 0x20;					// WRX = 1 -> reads on pos edge
	
	PA[0x3FC] |= 0x10;					// D/CX = 1 -> default
}

void writeDat(unsigned char input) // 1 byte
{
	PA[0x3FC] &= 0xDF;					// WRX = 0 -> going to write
	PE[0x3FC] = input & 0x3;
	PD[0x3FC] = input & 0xF;
	PC[0x3FC] = input & 0xF0;
	PA[0x3FC] |= 0x20;					// WRX = 1 -> reads on pos edge
	
}

void RGB(unsigned char RED, unsigned char GREEN, unsigned char BLUE)
{
	unsigned short color = ((RED & 0x1F) << 11);
	color |= ((GREEN & 0x3F) << 5);
	color |= (BLUE & 0x1F);
	writeColor(color);
}

void writeColor(unsigned short color)
{
	writeDat((color & 0xFF00) >> 8);
	writeDat((color & 0xFF));
}

void setArea(unsigned short x1, unsigned short x2, unsigned short y1, unsigned short y2)
{
  writeCmd(0x2A);				    //column address set, max 0xEF
	writeDat( (x1 >> 8) );    //SC MSB
	writeDat( (x1 & 0xFF) );  //SC LSB
	writeDat( (x2 >> 8) );    //EC MSB
	writeDat( (x2 & 0xFF) );  //EC LSB
	
	writeCmd(0x2B);				//page address set, max 0x13F
	writeDat( (y1 >> 8) );    //SP MSB
	writeDat( (y1 & 0xFF) );  //SP LSB
	writeDat( (y2 >> 8) );    //EP MSB
	writeDat( (y2 & 0xFF) );  //EP LSB
}

void LCD_Init()
{
	int i;
    writeCmd(0xCB);
    writeDat(0x39);
    writeDat(0x2C);
    writeDat(0x00);
    writeDat(0x34);
    writeDat(0x02);

    writeCmd(0xCF);
    writeDat(0x00);
    writeDat(0XC1);
    writeDat(0X30);

    writeCmd(0xE8);
    writeDat(0x85);
    writeDat(0x00);
    writeDat(0x78);

    writeCmd(0xEA);
    writeDat(0x00);
    writeDat(0x00);

    writeCmd(0xED);
    writeDat(0x64);
    writeDat(0x03);
    writeDat(0X12);
    writeDat(0X81);

    writeCmd(0xF7);
    writeDat(0x20);

    writeCmd(0xC0);    //Power control
    writeDat(0x23);   //VRH[5:0]

    writeCmd(0xC1);    //Power control
    writeDat(0x10);   //SAP[2:0];BT[3:0]

    writeCmd(0xC5);    //VCM control
    writeDat(0x3e); //¶Ô±È¶Èµ÷½Ú
    writeDat(0x28);

    writeCmd(0xC7);    //VCM control2
    writeDat(0x86);  //--

    writeCmd(0x36);    // Memory Access Control
    writeDat(0x48); //C8       //48 68ÊúÆÁ//28 E8 ºáÆÁ

    writeCmd(0x3A);
    writeDat(0x55);

    writeCmd(0xB1);
    writeDat(0x00);
    writeDat(0x18);

    writeCmd(0xB6);    // Display Function Control
    writeDat(0x08);
    writeDat(0x82);
    writeDat(0x27);

    writeCmd(0xF2);    // 3Gamma Function Disable
    writeDat(0x00);

    writeCmd(0x26);    //Gamma curve selected
    writeDat(0x01);

    writeCmd(0xE0);    //Set Gamma
    writeDat(0x0F);
    writeDat(0x31);
    writeDat(0x2B);
    writeDat(0x0C);
    writeDat(0x0E);
    writeDat(0x08);
    writeDat(0x4E);
    writeDat(0xF1);
    writeDat(0x37);
    writeDat(0x07);
    writeDat(0x10);
    writeDat(0x03);
    writeDat(0x0E);
    writeDat(0x09);
    writeDat(0x00);

    writeCmd(0XE1);    //Set Gamma
    writeDat(0x00);
    writeDat(0x0E);
    writeDat(0x14);
    writeDat(0x03);
    writeDat(0x11);
    writeDat(0x07);
    writeDat(0x31);
    writeDat(0xC1);
    writeDat(0x48);
    writeDat(0x08);
    writeDat(0x0F);
    writeDat(0x0C);
    writeDat(0x31);
    writeDat(0x36);
    writeDat(0x0F);

    writeCmd(0x11);    //Exit Sleep
		for( i = 0; i < 20000; i++) { i++;}

    writeCmd(0x29);    //Display on
}
