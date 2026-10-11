#include "bsp_encoder.h"

Encoder_t Encoder_Left;
Encoder_t Encoder_Right;

static void Encoder_TimInit(TIM_TypeDef *TIMx)
{
    TIM_TimeBaseInitTypeDef tim;
    TIM_ICInitTypeDef ic;

    tim.TIM_Period = 0xFFFF;
    tim.TIM_Prescaler = 0;
    tim.TIM_ClockDivision = TIM_CKD_DIV1;
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIMx, &tim);

    ic.TIM_Channel = TIM_Channel_1;
    ic.TIM_ICPolarity = TIM_ICPolarity_Rising;
    ic.TIM_ICSelection = TIM_ICSelection_DirectTI;
    ic.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    ic.TIM_ICFilter = 0x3;
    TIM_ICInit(TIMx, &ic);

    TIM_TIxExternalClockConfig(TIMx, TIM_TIxExternalCLK1Source_TI1,
                               TIM_ICPolarity_Rising, 0);
    TIM_SetCounter(TIMx, 0);
    TIM_Cmd(TIMx, ENABLE);
}

void Encoder_Init(void)
{
    GPIO_InitTypeDef gpio;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM4, ENABLE);

    gpio.GPIO_Mode = GPIO_Mode_IPU;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    gpio.GPIO_Pin = GPIO_Pin_0;     /* PA0 */
    GPIO_Init(GPIOA, &gpio);
    gpio.GPIO_Pin = GPIO_Pin_6;     /* PB6 */
    GPIO_Init(GPIOB, &gpio);

    Encoder_TimInit(TIM2);          /* 左 */
    Encoder_TimInit(TIM4);          /* 右 */
}

void Encoder_Update(void)
{
    Encoder_Left.cnt  = (int16_t)TIM_GetCounter(TIM2);
    Encoder_Right.cnt = (int16_t)TIM_GetCounter(TIM4);
    TIM_SetCounter(TIM2, 0);
    TIM_SetCounter(TIM4, 0);

    Encoder_Left.total  += Encoder_Left.cnt;
    Encoder_Right.total += Encoder_Right.cnt;

    Encoder_Left.rpm  = Encoder_Left.cnt  / SAMPLE_PERIOD_S / ENCODER_LINES * 60.0f;
    Encoder_Right.rpm = Encoder_Right.cnt / SAMPLE_PERIOD_S / ENCODER_LINES * 60.0f;

    Encoder_Left.speed  = Encoder_Left.cnt  / SAMPLE_PERIOD_S / ENCODER_LINES
                          * 3.1415926f * WHEEL_DIAMETER;
    Encoder_Right.speed = Encoder_Right.cnt / SAMPLE_PERIOD_S / ENCODER_LINES
                          * 3.1415926f * WHEEL_DIAMETER;
}
