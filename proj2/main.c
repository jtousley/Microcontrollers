//main.c
#include "GPIO.H"
#include "math.h"

volatile unsigned char *SYSCTL = (unsigned char *) 0x400FE000;
volatile unsigned char *CORE = (unsigned char *) 0xE000E000;
volatile unsigned char *TM0 = (unsigned char *) 0x40030000;
volatile unsigned char *TM1 = (unsigned char *) 0x40031000;
volatile unsigned char *TM2 = (unsigned char *) 0x40032000;
volatile unsigned int *CANMODULE = (unsigned int *) 0x40040000;
volatile CAN *CAN0 = (CAN *) 0x40040000;
volatile unsigned char *WDT0 = (unsigned char *) 0x40000000;

GPIO *PA = (GPIO *) 0x40004000;
GPIO *PC = (GPIO *) 0x40006000;
GPIO *PE = (GPIO *) 0x40024000;
GPIO *PF = (GPIO *) 0x40025000;

unsigned char ByteA = 0x00, ByteB = 0x07, ByteC = 0x65, ByteD = 0x00;

void enableClock(void);
void enablePortA(void);
void enablePortC(void);
void enablePortE(void);
void enableCAN0(void);
void enableWDT0(void);
void configurePorts(void);

void CAN0_Handler(void)
{
	unsigned short DA2;
	unsigned char PID;
	unsigned char STS;
	unsigned short NEWDAT,temp;
	unsigned char i;
	STS = CAN0->STS.B._0;
	CAN0->STS.B._0 = STS & ~0x18;
	if((STS & 0x10) != 0x10){
		//if it's not RXOK, I don't care
		return;
	}
	NEWDAT = CANMODULE[0x120/4];
	for(i = 0; i < 16; i++){
		NEWDAT = NEWDAT >> 1;
		if(NEWDAT == 0){
			NEWDAT = i + 1;
			break;
		}
	}

	if(NEWDAT == 0x00){
		NEWDAT = CANMODULE[0x124/4];
		for(i = 0; i < 16; i++){
			temp = NEWDAT & (1 << i);
			if(temp != 0){
				NEWDAT = i + 16;
				break;
			}
		}
	}

	CAN0->CRQR.B._0 = NEWDAT;
	CAN0->MCTL.H._0 = 0x0000;
	CAN0->MCTLR.H._0 = 0x0000;
	DA2 = CAN0->DA2R.H._0;
	PID = (DA2 & 0xFF);
	CAN0->ARB2.H._0 = (0x7E8 << 2) | 0x8000 | 0x2000; //transmit
	CAN0->CRQ.B._0 = NEWDAT;
	CAN0->MCTL.H._0 = 0x9088;
	switch(PID){
		case 0x00:
		//0x 06 41 00 BE 1F A8 13 00
		CAN0->DA1.H._0 = 0x4106;
		CAN0->DA2.H._0 = 0xBE00;
		CAN0->DB1.H._0 = 0xA81F;
		CAN0->DB2.H._0 = 0x0013;
		break;
		case 0x20:
		//0x 06 41 20 90 05 E0 15 00
		CAN0->DA1.H._0 = 0x4106;
		CAN0->DA2.H._0 = 0x9020;
		CAN0->DB1.H._0 = 0xE005;
		CAN0->DB2.H._0 = 0x0015;
		break;
		case 0x40:
		//0x 06 41 40 FE 1C 20 00 00
		CAN0->DA1.H._0 = 0x4106;
		CAN0->DA2.H._0 = 0xFE40;
		CAN0->DB1.H._0 = 0x201C;
		CAN0->DB2.H._0 = 0x0000;
		break;
		case 0x01:
		//0x 06 41 01 00 07 65 00 00
		CAN0->DA1.H._0 = 0x4106;
		CAN0->DA2.H._0 = (ByteA << 8) | 0x01;
		CAN0->DB1.H._0 = (ByteC << 8) | ByteB;
		CAN0->DB2.H._0 = (0x00 << 8) | ByteD;
		break;
		default:
		//should probably reset uc here
		SYSCTL[0x608] |= 0x20; //PF
		for(i = 0; i < 0xFE; i++);
		PF->DEN.b._1 = 0x1;
		PF->DIR.b._1 = 0x1;
		PF->DATA.b._1 = 0x1;
		//WDT0
		WDT0[0x008] |= 0x3;
		break;
	}
	CAN0->MCTL.H._0 |= 0x0100; //TXRQST
	CAN0->CRQ.B._0 = 0x01;
	CAN0->ARB2R.H._0 = (0x7DF << 2) | 0x8000; //receive
	CAN0->CRQR.B._0 = 0x01;
	CAN0->MCTL.H._0 = 0x1088;
}

