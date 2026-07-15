#include "head.h"
#define DELAY (3200000)
#define DATA_NUM 3
int32_t gz_sum = 0;  
int16_t gz_avg = 0; 


int pos_data[3] = {320,0,0};
int cnt=0,mode_flag=0;
int location_x=0,cnt_Light_mode_4,mode_4_light_voice,MOED2_Seting=0;
int location_y=0;

	
int main(void)
{
	SYSCFG_DL_init();
	oled_init();

	MPU6050_Init();
    delay_ms(300);
	 for(uint16_t i = 0; i < 100; i++)
    {
        MPU6050_GetData(&AX, &AY, &AZ, &GX, &GY, &GZ);
        gz_sum += GZ;
        delay_us(8);  
    }
	gz_avg = (int16_t)(gz_sum / 100);
	jy61p_Init();
	NVIC_EnableIRQ(GPIO_EncoderA_INT_IRQN);
	NVIC_EnableIRQ(TIMER_0_INST_INT_IRQN);
    NVIC_EnableIRQ(Motor_INST_INT_IRQN);
	NVIC_EnableIRQ(Steer_INST_INT_IRQN);
	DL_TimerG_startCounter(Motor_INST);
	DL_TimerA_startCounter(TIMER_0_INST);
	DL_TimerA_startCounter(Steer_INST);
	int test1=0,test2=0,test3=0,lock=0,clear_oled=0;

	while(1)
	{   
     
		check_buttons();  //按键检测
		
		 if( KeyNum ==1 && lock==0)
	{
		test1++;
		if(test1>2)
		{
			test1=0;
			clear_oled=0;
		}
	}
		
/////////////////////////////////////////////////////////模式一	
	if(test1 == 0 )
{
		if(lock==0)
		{
		display_6_8_string(1,1,"MODE_1               ");
		display_6_8_string(1,2,"Cycles        :      ");
		display_6_8_number(90,2,mode1_times);
		display_6_8_string(1,3,"Now_Angle     :      ");
		display_6_8_number(90,3,Rec_Yaw);

		display_6_8_string(1,6,"KEY2 SET TIMES !!! ");
		display_6_8_string(1,7,"KEY3 SET SPEED !!! ");
		}
		
		if( KeyNum == 2 && lock == 0 )
	{
		mode1_times++;
		if(mode1_times>5)
		mode1_times=1;	
	}
		if( KeyNum == 3 && lock == 0)
	{
		for(int i=0;i<8;i++)
		LCD_clear_L(0,i);
		lock=1;
		display_6_8_string(1,1,"SPEED :              ");
		display_6_8_number(90,1,basespeed);
        display_6_8_string(1,3,"KEY_1_TO_SUB         ");
        display_6_8_string(1,4,"KEY_2_TO_ADD         ");
        display_6_8_string(1,7,"KEY_3_TO_START       ");			
		KeyNum =0;	

	}
	if(KeyNum == 1 && lock == 1)
	{
		basespeed--;
        if(basespeed<10)	
		basespeed=10;  
		display_6_8_number(90,1,basespeed);
	}
	if(KeyNum == 2 &&lock == 1)
	{
		
		basespeed++;
		if(basespeed>25)	
		basespeed=25;  
		display_6_8_number(90,1,basespeed);
	}
	if(KeyNum == 3 && lock == 1)
	{
		MODE1_start=1;
	}
//     display_6_8_string(1,0,"                     ");
//	display_6_8_number( 80,0,HUIDU_test4);
}
////////////////////////////////////////////////////////////模式二		
	  if(test1 == 1 )
	{
        if(clear_oled==0)
		{
		for(int i=0;i<8;i++)
		LCD_clear_L(0,i);
		clear_oled=1;
		}
		display_6_8_string(1,1,"MODE_2_3             ");
		display_6_8_string(1,2,"NOW_Angle   :        ");	
		display_6_8_number(90,2,Rec_Yaw); 
		display_6_8_string(1,3,"NOW_Picture :        ");	
		display_6_8_number(90,3,Roll); 
	    display_6_8_string(1,7,"KEY3 TO Enter        ");
	             
	  if( KeyNum == 3&&lock == 0)
	{
		   lock=1;	
           DL_GPIO_setPins(PORTA_PORT, PORTA_beep_PIN);   		
		   display_6_8_string(1,4,"KEY2_Record :        ");
		   KeyNum=0;
	}

	if(  KeyNum == 2&&lock == 1)
	{ 
		 lock=2;
		 Target_Yaw=Rec_Yaw;
		 display_6_8_number(80,4,Target_Yaw);
		
	}
	 if(KeyNum == 3&&lock == 2)
	 {
		 MODE2_start=1;
		  DL_GPIO_clearPins(PORTA_PORT, PORTA_beep_PIN);   		
	 }
}
/////////////////////////////////////////////////////////////////模式三	
	if(test1 == 2 )
{
		if(lock ==0)
		{
			basespeed=12;
		display_6_8_string(1,1,"MODE_4               ");
		display_6_8_string(1,2,"Cycles        :       ");
		display_6_8_number(90,2,mode1_times);
		display_6_8_string(1,3,"Now_Angle     :       ");
		display_6_8_number(90,3,Rec_Yaw);
        display_6_8_string(1,4,"NOW_Picture   :       ");	
		display_6_8_number(90,4,Roll); 
		display_6_8_string(1,6,"KEY2 SET TIMES !!! ");
		display_6_8_string(1,7,"KEY3 SET SPEED !!! ");
		}
		
		if( KeyNum ==2&&lock==0 )
	{
		mode1_times++;
		if(mode1_times>2)
		mode1_times=1;	
	}
		if( KeyNum ==3&&lock==0)
	{
		for(int i=0;i<8;i++)
		LCD_clear_L(0,i);
		lock=1;
		display_6_8_string(1,1,"SPEED :              ");
		display_6_8_number(80,1,basespeed);
        display_6_8_string(1,3,"KEY_1_TO_SUB         ");
        display_6_8_string(1,4,"KEY_2_TO_ADD         ");
        display_6_8_string(1,7,"KEY_3_TO_START       ");			
		KeyNum =0;	

	}
	if(KeyNum ==1&&lock==1)
	{
		
		basespeed--;
        if(basespeed<10)	
		basespeed=10;  
		display_6_8_number(80,1,basespeed);
	}
	if(KeyNum ==2&&lock==1)
	{
		
		basespeed++;
		if(basespeed>25)	
		basespeed=25; 
		display_6_8_number(80,1,basespeed); 
	}
	if(KeyNum ==3&&lock==1)
	{
		MODE3_start=1;
	}
	
}


		 HUIDU();
	display_6_8_number(0, 0,sensorPins[0]);
	display_6_8_number(0, 1,sensorPins[1]);	
	display_6_8_number(0, 2,sensorPins[2]);	 
	display_6_8_number(0, 3,sensorPins[3]);	
	display_6_8_number(0, 4,sensorPins[4]);	
	display_6_8_number(0, 5,sensorPins[5]);	
	display_6_8_number(0, 6,sensorPins[6]);	
	display_6_8_number(0, 7,sensorPins[7]);	

   }
}





