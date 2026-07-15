#ifndef __ENCODER_H
#define __ENCODER_H

#include "head.h"

void GROUP1_IRQHandler(void);

extern volatile int32_t gEncoderCount, gEncoderCount1,gEncodersum, gEncodersum1;

#endif