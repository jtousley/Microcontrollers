#include "GPIO.H"

unsigned char *SYSCTL 	= (unsigned char *) 0x400FE000;
unsigned char *CORE 		= (unsigned char *) 0xE000E000;
unsigned char *TM0 			= (unsigned char *) 0x40030000;
unsigned char *TM1 			= (unsigned char *) 0x40031000;
unsigned char *TM2 			= (unsigned char *) 0x40032000;
unsigned char *CAN0 		= (unsigned char *) 0x40040000;
GPIO *PE = (GPIO *) 0x40024000;
//REG *PF = (REG *) 0x40025000;
REG *CTL				= (REG *) 0x40040000;
REG *STS 			= (REG *) 0x40040004;
REG *ERR 			= (REG *) 0x40040008;
REG *BIT 			= (REG *) 0x4004000C;
REG *INT 			= (REG *) 0x40040010;
REG *TST 			= (REG *) 0x40040014;
REG *BRPE 			= (REG *) 0x40040018;
REG *CRQ 		= (REG *) 0x40040020;
REG *CMSK		= (REG *) 0x40040024;
REG *MSK1		= (REG *) 0x40040028;
REG *MSK2		= (REG *) 0x4004002C;
REG *ARB1 	= (REG *) 0x40040030;
REG *ARB2		= (REG *) 0x40040034;
REG *MCTL		= (REG *) 0x40040038;
REG *DA1		= (REG *) 0x4004003C;
REG *DA2		= (REG *) 0x40040040;
REG *DB1		= (REG *) 0x40040044;
REG *DB2		= (REG *) 0x40040048;

int BUSY = 0;

void enableClock(void);
void enablePortE(void);
void enableCAN0(void);
void configurePorts(void);

int main(void)
{
	configurePorts();
		
		DA1->H._0 = 0xAAAA;
		DA2->H._0 = 0x0000;
		DB1->H._0 = 0x0000;
		DB2->H._0 = 0x0000;
		
		CRQ->B._0 = 0x02;
		if(CRQ->b._15 == 1){
			BUSY++;
		}
		MCTL->b._8 = 0x1;							//CANIF1MCTL TXRQST SEND MSG!
		CRQ->B._0 = 0x02;
		if(CRQ->b._15 == 1){
			BUSY++;
		}
		
	while(1);
}

void enableCAN0(void)
{
	unsigned int i;
	CTL->B._0 = 0x41;
//CAN0[0x0] |= 0x41;					//CCE enable; INIT enable
	//TSEG2 = 2; TSEG1 = 3; SJW = 0; BRP = 4;
	//bit time = 2us; t_q = .25us = BRP / sysclk = 4 / 16MHz
	BIT->H._0 = 0x2304;
	//*(int*)&CAN0[0x00C] = 0x2304;
	CTL->B._0 = 0x00;
	//CAN0[0x0] &= 0xBE;					//CCE disable; INIT disable
	
	MCTL->W = 0x00000000;
	
	for(i = 1; i < 0x21; i++){
		CRQ->B._0 = i;
		ARB2->b._15 = 0;		//MSGVAL message not valid
	}
	
	//config
		//WRNRD = 1; MSK = 1; ARB = 1; CTL = 1;
		//CLRINTPND = 0; NEWDAT = 0; DATAA = DATAB = 1;
		CMSK->B._0 |= 0xF3;
		//MSK[15:0] = 0;
		MSK1->H._0 = 0x0000;
		//MXTD = 0; MDIR = 0; MSK[28:16] = 0;
		MSK2->H._0 = 0x0000;
		//ID[15:0] = 0;
		ARB1->H._0 = 0x0000;
		//MSGVAL = 1; XTD = 0 (11bitID); DIR = 1(ignored); ID[28:16] = 0x7E0 (+8);
		ARB2->H._0 = (0x2FE8) << 2;
		//NEWDAT = X; MSGLST = X; INTPND = X; UMASK = 0;
		//TXIE = 0; RXIE = 0; RMTEN = 0; TXRQST = X; EOB = 1; DLC = 1;
		MCTL->H._0 = 0x0081;
	
	TST->b._2 = 0x1;		//enable Basic mode

	//interrupts
	//CORE[0x104] |= 0x80;			//enable interrupts CAN0
}

void enableClock(void)
{
	unsigned int i;
	*(int*)&SYSCTL[0x060] = 0x8E3D40;		//sysclk 16MHz
 // *(int*)&SYSCTL[0x060] = 0x14E1540;		//sysclk 66.67 MHz
  for(i = 0; i < 0xFF; i++);
	SYSCTL[0x608] |= 0x10;				//enable PE
	SYSCTL[0x103] |= 0x1;				//legacy enable CAN0 (hopefully)
  for(i = 0; i < 0xFF; i++);
}

void enablePortE(void)
{
	//PE4-5 CAN0
		PE->AFSEL.B._0 |= 0x30;
		PE->DEN.B._0 |= 0x30;
		PE->PCTL.B._2 = 0x88;		//CAN
		PE->DIR.b._5 = 0x1;			//PE5 output
		PE->DIR.b._4 = 0x0;			//PE4 input
}
void configurePorts(void)
{
	enableClock();
	enablePortE();
	enableCAN0();
}

