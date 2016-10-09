// This function draws three empty boxes. The user touches the box
// to fill the box and turn on a corresponding external light.

#include "LCD.H"
#include "stdbool.h"

unsigned char *SYSCTL = (unsigned char *) 0x400FE000;
unsigned char *PA = (unsigned char *) 0x40004000;
unsigned char *PB = (unsigned char *) 0x40005000;
unsigned char *PC = (unsigned char *) 0x40006000;
unsigned char *PD = (unsigned char *) 0x40007000;
unsigned char *PE = (unsigned char *) 0x40024000;
unsigned char *PF = (unsigned char *) 0x40025000;
unsigned char *CORE = (unsigned char *) 0xE000E000;
unsigned char *TM0 = (unsigned char *) 0x40030000;
unsigned char *TM1 = (unsigned char *) 0x40031000;
unsigned char *TM2 = (unsigned char *) 0x40032000;
unsigned char *SSI1 = (unsigned char *) 0x40009000;
unsigned int *SSIDR = (unsigned int *) 0x40009008;

//globals for TP interrupts, modified in GPIOF_Handler
volatile unsigned int CNTX = 0;        //number of valid interrupts for x
volatile unsigned int CNTY = 0;					// number of valid interrupts for y
volatile unsigned int SUMX = 0;        //sum x coordinates of press
volatile unsigned int SUMY = 0;       //sum y coordinates of press
volatile bool input = false;

void GPIOF_Handler(void);

void enableClock(void);
void enableSSI1(void);
void enablePortA(void);
void enablePortB(void);
void enablePortC(void);
void enablePortD(void);
void enablePortE(void);
void enablePortF(void);
void configurePorts(void);

void flipRed(void);
void flipGreen(void);
void flipYel(void);

int main(void)
{
  volatile unsigned int xcoord;
  volatile unsigned int ycoord;
	unsigned int i;
  //SSI1[0x020] = 0x02;       // RTIC clear SSIICR... maybe
	configurePorts();
	LCD_Init();
	
	setArea(0x00, ENDCOL, 0x00, ENDPAGE);
  drawSizeColor(TOTALAREA, 0xFF, 0xFF, 0xFF);      //white
	drawSizeColor(TOTALAREA, 0x00, 0x00, 0x01);      //black
  //draw outlines
  drawRedSquare(); 
  drawGreenSquare();
  drawYelSquare();
  //default off
  drawRedEmpty();
  drawGreenEmpty();
  drawYelEmpty();

   while (1)
   {
		 //SSI1[0x008] = 0xD0;
		 if(input){
        input = false;
        xcoord = SUMX / CNTX;
        ycoord = SUMY / CNTY;
        
				if(LOWX < xcoord && xcoord < HIGHX){
					if(REDLY < ycoord && ycoord < REDHY){
						flipRed();
					}
					else if(GREENLY < ycoord && ycoord < GREENHY){
						flipGreen();
					}
					else if(YELLY < ycoord && ycoord < YELHY){
						flipYel();
					}
				}
        SUMX = 0;
        SUMY = 0;
        CNTX = 0;
				CNTY = 0;
				
				for(i = 0; i < 0xFFFF; i++);
      }
   }
}

volatile unsigned short RXX = 0x0000;
volatile unsigned short RXY = 0x0000;

void GPIOF_Handler(void)
{	
  unsigned char TX = 0xD0; 					//x coordinate request
	unsigned j = 0;
	
	while((SSI1[0x00C] & 0x1) == 0x0);		//wait for transmit FIFO to be empty
	while((SSI1[0x00C] & 0x4) == 0x4){		//while receive FIFO is not empty
		RXX = SSIDR[0];
	}
  SSI1[0x008] = TX;                     //ask for x coordinate
	for(j = 0; j < 0x1FF; j++);						// wait for receive FIFO
	
  RXX = SSIDR[0];  	//read data
	RXX = (RXX & 0x7FFF) >> 3;				//disregard first bit
	SUMX += RXX;
	CNTX++;
  
  TX = 0x90;      //y coordinate request
  while((SSI1[0x00C] & 0x2) == 0x0);		//wait TNF = 0; TNF = 1, continue
	while((SSI1[0x00C] & 0x4) == 0x4){
		RXY = SSIDR[0];	                 //read data	
	}
  SSI1[0x008] = TX;                     //ask for x coordinate
  for(j = 0; j < 0x1FF; j++);						//wait for receive FIFO
	RXY = SSIDR[0];                     //read data
	RXY = (RXY & 0x7FFF) >> 3;				//disregard first bit
	SUMY += RXY;
	CNTY++;
	
  input = true;
	PF[0x41C] |= 0x10;	//ack
	CORE[0x283] |= 0x40;	//unpend
}

