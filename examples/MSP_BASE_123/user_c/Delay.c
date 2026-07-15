
#include "delay.h"


#define LOOPS_PER_US   ((CPU_FREQ_MHZ * 5) / 4)  


void delay_init(void)
{
    // 如果使用定时器实现延时，可在此初始化
    // 当前使用空循环实现，无需初始化
}



void delay_ms(uint32_t ms)
{
    for (volatile uint32_t i = 0; i < ms; i++) {
        delay_us(1000); // 1ms = 1000us
    }
}



void delay_us(uint32_t us)
{
    volatile uint32_t cycles = us * LOOPS_PER_US;
    while(cycles--); 
}