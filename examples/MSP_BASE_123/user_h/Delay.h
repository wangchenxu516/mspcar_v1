/* delay.h - 精准延时模块 */
#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

//-------------------------
// 时钟配置（根据实际系统时钟修改）
//-------------------------
#define CPU_FREQ_MHZ   16      // 系统主频（单位：MHz）

//-------------------------
// 函数声明
//-------------------------
void delay_init(void);          // 延时初始化（可选）
void delay_ms(uint32_t ms);     // 毫秒级延时
void delay_us(uint32_t us);     // 微秒级延时

#endif /* DELAY_H */