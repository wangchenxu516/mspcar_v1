#ifndef TIMER_H
#define TIMER_H

void TIMER_0_INST_IRQHandler(void);
extern volatile float Count,Count1,Count2;
extern int cnt1,setoff;
extern volatile float angleError,kii,Yaw_error;
extern volatile float motorPosition;
#endif