#include "head.h"
/*0-100 duty调节转弯时的电机转速*/
	void Set_Duty(float duty, uint8_t channel)
	{
    uint32_t CompareValue=0;		
		CompareValue = 32000 - 320*duty;//320*100=32000，调节duty（1-100）得到对应的compare值调节电机高电平占空比调节转速
		if(channel==0)	
		{ 	
			DL_TimerG_setCaptureCompareValue(Motor_INST,CompareValue, DL_TIMER_CC_0_INDEX);//左电机使能
		}
		
		if(channel==1)
			
		{  	
			DL_TimerG_setCaptureCompareValue(Motor_INST,CompareValue, DL_TIMER_CC_1_INDEX);//右电机使能
		}
			   
	}		
