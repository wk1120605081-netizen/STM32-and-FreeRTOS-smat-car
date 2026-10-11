#include "bsp_servo.h"

void Servo_Init(void)
{
    GPIO_InitTypeDef g;
    TIM_TimeBaseInitTypeDef t;
    TIM_OCInitTypeDef oc;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_TIM1, ENABLE);

    g.GPIO_Mode = GPIO_Mode_AF_PP;
    g.GPIO_Speed = GPIO_Speed_50MHz;
    g.GPIO_Pin = GPIO_Pin_8;          /* PA8 */
    GPIO_Init(GPIOA, &g);

    /* 1MHz计数, 周期20000 = 20ms = 50Hz */
    t.TIM_Period = 19999;
    t.TIM_Prescaler = 71;
    t.TIM_ClockDivision = TIM_CKD_DIV1;
    t.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &t);

    oc.TIM_OCMode = TIM_OCMode_PWM1;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_Pulse = 1500;              /* 上电回中90度 */
    oc.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM1, &oc);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);

    TIM_CtrlPWMOutputs(TIM1, ENABLE); /* 高级定时器必须开主输出 */
    TIM_Cmd(TIM1, ENABLE);
}

void Servo_SetAngle(uint8_t angle)
{
    uint16_t ccr;
    if (angle > 180) angle = 180;
    ccr = 500 + (uint16_t)angle * 2000 / 180;   /* 500~2500us */
    TIM_SetCompare1(TIM1, ccr);
}
