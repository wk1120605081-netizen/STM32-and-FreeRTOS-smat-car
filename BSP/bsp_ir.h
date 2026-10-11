#ifndef __BSP_IR_H
#define __BSP_IR_H

#include "stm32f10x.h"

/* 左右盲区红外(TCRT5000): 左DO=PA1, 右DO=PA2, 斜45度装车头两侧 */

#define IR_DETECTED   0    /* 检测到障碍时DO电平, 实测改 */

void IR_Init(void);
uint8_t IR_LeftBlocked(void);
uint8_t IR_RightBlocked(void);

#endif
