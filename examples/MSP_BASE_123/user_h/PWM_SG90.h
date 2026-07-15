/*
 * 立创开发板软硬件资料与相关扩展板软硬件资料官网全部开源
 * 开发板官网：www.lckfb.com
 * 文档网站：wiki.lckfb.com
 * 技术支持常驻论坛，任何技术问题欢迎随时交流学习
 * 嘉立创社区问答：https://www.jlc-bbs.com/lckfb
 * 关注bilibili账号：【立创开发板】，掌握我们的最新动态！
 * 不靠卖板赚钱，以培养中国工程师为己任
 */

#ifndef __BSP_SG90_H__
#define __BSP_SG90_H__

#include "board.h"
extern float norm_x[4], norm_y[4];

unsigned int Get_Servo_Angle(void);
unsigned int Get_Servo_Angle1(void);

void Set_Servo_Angle_X( float angle);
void Set_Servo_Angle_Y( int angle);
void Set_Servo_Angle_D( int speed);
void Servo_Set(float x,int y,float j,int k,int l);//舵机角度控制函数
//void move_towards_target(float target_x,float target_y,float current_x,float current_y);//追踪点位控制
extern int servo_angle_x;  // 水平舵机角度
extern int servo_angle_y;  // 垂直舵机角度
extern float error_x,error_y;
extern float adjustment_x,adjustment_y;
void move_towards_target(float target_x,float current_x);

#endif /* __BSP_SG90_H__ */