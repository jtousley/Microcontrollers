#ifndef LCD_H
#define LCD_H

// page 238 of the data sheet.
#define white          0xFFFF
#define black          0x0001
#define grey           0xF7DE
#define blue           0x001F
#define red            0xF800
#define magenta        0xF81F
#define green          0x07E0
#define cyan           0x7FFF
#define yellow         0xFFE0
#define ENDCOL         0x00EF           //total columns
#define ENDPAGE        0x013F           //total rows
#define OUTSQSC        0x52             //Outer square starting column (82)
#define OUTSQEC        0x9D             //Outer square ending column (157)
#define INSQSC         0x59             //Inner square starting column (92)
#define INSQEC         0x96             //Inner square ending column (147)
#define TOTALAREA      0x12C00          //total screen area
#define SQUAREAREA     0x15F9           //total area per square (75^2)
#define INNERSQAREA    0xE89            //total area for inner part of square (55^2)
#define LOWX					 0x578			//1400
#define HIGHX					 0xAF0			//2800
#define REDLY					 0x190			//400
#define REDHY					 0x3E8			//1000
#define GREENLY				 0x514			//1300
#define GREENHY				 0x7D0			//2000
#define YELLY					 0x8FC			//2300
#define YELHY					 0xBB8			//3000

extern unsigned char *SYSCTL;

extern unsigned char *PA;
extern unsigned char *PB;
extern unsigned char *PC;
extern unsigned char *PD;
extern unsigned char *PE;
extern unsigned char *PF;
extern unsigned char *CORE;
extern unsigned char *TM0;
extern unsigned char *TM1;
extern unsigned char *TM2;
extern unsigned char *SSI1;

void writeCmd(unsigned char CMD);
void writeDat(unsigned char DAT);
void writeDat2(unsigned short DAT);
void writeDat4(unsigned int DAT);

void setArea(unsigned short x1, unsigned short x2, unsigned short y1, unsigned short y2);
void RGB(unsigned char RED, unsigned char GREEN, unsigned char BLUE);
void writeColor(unsigned short color);

void LCD_Init(void);
void drawSizeColor(unsigned int AREA, unsigned char RED, unsigned char GREEN, unsigned char BLUE);

void drawRedSquare(void);
void drawGreenSquare(void);
void drawYelSquare(void);

void drawRedEmpty(void);
void drawGreenEmpty(void);
void drawYelEmpty(void);


#endif
