#ifndef __BSP_SERVO_H
#define __BSP_SERVO_H

#include "stm32f10x.h"

/* SG90 舵机 信号=PA8(TIM1_CH1), 50Hz, 0.5~2.5ms = 0~180度 */

void Servo_Init(void);
void Servo_SetAngle(uint8_t angle);    /* 0~180 */

#endif
