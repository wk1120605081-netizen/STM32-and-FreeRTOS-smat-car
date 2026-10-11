#ifndef __BSP_MOTOR_H
#define __BSP_MOTOR_H

#include "stm32f10x.h"

/* 引脚:
   A路(左): PWMA=PA6(TIM3_CH1)  AIN1=PA4  AIN2=PA5
   B路(右): PWMB=PA7(TIM3_CH2)  BIN1=PB0  BIN2=PB1
   STBY=PB5
*/
#define AIN1_PORT   GPIOA
#define AIN1_PIN    GPIO_Pin_4
#define AIN2_PORT   GPIOA
#define AIN2_PIN    GPIO_Pin_5
#define BIN1_PORT   GPIOB
#define BIN1_PIN    GPIO_Pin_0
#define BIN2_PORT   GPIOB
#define BIN2_PIN    GPIO_Pin_1
#define STBY_PORT   GPIOB
#define STBY_PIN    GPIO_Pin_5

void Motor_Init(void);
void Motor_SetLeft(int8_t speed);    /* -100~100, 负=反转 */
void Motor_SetRight(int8_t speed);
void Motor_SetSpeed(int8_t left, int8_t right);
void Motor_Stop(void);

#endif
