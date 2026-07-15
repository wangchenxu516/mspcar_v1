#ifndef HUIDU_H
#define HUIDU_H

#include "head.h"

extern volatile int   sensorPins[],Cycle_TIMES;
extern volatile float error1,derivative1,lastError,correction;
extern int HUIDU_test4,HUIDU_test;
void HUIDU(void);
void HUIDU_Track(void);
void HUIDU_Track_Reset(void);
void HUIDU_Track_SetSpeed(float speed);
void HUIDU_Track_Stop(void);
void Motor_Channel0_Test(void);

#endif
