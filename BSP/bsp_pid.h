#ifndef __BSP_PID_H
#define __BSP_PID_H

#include "stm32f10x.h"

/* 增量式 PID 速度闭环, 每 100ms 计算一次(和编码器采样同步)
   反馈量 = Encoder 的 cnt(100ms脉冲数), 目标量也是 cnt */

typedef struct {
    int16_t target;     /* 目标 cnt(100ms脉冲) */
    int16_t feedback;   /* 实际 cnt */
    float   Kp;
    float   Ki;
    float   Kd;
    float   err;
    float   err_last;
    float   err_prev;
    float   out;        /* PID输出, 叠加到PWM */
    int8_t  pwm;        /* 最终PWM -100~100 */
    int8_t  pwm_base;   /* 基础PWM(前馈) */
} PID_t;

extern PID_t PID_Left;
extern PID_t PID_Right;

void  PID_Init(void);
void  PID_SetTarget(int16_t left_cnt, int16_t right_cnt);
void  PID_Calc(PID_t *p);          /* 每100ms对左右各调一次 */
void  PID_Update(void);           /* 读编码器+计算+输出, 100ms */
void  PID_RunFeedback(int16_t left_fb, int16_t right_fb); /* 给定反馈计算输出(不读编码器) */
void  PID_Stop(void);

#endif
