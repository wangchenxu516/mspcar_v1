#include "head.h"

volatile float Actual, Actual1,Actual_jy62;
volatile float Error0=0, Error1=0, Error2=0, Error3=0,Error0_jy62,Error1_jy62;
volatile float ErrorInt=0, ErrorInt1=0;
volatile float Out=1, Out1=1,Speed_Out,Speed_Out1;
volatile float Target, Target1,Speed,Speed1;
//volatile float Kp,Ki,Kd;
volatile float test;


	
void Motor(void)         //小车速度环  
{
//	Kp = 0.15; Ki =0.20; Kd = 0.1;
//	Actual = gEncoderCount; 
//	Error1 = Error0;
//	Error0 = Target - Actual;
//	ErrorInt += Error0;
//	Out = Kp * Error0 + Ki * ErrorInt + Kd * (Error0 - Error1);
//	if(Out >= 80) {
//		Out = 80;
//	}
//	if(Out <-80) {
//		Out = -80;
//	}


//	Actual1 = gEncoderCount1;
//	Error3 = Error2;
//	Error2 = Target1 - Actual1;
//	ErrorInt1 += Error2;
//	Out1 = Kp * Error2 + Ki * ErrorInt1 + Kd * (Error2 - Error3);
//	if(Out1 >= 80) {
//		Out1 = 80;
//	} 
//	if(Out1 <-80) {
//	Out1 = -80;
//	}
//	

//	 if(Out > 0)
//	 {
//         Speed_Out=Out;
//		DL_GPIO_setPins(PORTA_PORT, PORTA_PIN_14_PIN);
//		DL_GPIO_clearPins(PORTA_PORT,PORTA_PIN_13_PIN );
//	 }
//	else 
//	{
//         Speed_Out=-Out;
//		 DL_GPIO_setPins(PORTA_PORT,PORTA_PIN_13_PIN );
//		 DL_GPIO_clearPins(PORTA_PORT,PORTA_PIN_14_PIN );
//     }
//	
//	 
//	 
//	if(Out1 > 0) 
//	 {
//         Speed_Out1=Out1;
//		DL_GPIO_setPins(PORTA_PORT,PORTA_PIN_15_PIN );
//		DL_GPIO_clearPins(PORTA_PORT,PORTA_PIN_16_PIN );
//	 }
//	else
//	{
//        Speed_Out1=-Out1; 
//		DL_GPIO_setPins(PORTA_PORT, PORTA_PIN_16_PIN);
//		 DL_GPIO_clearPins(PORTA_PORT,PORTA_PIN_15_PIN );
//		
//	}
//	
//	Set_Duty(Speed_Out,  0);
//	Set_Duty(Speed_Out1, 1);
//	
//	gEncoderCount = 0; 
//	gEncoderCount1 = 0;
	
}

	



