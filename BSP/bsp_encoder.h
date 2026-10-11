#ifndef __BSP_ENCODER_H
#define __BSP_ENCODER_H

#include "stm32f10x.h"

/* 左测速 DO=PA0(TIM2_CH1), 右测速 DO=PB6(TIM4_CH1) */

#define ENCODER_LINES     20      /* 码盘一圈缝数, 数实物后改 */
#define WHEEL_DIAMETER    0.065f  /* 轮径(米) */
#define SAMPLE_PERIOD_S   0.1f    /* 采样周期 100ms */

typedef struct {
    int16_t  cnt;        /* 本周期(100ms)脉冲数, PID直接用它做反馈 */
    int32_t  total;      /* 累计脉冲 */
    float    rpm;        /* 转/分 */
    float    speed;      /* 线速度 m/s */
} Encoder_t;

extern Encoder_t Encoder_Left;
extern Encoder_t Encoder_Right;

void Encoder_Init(void);
void Encoder_Update(void);   /* 固定每100ms调用一次 */

#endif
