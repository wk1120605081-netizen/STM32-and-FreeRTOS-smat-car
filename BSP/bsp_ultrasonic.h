#ifndef __BSP_ULTRASONIC_H
#define __BSP_ULTRASONIC_H

#include "stm32f10x.h"

/* HC-SR04: Trig=PB10(输出), Echo=PB11(输入)
   C8T6定时器已用完, 用 Cortex-M3 的 DWT->CYCCNT 测Echo高电平宽度, 不占定时器 */

void  Ultrasonic_Init(void);
float Ultrasonic_GetCm(void);     /* 返回距离cm; -1=超时无回波 */

#endif
