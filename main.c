#include "main.h"
#include "N76E003.h"
#include "SFR_Macro.h"
#include "Function_define.h"
#include "Common.h"

#define RUN_MODE	0x01
#define STOP_MODE	0x02
#define CHILD_LOCK_STOP_MODE	0x04
#define CHARGE_MODE	0x08

#define BT_NO_PRESS	0
#define BT_SHORT_PRESS	1
#define BT_LONG_PRESS		2

void Timer0_Init();
float getVDD();
void set_MotorDuty(unsigned int value);
void RunMode();
void ChildLockMode();
void get_ButtonPress();
void Charging();
unsigned char u8DebounceTime, u8DebounceCount, u8ButtonState, u8TrimmerMode, u8ChargeMode;
volatile unsigned int u16Delay1ms, u16VddDelay;
unsigned char u8PowerUpMode;
void timer0_itCallback()	interrupt 1{
	TH0 = 0xC1;
	TL0 = 0x7F;
	
	if(u16Delay1ms){
		u16Delay1ms--;
	}
	if(u8DebounceTime){
		u8DebounceTime--;
	}
	if(u16VddDelay){
		u16VddDelay--;
	}
}

unsigned int temp;
void PinIT(void) interrupt 7
{
  clr_EPI;
	PIF=0x00;
	u8PowerUpMode=1;
//	u8ButtonState=BT_NO_PRESS;
//	u8TrimmerMode = STOP_MODE;
//	u8DebounceTime=0;
//	u8DebounceCount=0;
//	get_ButtonPress();
//	P12=1;
//	temp=100;
//	while(--temp){
//		nop;
//		nop;
//	}
//	P12=0;
}


void set_PWM_period(unsigned int value)
{
  PWMPL = (value & 0x00FF);
  PWMPH = ((value & 0xFF00) >> 8); 
}


	sfr PORDIS = 0xFD;
float Vdd,Duty;
void main(){
	P00_PushPull_Mode;
	P10_Input_Mode;
	P12_PushPull_Mode;
	P13_PushPull_Mode;
	P14_PushPull_Mode;
	P05_PushPull_Mode;
	P03_Input_Mode;
	P11_Input_Mode;

	SFRS = 0x00;
	TA = 0xAA;
	TA = 0x55;
	PORDIS = 0x5A;
	TA = 0xAA;
	TA = 0x55;
	PORDIS = 0xA5;
	
	Enable_BIT3_LowLevel_Trig;
  Enable_BIT1_HighLevel_Trig;
  Enable_INT_Port1;
	Enable_INT_Port0;
	PWM2_P05_OUTPUT_ENABLE;
	PWM_IMDEPENDENT_MODE;
	PWM_EDGE_TYPE;
	set_CLRPWM;
//	PWM_CLOCK_FSYS;
	PWM_CLOCK_DIV_32;
//	PWM1_OUTPUT_NORMAL;
	PWM2_OUTPUT_NORMAL;
	set_PWM_period(319);
	set_PWMRUN;
	u16Delay1ms=1000;
	set_EA;
	set_ADCEN;
	Timer0_Init();
	u8TrimmerMode=STOP_MODE;
	while(1){
			get_ButtonPress();
			if(!u8ChargeMode){
					if(u8TrimmerMode == RUN_MODE){
							RunMode();
					}
					else if(u8TrimmerMode == STOP_MODE){
							set_MotorDuty(0);
							P13=0;
							P12=0;
							P14=0;
							clr_PWMRUN;
							PWM2_P05_OUTPUT_DISABLE;
						P05=0;
							if(!u8PowerUpMode){
								PIF=0x00;
								set_EPI;
								set_PD;
							}
					}
					else if(u8TrimmerMode == CHILD_LOCK_STOP_MODE){
						ChildLockMode();
					}					
			}
			else{
					if(u8TrimmerMode == RUN_MODE){
							RunMode();
					}
					else if(u8TrimmerMode == STOP_MODE){
							if(!u16Delay1ms){
									u16Delay1ms=800;
									Charging();
							}
					}
					else if(u8TrimmerMode == CHILD_LOCK_STOP_MODE){
							u8TrimmerMode=STOP_MODE;
					}
			}
		
//			if(u8TrimmerMode == RUN_MODE){
//				RunMode();
//			}
//			else if(u8TrimmerMode == STOP_MODE){
//					set_MotorDuty(0);
//					P13=0;
//					P12=0;
//					P14=0;
//			}
//			else if(u8TrimmerMode == CHILD_LOCK_STOP_MODE){
//				ChildLockMode();
//			}
	}
}

