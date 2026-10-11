#include "Delay.h"

/* 注意: 使用 FreeRTOS 时本文件的 Delay_ms 仅用于初始化阶段,
   任务里延时请用 vTaskDelay, 见 freertos_app.c */

void Delay_us(uint32_t xus)
{
    SysTick->LOAD = 72 * xus;
    SysTick->VAL = 0x00;
    SysTick->CTRL = 0x00000005;
    while (!(SysTick->CTRL & 0x00010000));
    SysTick->CTRL = 0x00000004;
    SysTick->VAL = 0x00;
}

void Delay_ms(uint32_t xms)
{
    while (xms--) Delay_us(1000);
}

void Delay_s(uint32_t xs)
{
    while (xs--) Delay_ms(1000);
}
