
/* key.h - 按键检测模块头文件 */
#ifndef KEY_H
#define KEY_H

#include <stdint.h>
#include <ti/devices/msp/msp.h>

//-------------------------
// 硬件引脚定义（需与实际电路一致）
//-------------------------
#define KEY1_PORT   PORTB_PORT   // KEY1端口
#define KEY1_PIN    GPIO_KEY_KEY_1_PIN
#define KEY2_PORT   PORTB_PORT   // KEY2端口
#define KEY2_PIN    GPIO_KEY_KEY_2_PIN
#define KEY3_PORT   PORTB_PORT   // KEY3端口
#define KEY3_PIN    GPIO_KEY_KEY_3_PIN

//-------------------------
// 函数声明
//-------------------------
void Key_Init(void);          // 按键初始化
void check_buttons();

extern int KeyNum,KeyNum2;
#endif /* KEY_H */