#include "head.h"  

volatile int32_t gEncoderCount, gEncoderCount1,gEncodersum, gEncodersum1;

uint32_t gpio_interrup;
void GROUP1_IRQHandler(void)
{
//	//获取中断信号
//	gpio_interrup = DL_GPIO_getEnabledInterruptStatus(GPIOB, GPIO_EncoderA_PIN_0_PIN | GPIO_EncoderA_PIN_1_PIN | GPIO_EncoderA_PIN_2_PIN | GPIO_EncoderA_PIN_3_PIN);
//	//encoderA
//	if((gpio_interrup & GPIO_EncoderA_PIN_0_PIN)==GPIO_EncoderA_PIN_0_PIN)
//	{
//		if(!DL_GPIO_readPins(GPIOB,GPIO_EncoderA_PIN_1_PIN))
//		{
//			  gEncoderCount--;
//    		  gEncodersum--;
//		}
//		else
//		{
//			  gEncoderCount++;
//    		  gEncodersum++;
//		}
//	}
//	else if((gpio_interrup & GPIO_EncoderA_PIN_1_PIN)==GPIO_EncoderA_PIN_1_PIN)
//	{
//		if(!DL_GPIO_readPins(GPIOB,GPIO_EncoderA_PIN_0_PIN))
//		{
//			  gEncoderCount++;
//    		  gEncodersum++;
//		}
//		else
//		{
//			  gEncoderCount--;
//    		  gEncodersum--;
//		}
//	}
//	//encoderB
//	if((gpio_interrup & GPIO_EncoderA_PIN_2_PIN)==GPIO_EncoderA_PIN_2_PIN)
//	{
//		if(!DL_GPIO_readPins(GPIOB,GPIO_EncoderA_PIN_3_PIN))
//		{
//			  gEncoderCount1--;
//			  gEncodersum1--;
//		}
//		else
//		{
//			  gEncoderCount1++;
//			  gEncodersum1++;
//		}
//	}
//	else if((gpio_interrup & GPIO_EncoderA_PIN_3_PIN)==GPIO_EncoderA_PIN_3_PIN)
//	{
//		if(!DL_GPIO_readPins(GPIOB,GPIO_EncoderA_PIN_2_PIN))
//		{
//			  gEncoderCount1++;
//			  gEncodersum1++;
//		}
//		else
//		{
//			  gEncoderCount1--;
//			  gEncodersum1--;
//		}
//	}
//	DL_GPIO_clearInterruptStatus(GPIOB,GPIO_EncoderA_PIN_0_PIN | GPIO_EncoderA_PIN_1_PIN|GPIO_EncoderA_PIN_2_PIN|GPIO_EncoderA_PIN_3_PIN);
}