void enableClock(){
	unsigned int i;
//  *(int*)&SYSCTL[0x060] = 0x8E3D40;		//sysclk 16MHz
  *(int*)&SYSCTL[0x060] = 0x14E1540;		//sysclk 66.67 MHz
  for(i = 0; i < 0xFF; i++);
	SYSCTL[0x608] = 0x3F;				//enable PA PB PC PD PE PF
  SYSCTL[0x61C] = 0x02;       // enable SSI1
	
	__asm__ {nop};
	__asm__ {nop};
	__asm__ {nop};
}
void enableSSI1(void)
{
  SSI1[0x004] = 0x00;           // SSICR1 SSI disable
	SSI1[0xFC8] = 0x00;           // SSICC = PIOSC
  SSI1[0x010] = 0x0A;           // SSICPSR (clk prescale) CPSDVSR = 2
  // SSICR0 SCR = 1 -> ~22.22 MHz / ( 2 * (1 + 1))
  *(int*)&SSI1[0x000] = 0x62E;           // SSICR0 Microwire, 16bits, SCR = 7
  SSI1[0x004] = 0x02;           // SSICR1 SSI enable  
}

void configurePorts()
{
	enableClock();

	enablePortA();
	enablePortB();
	enablePortC();
	enablePortD();
	enablePortE();
  enablePortF();
  enableSSI1();
	
	PA[0x3FC] = 0x00;
	PB[0x3FC] = 0xFF;
	PC[0x3FC] = 0x00;
	PD[0x3FC] = 0x00;
	PE[0x3FC] = 0x00;
	
	PA[0x3FC] |= 0xC4;						//RESET and RDX high
	PA[0x3FC] &= 0xF7;					// CSX chip select low
	PB[0x3FC] = 0x07;						// LEDs active low, TP_CS low
	
	//interrupts
  PF[0x404] = 0x10;                 // PF4 level sensitive GPIOIS
  PF[0x40C] = 0x00;                 // low level GPIOIEV
  PF[0x410] = 0x10;                 // PF4 GPIOIM
	CORE[0x103] = 0x40;         			// enable interrupts PF
}

void enablePortA(void)
{
	PA[0x420] = 0x00;						//AFSEL
	PA[0x51C] = 0xFF;						//DEN
	PA[0x400] = 0xFC;						//DIR
	PA[0x514] = 0xFF;					//PDR
}

void enablePortB(void)
{
	PB[0x420] = 0x00;						//AFSEL
	PB[0x51C] = 0xFF;						//DEN
	PB[0x400] = 0xFF;						//DIR
	PB[0x514] = 0xFF;						//PDR
}

void enablePortC(void)
{
	PC[0x51C] = 0xFF;						//DEN
	PC[0x400] = 0xFF;						//DIR
	PC[0x514] = 0xFF;					//PDR
}

void enablePortD(void)
{
	*(int*)&PD[0x520] = 0x4C4F434B;					//GPIO Unlcok
	PD[0x524] = 0x80;											//Commit register
	
	PD[0x51C] = 0xFF;						//DEN
	PD[0x400] = 0xFF;						//DIR
	PD[0x514] = 0xFF;					//PDR
}

void enablePortE(void)
{
	PE[0x51C] = 0xFF;						//DEN
	PE[0x400] = 0xFF;						//DIR
	PE[0x514] = 0xFF;					//PDR
}


void enablePortF(void)
{
  *(int*)&PF[0x520] = 0x4C4F434B;					//GPIO Unlock
	PF[0x524] = 0x01;											//Commit register
	
  PF[0x420] = 0x0F;                    //AFSEL
	PF[0x51C] = 0x1F;					        	  //DEN
	PF[0x400] = 0x0E;					        	//DIR - PF0 input
	PF[0x510] = 0x1D;										//PUR
	PF[0x514] = 0x02;					          //PDR
//	PF[0x50C] = 0x02;
  *(int*)&PF[0x52C] = 0x00002222;     //PCTL - SSI1
}

void flipRed(void)
{
	//PB0
	unsigned char val = (PB[0x3FC] & 0x1);
	if(val == 0x1){
		drawRedSquare();
		PB[0x3FC] &= 0xFE;
	}
	else{
		drawRedEmpty();
		PB[0x3FC] |= 0x1;
	}
}
void flipGreen(void)
{
	//PB1
	unsigned char val = (PB[0x3FC] & 0x2);
	if(val == 0x2){
		drawGreenSquare();
		PB[0x3FC] &= 0xFD;
	}
	else{
		drawGreenEmpty();
		PB[0x3FC] |= 0x2;
	}
}
void flipYel(void)
{
	//PB2
	unsigned char val = (PB[0x3FC] & 0x4);
	if(val == 0x4){
		drawYelSquare();
		PB[0x3FC] &= 0xFB;
	}
	else{
		drawYelEmpty();
		PB[0x3FC] |= 0x4;
	}
}


// PA2 - RST
// PA3 - CSX
// PA4 - D/CX
// PA5 - WRX
// PA6 - RDX
// PA7 - Backlight

// PB0 - Red LED
// PB1 - Green LED
// PB2 - Blue LED
// PB3 - TP_CS pin 30 

// PF - SSI1
// PF0 - Rx - SDO pin 32
// PF1 - Tx - SDI pin 34
// PF2 - Clk - CLK pin 33
// PF3 - Fss - TP_CS pin 30
// PF4 - PENIRQ - IRQ pin 31
