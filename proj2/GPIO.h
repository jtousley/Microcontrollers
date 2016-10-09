typedef struct{
	volatile unsigned int _0:1,_1:1,_2:1,_3:1,_4:1,_5:1,_6:1,_7:1,
	_8:1,_9:1,_10:1,_11:1,_12:1,_13:1,_14:1,_15:1,
	_16:1,_17:1,_18:1,_19:1,_20:1,_21:1,_22:1,_23:1,
	_24:1,_25:1,_26:1,_27:1,_28:1,_29:1,_30:1,_31:1;
}bitReg;

typedef struct{
	volatile unsigned char _0,_1,_2,_3;
}byteReg;

typedef struct{
	volatile unsigned short _0,_1;
}halfwordReg;

//access register via bit, byte, halfword, and word
typedef union{
	bitReg b;
	byteReg B;
	halfwordReg H;
	volatile unsigned int W;
}REG;

typedef struct{
	volatile unsigned char DATA_[1019]; //TI bit addresses
	REG DATA; //0x3FC
	REG DIR; //0x400
	REG IS; //0x404
	REG IBE; //0x408
	REG IEV; //0x40C
	REG IM; //0x410
	REG RIS; //0x414
	REG MIS; //0x418
	REG ICR; //0x41C
	REG AFSEL; //0x420
	volatile unsigned char RES[220]; //reserved
	REG DR2R; //0x500
	REG DR4R; //0x504
	REG DR8R; //0x508
	REG ODR; //0x50C
	REG PUR; //0x510
	REG PDR; //0x514
	REG SLR; //0x518
	REG DEN; //0x51C
	REG LOCK; //0x520
	REG CR; //0x524
	REG AMSEL; //0x528
	REG PCTL; //0x52C
}GPIO;
