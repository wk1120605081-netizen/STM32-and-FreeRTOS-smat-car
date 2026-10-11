#ifndef __BSP_TRACKING_H
#define __BSP_TRACKING_H

#include "stm32f10x.h"

/* 五路循迹 OUT1~5 = PB12 PB13 PB14 PB15 PA12 (OUT1最左, OUT5最右) */

#define LINE_DETECTED   0    /* 检测到黑线时DO的电平(0或1), 先用程序读raw标定 */

void Tracking_Init(void);
uint8_t Tracking_GetRaw(void);    /* bit0~4=OUT1~5 原始电平(1=高) */
uint8_t Tracking_GetLine(void);   /* bit0~4=是否压到黑线(1=压) */
int8_t  Tracking_GetError(void);  /* -2~2 加权位置; 全丢线时保持上次 */

#endif
