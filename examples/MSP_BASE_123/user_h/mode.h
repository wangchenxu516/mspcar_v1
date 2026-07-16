#ifndef MODE_H
#define MODE_H
#include "head.h"

typedef enum
{
    TASK_WAIT_FIRST_START = 0,
    TASK_FIRST_RUNNING,
    TASK_WAIT_SECOND_START,
    TASK_SECOND_RUNNING,
    TASK_FINISHED
} TaskState;

void Task_Control_Init(void);
void Task_HandleStartButton(void);
void Task_Update(void);


void mode_1(void);
void mode_2(void);
void mode_3(void);
//void mode_4(void);

extern float leftSpeed,rightSpeed,Speed_jy62_L,Speed_jy62_R;
//extern int flag_Light_mode_2,cnt_mode3,Flag_mode3,
//Track_mode_flag,mode2_start,no_light,Flag_mode4_open,MODE_4_5,mode4_Turn_flag,mode4_Turn_cnt,Flag_mode4_run;

extern int mode1_times,MODE1_flag1,MODE1_start,MODE2_start,MODE3_start,MODE4_start,basespeed;

extern volatile TaskState taskState;
extern volatile uint16_t taskSavedTargetAngle;
extern volatile float taskTravelledAngle;
extern volatile float taskRemainingAngle;

#endif
