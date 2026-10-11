#ifndef __BSP_BEEP_H
#define __BSP_BEEP_H

#include "stm32f10x.h"

/* 有源蜂鸣器(低电平触发) IO=PA11 */

void Beep_Init(void);
void Beep_On(void);
void Beep_Off(void);
void Beep_Blip(uint16_t ms);

#endif
