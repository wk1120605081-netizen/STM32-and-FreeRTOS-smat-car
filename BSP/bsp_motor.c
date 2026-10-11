#include "bsp_motor.h"

static void Motor_GPIO_Init(void)
{
    GPIO_InitTypeDef g;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    g.GPIO_Mode = GPIO_Mode_Out_PP;
    g.GPIO_Speed = GPIO_Speed_50MHz;

    g.GPIO_Pin = AIN1_PIN | AIN2_PIN;
    GPIO_Init(GPIOA, &g);
    g.GPIO_Pin = BIN1_PIN | BIN2_PIN | STBY_PIN;
    GPIO_Init(GPIOB, &g);

    GPIO_ResetBits(GPIOA, AIN1_PIN | AIN2_PIN);
    GPIO_ResetBits(GPIOB, BIN1_PIN | BIN2_PIN | STBY_PIN);
}

static void Motor_PWM_Init(void)
{
    GPIO_InitTypeDef g;
    TIM_TimeBaseInitTypeDef t;
    TIM_OCInitTypeDef oc;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    g.GPIO_Mode = GPIO_Mode_AF_PP;
    g.GPIO_Speed = GPIO_Speed_50MHz;
    g.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &g);

    /* 1kHz: 72MHz/720/100 */
    t.TIM_Period = 99;
    t.TIM_Prescaler = 719;
    t.TIM_ClockDivision = TIM_CKD_DIV1;
    t.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &t);

    oc.TIM_OCMode = TIM_OCMode_PWM1;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_Pulse = 0;
    oc.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM3, &oc);
    TIM_OC2Init(TIM3, &oc);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_Cmd(TIM3, ENABLE);
}

void Motor_Init(void)
{
    Motor_GPIO_Init();
    Motor_PWM_Init();
    TIM_SetCompare1(TIM3, 0);
    TIM_SetCompare2(TIM3, 0);
    GPIO_SetBits(STBY_PORT, STBY_PIN);   /* 最后使能, 防自转 */
}

void Motor_SetLeft(int8_t s)
{
    if (s > 0) {
        GPIO_SetBits(AIN1_PORT, AIN1_PIN);
        GPIO_ResetBits(AIN2_PORT, AIN2_PIN);
    } else if (s < 0) {
        GPIO_ResetBits(AIN1_PORT, AIN1_PIN);
        GPIO_SetBits(AIN2_PORT, AIN2_PIN);
        s = -s;
    } else {
        GPIO_ResetBits(AIN1_PORT, AIN1_PIN);
        GPIO_ResetBits(AIN2_PORT, AIN2_PIN);
    }
    if (s > 100) s = 100;
    TIM_SetCompare1(TIM3, s);
}

void Motor_SetRight(int8_t s)
{
    if (s > 0) {
        GPIO_SetBits(BIN1_PORT, BIN1_PIN);
        GPIO_ResetBits(BIN2_PORT, BIN2_PIN);
    } else if (s < 0) {
        GPIO_ResetBits(BIN1_PORT, BIN1_PIN);
        GPIO_SetBits(BIN2_PORT, BIN2_PIN);
        s = -s;
    } else {
        GPIO_ResetBits(BIN1_PORT, BIN1_PIN);
        GPIO_ResetBits(BIN2_PORT, BIN2_PIN);
    }
    if (s > 100) s = 100;
    TIM_SetCompare2(TIM3, s);
}

void Motor_SetSpeed(int8_t l, int8_t r)
{
    Motor_SetLeft(l);
    Motor_SetRight(r);
}

void Motor_Stop(void)
{
    TIM_SetCompare1(TIM3, 0);
    TIM_SetCompare2(TIM3, 0);
    GPIO_ResetBits(AIN1_PORT, AIN1_PIN);
    GPIO_ResetBits(AIN2_PORT, AIN2_PIN);
    GPIO_ResetBits(BIN1_PORT, BIN1_PIN);
    GPIO_ResetBits(BIN2_PORT, BIN2_PIN);
}
