#include "app.h"
#include "N76E003.h"
#include "SFR_Macro.h"
#include "Function_define.h"
#include "Common.h"
#include "timer.h"

//#define	set_IAPEN	BIT_TMP=EA;EA=0;TA=0xAA;TA=0x55;CHPCON|=SET_BIT0	;EA=BIT_TMP
//#define	set_IAPGO	BIT_TMP=EA;EA=0;TA=0xAA;TA=0x55;IAPTRG|=SET_BIT0	;EA=BIT_TMP
//#define	clr_IAPEN	BIT_TMP=EA;EA=0;TA=0xAA;TA=0x55;CHPCON&=~SET_BIT0	;EA=BIT_TMP




void Init(){
	
}

void ADC_Init(){
	
}

void Timer2_Init(){
	
}



unsigned char	u8AdcBusy, u16AdcData;
void ADC_Tasks(){
	
}

