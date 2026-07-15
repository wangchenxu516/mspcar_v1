#include "head.h"
int KeyNum=0;

void check_buttons() {
    KeyNum = 0;  
    if (!DL_GPIO_readPins(KEY2_PORT, KEY2_PIN)) {
        delay_ms(1);  
        if (!DL_GPIO_readPins(KEY2_PORT, KEY2_PIN)) {  
            KeyNum = 1;
//按键消抖，消抖后确认按下key2
            while (!DL_GPIO_readPins(KEY2_PORT, KEY2_PIN));
            delay_ms(1);  
        }
    }
    
 
    if (!DL_GPIO_readPins(KEY1_PORT, KEY1_PIN)) {
        delay_ms(1); 
        if (!DL_GPIO_readPins(KEY1_PORT, KEY1_PIN)) {  
            KeyNum = 2;
//按键消抖，消抖后确认按下key1
            while (!DL_GPIO_readPins(KEY1_PORT, KEY1_PIN));
            delay_ms(1); 
        }
    }
    

    if (!DL_GPIO_readPins(KEY3_PORT, KEY3_PIN)) {
        delay_ms(1);  
        if (!DL_GPIO_readPins(KEY3_PORT, KEY3_PIN)) {  
            KeyNum = 3;
 //按键消抖，消抖后确认按下key3
            while (!DL_GPIO_readPins(KEY3_PORT, KEY3_PIN));
            delay_ms(1); 
        }
    }
}
