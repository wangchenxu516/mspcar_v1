#include "head.h"


//声光提示以及OLED控制
int cnt_clean;

void buzzer_light_setup()
{
	 DL_GPIO_setPins(PORTA_PORT, PORTA_beep_PIN);   
	 DL_GPIO_setPins(PORTB_PORT, PORTB_Red_PIN);     
}

void buzzer_light_clear()
{
	DL_GPIO_clearPins(PORTA_PORT, PORTA_beep_PIN);
	DL_GPIO_clearPins(PORTB_PORT, PORTB_Red_PIN);
}

void buzzer_light_up_and_down()
{
	
	 buzzer_light_setup();
	 delay_ms(15);
	 buzzer_light_clear();
}
void buzzer_light_up_and_down_double()
{
	 buzzer_light_setup();
	 delay_ms(15);
	 buzzer_light_clear();
	 delay_ms(10);
	 buzzer_light_setup();
	 delay_ms(15);
	 buzzer_light_clear();

}
void OLED_clean()
{	
	cnt_clean++;
	if(cnt_clean>=300)
	{
		LCD_clear_L(0,0);
		LCD_clear_L(0,1);
		LCD_clear_L(0,2);
		LCD_clear_L(0,3);
		LCD_clear_L(0,4);
		LCD_clear_L(0,5);
		LCD_clear_L(0,6);
		LCD_clear_L(0,7);
		cnt_clean=0;
	}
}