void get_ButtonPress(){
	if(u8DebounceTime==0){
		u8DebounceTime=10;
		
		P00=1;
		nop;nop;nop;nop;
		u8ChargeMode=!P10;
		P00=0;
			if(u8PowerUpMode==1)
		u8PowerUpMode=0;
		
		if(P03==0){
			if(u8DebounceCount<250)
				u8DebounceCount++;
			if(u8ButtonState !=BT_LONG_PRESS){
				if(u8DebounceCount>150){
					u8ButtonState=2;
				}
				else if(u8DebounceCount>20){
					u8ButtonState=BT_SHORT_PRESS;
				}
				else{
					u8ButtonState=BT_NO_PRESS;
				}
				
				if(u8ButtonState==BT_LONG_PRESS){
					if(u8TrimmerMode==CHILD_LOCK_STOP_MODE){
						u8TrimmerMode = RUN_MODE;
						u16VddDelay=0;
					}
					else{
						u8TrimmerMode=CHILD_LOCK_STOP_MODE;
					}
				}
			}
		}
		else{
			u8DebounceCount=0;
			if(u8ButtonState==BT_SHORT_PRESS){
				if(u8TrimmerMode == RUN_MODE){
					u8TrimmerMode = STOP_MODE;
					u8ButtonState=BT_NO_PRESS;
					set_MotorDuty(0);
					clr_PWMRUN;
					u8PowerUpMode=0;
				}
				else if(u8TrimmerMode == STOP_MODE){
					u8TrimmerMode = RUN_MODE;
					u16VddDelay=0;
					P12=0;P13=0;P14=0;
				}
			}
			u8ButtonState=BT_NO_PRESS;
		}
	}
}

void RunMode(){
	if(PWMRUN==0){
		PWMRUN=1;
		PWM2_P05_OUTPUT_ENABLE;
	}
	if(u8PowerUpMode==1){
		u8PowerUpMode=0;
	}
	if(u16Delay1ms==0){
		if(u16VddDelay==0 && P14==0){
			set_MotorDuty(0);
			nop;
			nop;
			Vdd=getVDD()-100;
			Duty=3100/Vdd;
			if(Duty>1){
					u8TrimmerMode = STOP_MODE;
					set_MotorDuty(0);
					clr_PWMRUN;
					u8PowerUpMode=0;
					return;
			}
			Duty*=320;
			set_MotorDuty((uint16_t)(Duty-1));
			u16VddDelay=60000;
		}
//		Vdd=getVDD()-300;
//		Vdd= Vdd/3300;

		if(!u8ChargeMode){
			u16Delay1ms=1000;
			P12=0;
			P13=0;
		
			if(P14==0){
				if(Vdd>2500)
					P14=1;
				if(Vdd>3300)
					P13=1;
				if(Vdd>3900)
					P12=1;
			}
			else{
				P14=0;
				P13=0;
				P12=0;
			}
		}
		else{
			u16Delay1ms=800;
			Charging();
		}
	}
}


void ChildLockMode(){
	if(u8PowerUpMode==1)
		u8PowerUpMode=0;
	set_MotorDuty(0);
	if(u16Delay1ms==0){
			if(P13==0){
				u16Delay1ms=1000;
				P13=1;
				P12=1;
				P14=0;
			}
			else{
				u16Delay1ms=1500;
				P13=0;
				P12=0;
				P14=0;
			}
	}
}

void Charging(){
	if(P14==0){
		P14=1;
	}
	else if(P13==0){
		P13=1;
	}
	else if(P12==0){
		P12=1;
	}
	else{
		P12=0;
		P13=0;
		P14=0;
	}
}

void Timer0_Init(){
	TIMER0_MODE1_ENABLE;
	CKCON = 0x08;
//	TE0=1;
	IE|= 0x02;
	TH0 = 0xC1;
	TL0 = 0x7F;
	set_TR0;
}

double	Bandgap_Value, Bandgap_Voltage;
void READ_BANDGAP(){
	unsigned char BandgapHigh, BandgapLow;
	set_IAPEN;
	IAPAL = 0x0C;
	IAPAH = 0x00;
	IAPCN = 0x04;
	set_IAPGO;
	BandgapHigh=IAPFD;
	IAPAL=0x0d;
	IAPAH=0x00;
	IAPCN=0x04;
	set_IAPGO;
	BandgapLow=IAPFD;
	BandgapLow=BandgapLow&0x0F;
	clr_IAPEN;	//Disable IAPEN
	Bandgap_Value=(BandgapHigh<<4)+BandgapLow;
	Bandgap_Voltage=3072/(0x1000/Bandgap_Value);
}

double VDD_Voltage;
void ADC_Bypass(){
	unsigned char ozc;
	for(ozc=0;ozc<0x03;ozc++){
		clr_ADCF;
		set_ADCS;
		while(ADCF==0);
	}
}

float getVDD(){
	double bgvalue;
	READ_BANDGAP();
	Enable_ADC_BandGap;
	ADC_Bypass();
	u16Delay1ms=2;
	while(u16Delay1ms);
	clr_ADCF;
	set_ADCS;
	while(ADCF==0);
	bgvalue=(ADCRH<<4)+ADCRL;
	VDD_Voltage=Bandgap_Value*3072;//(0xfff/bgvalue)*Bandgap_Voltage;
	VDD_Voltage = VDD_Voltage/bgvalue;
	return VDD_Voltage;
}

void set_MotorDuty(unsigned int value)
{
  PWM2L = (value & 0x00FF);
  PWM2H = ((value & 0xFF00) >> 8);
  set_LOAD;
}