int main(void)
{
	configurePorts();
	while(1);
}

void enableWDT0(void)
{
	*(int *)&WDT0[0x0] = 0xFFFF;
}

void enableCAN0(void)
{
	unsigned int i;
	CAN0->CTL.B._0 = 0x45; //INIT, SIE, CCE
	while((CAN0->CRQ.b._15 & 0x1) == 0x1); //wait for busy bit to clear
	//WRNRD, ARB, CONTROL = 1
	CAN0->CMSK.B._0 |= 0xB0;
	CAN0->ARB2.W = 0x0;
	CAN0->MCTL.W = 0x0;
	CAN0->BIT.H._0 = 0x2383;
	CAN0->CTL.B._0 = (CAN0->CTL.B._0 & ~0x40); //clear CCE
	for(i = 1; i <= 32; i++){
		while((CAN0->CRQ.b._15 & 0x1) == 0x1); //wait for busy bit to
		clear
		CAN0->CRQ.B._0 = i;
		CAN0->CRQR.B._0 = i;
	}
	CAN0->CMSK.B._0 = 0xF3;
	CAN0->MSK1.H._0 = 0x0;
	CAN0->MSK2.H._0 = (0x7DF << 2);
	CAN0->ARB1.H._0 = 0x0;
	CAN0->ARB2.H._0 = (0x7DF << 2) | 0x8000;
	CAN0->CMSKR.B._0 = 0x73;
	CAN0->MSK1R.H._0 = 0x0000;
	CAN0->MSK2R.H._0 = (0x7DF << 2);
	CAN0->ARB1R.H._0 = 0x0000;
	CAN0->ARB2R.H._0 = (0x7DF << 2) | 0x8000;
	for(i = 1; i <= 32; i++){
		while((CAN0->CRQ.b._15 & 0x1) == 0x1); //wait for busy bit to clear
		CAN0->CRQ.B._0 = i;
		CAN0->CRQR.B._0 = i;
	}

	CAN0->CTL.B._0 = (CAN0->CTL.B._0 & ~0x01);
	CAN0->CRQ.B._0 = 0x1;
	CAN0->CRQR.B._0 = 0x1;
	CAN0->MCTL.H._0 |= 0x1088; //UMASK, DLC = 8, EOB
	CAN0->MCTLR.H._0 |= 0x1088;
	CAN0->DA1R.H._0 = 0x0000;
	CAN0->DA2R.H._0 = 0x0000;
	CAN0->DB1R.H._0 = 0x0000;
	CAN0->DB2R.H._0 = 0x0000;
	//interrupts
	CAN0->CTL.b._1 = 0x1; //CTL IE
	CAN0->MCTLR.b._10 = 0x1; //MCTL RXIE
	CORE[0x104] |= 0x80; //enable interrupts CAN0
}

void enableClock(void)
{
	unsigned int i;
	*(int*)&SYSCTL[0x060] = 0x8E3D40; //sysclk 16MHz
	 for(i = 0; i < 0xFF; i++);
	SYSCTL[0x608] |= 0x15; //enable PE PC PA
	SYSCTL[0x103] |= 0x1; //legacy enable CAN0
	17
	SYSCTL[0x600] |= 0x1; //WDT0 clock enable
	 for(i = 0; i < 0xFF; i++);
}

void enablePortA(void)
{
	PA->DEN.B._0 |= 0xFC;
	PA->DIR.B._0 &= ~0x03;
	PA->PDR.B._0 |= 0xFC;
}

void enablePortC(void)
{
	PC->DEN.B._0 |= 0xF0;
	PC->DIR.B._0 &= ~0xF0;
	PC->PDR.B._0 |= 0xF0;
}

void enablePortE(void)
{
	//PE4-5 CAN0
	PE->AFSEL.B._0 |= 0x30;
	PE->DEN.B._0 |= 0x3F;
	PE->PCTL.B._2 = 0x88; //CAN
	PE->DIR.b._5 = 0x1; //PE5 output
	PE->DIR.b._4 = 0x0; //PE4 input
	PE->PDR.B._0 |= 0x10; //PDR
	PE->DIR.B._0 &= ~0x0F;
	PE->PDR.B._0 |= 0x0F;
}

void configurePorts(void)
{
	enableClock();
	enablePortA();
	enablePortC();
	enablePortE();
	enableCAN0();
	enableWDT0();
	ByteA = (~PA->DATA.b._7 << 7);
	ByteB = ((PA->DATA.B._0 & 0x70) >> 4);
	ByteC = (((PA->DATA.B._0 & 0xC) << 4) | ((PC->DATA.B._0 &0xF0) >> 2) | (PE>DATA.B._0 & 0x3));
}
