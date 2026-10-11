#include "bsp_ultrasonic.h"
#include "Delay.h"

#define TRIG_PORT   GPIOB
#define TRIG_PIN    GPIO_Pin_10
#define ECHO_PORT   GPIOB
#define ECHO_PIN    GPIO_Pin_11

/* Cortex-M3 DWT 周期计数器固定地址, 不依赖 core_cm3.h 版本 */
#define DWT_CTRL                 (*(volatile uint32_t *)0xE0001000UL)
#define DWT_CYCCNT               (*(volatile uint32_t *)0xE0001004UL)
#define DEMCR                    (*(volatile uint32_t *)0xE000EDFCUL)
#define DEMCR_TRCENA_Msk         (1UL << 24)
#define DWT_CTRL_CYCCNTENA_Msk   (1UL << 0)

static void DWT_Init(void)
{
    DEMCR |= DEMCR_TRCENA_Msk;
    DWT_CYCCNT = 0;
    DWT_CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void Ultrasonic_Init(void)
{
    GPIO_InitTypeDef g;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    g.GPIO_Speed = GPIO_Speed_50MHz;

    g.GPIO_Mode = GPIO_Mode_Out_PP;
    g.GPIO_Pin = TRIG_PIN;
    GPIO_Init(GPIOB, &g);

    g.GPIO_Mode = GPIO_Mode_IPD;      /* Echo下拉输入 */
    g.GPIO_Pin = ECHO_PIN;
    GPIO_Init(GPIOB, &g);

    GPIO_ResetBits(TRIG_PORT, TRIG_PIN);
    DWT_Init();
}

float Ultrasonic_GetCm(void)
{
    uint32_t t0, t1, cycles;
    uint32_t guard;

    /* 触发: 10us高脉冲 */
    GPIO_ResetBits(TRIG_PORT, TRIG_PIN);
    Delay_us(2);
    GPIO_SetBits(TRIG_PORT, TRIG_PIN);
    Delay_us(10);
    GPIO_ResetBits(TRIG_PORT, TRIG_PIN);

    /* 等Echo拉高 */
    guard = 300000;
    while (GPIO_ReadInputDataBit(ECHO_PORT, ECHO_PIN) == Bit_RESET && --guard);
    if (guard == 0) return -1.0f;

    t0 = DWT_CYCCNT;

    /* 等Echo拉低 */
    guard = 600000;
    while (GPIO_ReadInputDataBit(ECHO_PORT, ECHO_PIN) == Bit_SET && --guard);
    t1 = DWT_CYCCNT;

    if (guard == 0) return -1.0f;

    cycles = t1 - t0;
    return (float)cycles / 72.0f / 58.0f;   /* us=cycles/72, cm=us/58 */
}
