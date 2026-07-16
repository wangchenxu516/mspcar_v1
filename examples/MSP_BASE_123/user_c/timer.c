#include "head.h"
int cnt1,setoff;
volatile float Count,Count1,Count2;//Count ：计数器定义，用于计数；Count1：步进电机已发出的净脉冲数
volatile float motorPosition = 0;         
volatile float angleError,kii,Yaw_error;
void TIMER_0_INST_IRQHandler(void)  
{
        switch (DL_TimerG_getPendingInterrupt(TIMER_0_INST)) {
        case DL_TIMER_IIDX_ZERO:	
       
        Count++;
 //10ms触发逻辑，在每一次mode所有指令（圈数达到）结束后，每10ms执行一次扫描一次模式
		if(Count==5)
   {	   		
		 Actual_Yaw_cal();
		 Task_Update();
	   Count=0;
   } 
//每10msHUIDU()-> HUIDU_Track()执行，直到所有内容执行完毕

   
if(motorPosition<angleError)
{ 
	DL_GPIO_clearPins(GPIOA, PORTB_Step_Dir_PIN);
	DL_GPIO_setPins(GPIOA, PORTB_Step_PIN);
	Count1++;
	DL_GPIO_clearPins(GPIOA, PORTB_Step_PIN);
	
}

	 if(motorPosition>angleError+0.3)
{
	DL_GPIO_setPins(GPIOA, PORTB_Step_Dir_PIN);
	DL_GPIO_setPins(GPIOA, PORTB_Step_PIN);
	Count1--;
	DL_GPIO_clearPins(GPIOA, PORTB_Step_PIN);
}
	motorPosition=0.225*Count1;
		
	
            break;
            default:
            break;
    }
}
