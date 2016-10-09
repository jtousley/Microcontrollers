#include "GPIO.h"

GPIO *PA = (GPIO *) 0x40004000;
GPIO *PB = (GPIO *) 0x40005000;
GPIO *PC = (GPIO *) 0x40006000;
GPIO *PD = (GPIO *) 0x40007000;
GPIO *PE = (GPIO *) 0x40024000;
GPIO *PF = (GPIO *) 0x40025000;
unsigned char *SYSCTL = (unsigned char *) 0x400FE000;
unsigned char *CORE = (unsigned char *) 0xE000E000;
unsigned char *ADC0 = (unsigned char *) 0x40038000;
unsigned char *TM0 = (unsigned char *) 0x40030000;
unsigned char *TM1 = (unsigned char *) 0x40031000;
unsigned char *TM2 = (unsigned char *) 0x40032000;
unsigned char *I2C1 = (unsigned char *) 0x40021000;

unsigned char tabAdd = 0;
unsigned short SINTABLE[40] = 
{
	0x800,0x940,0xa78,0xba1,0xcb3,
	0xda7,0xe78,0xf20,0xf9b,0xfe6,
	0xfff,0xfe6,0xf9b,0xf20,0xe78,
	0xda7,0xcb3,0xba1,0xa78,0x940,
	0x800,0x6bf,0x587,0x45e,0x34c,
	0x258,0x187,0xdf,0x64,0x19,
	0x0,0x19,0x64,0xdf,0x187,
	0x258,0x34c,0x45e,0x587,0x6bf
};

void enableClock(void);
void enablePortA(void);
void enablePortB(void);
void enablePortC(void);
void enablePortD(void);
void enablePortE(void);
void enablePortF(void);
void enableADC0(void);
void enableTM0(void);
void enableTM1(void);
void enableTM2(void);
void enableI2C1(void);
void configurePorts(void);

volatile unsigned int VOLTAGE = 0;

void ADC0SS0_Handler(void)
{
	ADC0[0x00C] |= 0x01;					//ack interrupt
	VOLTAGE = *(int*)&ADC0[0x48];			//Result FIFO
	
	PD->DATA.b._1 = ~PD->DATA.b._1;
}

void TIMER1A_Handler(void)
{
	TM1[0x024] |= 0x1;						//ack
	
	TM2[0x00C] &= 0x00;												//GPTMCTL
	*(int*)&TM2[0x028] = VOLTAGE + 0x7D0;			// change frequency
	TM2[0x00C] |= 0x1;												//start timer
}

void TIMER2A_Handler(void)
{
	unsigned short Tx = SINTABLE[tabAdd++];
	unsigned char B1 = (Tx >> 8);
	unsigned char B2 = (Tx & 0xFF);
	
	if(tabAdd == 39){
		tabAdd = 0;
	}
	TM2[0x024] |= 0x1;						//ack

	while((I2C1[0x4] & 0x1) == 0x1);		//wait MCS BUSY
	I2C1[0x008] = B1;							//MDR
	while((I2C1[0x4] & 0x40) == 0x40);	// wait MCS BUSBSY
	I2C1[0x004] = 0x03;						//MCS
	while((I2C1[0x4] & 0x1) == 0x1);		//wait MCS BUSY
	
	I2C1[0x008] = B2;							//MDR
	I2C1[0x004] = 0x05;						//MCS
	while((I2C1[0x4] & 0x1) == 0x1);		//wait MCS BUSY
	
}

int main(void)
{
	configurePorts();
	
	while(1);
}

void configurePorts()
{
	enableClock();

	enablePortA();
	enablePortD();
	enableADC0();
	enableTM0();
	enableI2C1();
	enableTM1();
	enableTM2();
}

void enableClock(){
	unsigned int i;
//  *(int*)&SYSCTL[0x060] = 0x8E3D40;		//sysclk 16MHz
  *(int*)&SYSCTL[0x060] = 0x24E1540;		//sysclk 40 MHz
  for(i = 0; i < 0xFF; i++);
	SYSCTL[0x608] = 0x09;				//enable PA PD PF
	SYSCTL[0x638] |= 0x01;				//enable ADC0
	SYSCTL[0x604] |= 0x07;				//TM0 TM1 TM2
	SYSCTL[0x620] |= 0x02;				// I2C1
	
	*(int*)&CORE[0x100] = 0xA04000;					//NVIC interrupt ADC0SS0 TM1A TM2A
}

void enablePortA(void)
{
	PA->AFSEL.B._0 = 0xC0;
	PA->DEN.B._0 = 0xC0;
	PA->DIR.B._0 = 0xC0;
	PA->PCTL.W = 0x33000000;
	PA->ODR.b._7 = 0x1;
	PA->PUR.B._0 |= 0xC0;
}

void enablePortD(void)
{
	PD->LOCK.W = 0x4C4F434B;					//GPIO Unlcok
	PD->CR.B._0 = 0x80;
	PD->DEN.B._0 = 0x00;
	PD->DIR.B._0 = 0x00;
	PD->AMSEL.b._0 = 0x1;
	
	PD->DEN.b._1 = 0x1;
	PD->DIR.b._1 = 0x1;
}

void enableADC0(void)
{
	ADC0[0x0] = 0x00;						//ADC0 disable
	ADC0[0x8] |= 0x01;						//SS0 interrupt enable
	ADC0[0x14] = 0x05;						//ADCEMUX trigger SS0 set timer 
	ADC0[0x040] = 0x7;						//ADCSSMUX0 select AIN7
	ADC0[0x44] |= 0x06;						//ADCSSCTL0 interrupt enable, 1st sample is end
	ADC0[0x0] |= 0x01;						//ADC SS0 enable
}

//Timer A
void enableTM0(void)
{
	TM0[0x00C] &= 0x00;					//GPTMCTL
	TM0[0x0] &= 0x00;					//GPTMCFG
	TM0[0x4] = 0x02;						//periodic
	*(int*)&TM0[0x028] = 0x13880;			// 2ms = INITIAL / 40 MHz
	TM0[0x00C] |= 0x21;					//start timer with ADC
}

//Timer B
void enableTM1(void)
{
	TM1[0x00C] &= 0x00;						//GPTMCTL
	TM1[0x0] &= 0x00;							//GPTMCFG
	TM1[0x4] = 0x02;							//periodic
	TM1[0x018] |= 0x1;						//enable interrupts GPTMIMR
	*(int*)&TM1[0x028] = 0x1312D00;			// 500ms = INITIAL / 40 MHz
	TM1[0x00C] |= 0x1;						//start timer 
}

//Timer C
void enableTM2(void)
{
	TM2[0x00C] &= 0x00;						//GPTMCTL
	TM2[0x0] &= 0x00;							//GPTMCFG
	TM2[0x4] = 0x02;							//periodic
	TM2[0x018] |= 0x1;						//enable interrupts GPTMIMR
	*(int*)&TM2[0x028] = 0xFCF;			// .1ms = INITIAL / 40 MHz
	TM2[0x00C] |= 0x1;						//start timer 
}

void enableI2C1(void)
{
	I2C1[0x004] = 0x00;		//I2C disable
	I2C1[0x020] |= 0x10;	//master function enable
	I2C1[0x00C] = 0x4;		//TPR = 4
	I2C1[0x000] = (0x63 << 1);		// slave address = 0x63
}
