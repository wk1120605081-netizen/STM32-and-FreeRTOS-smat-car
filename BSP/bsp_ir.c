#include "bsp_ir.h"

void IR_Init(void)
{
    GPIO_InitTypeDef g;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    g.GPIO_Mode = GPIO_Mode_IPU;
    g.GPIO_Speed = GPIO_Speed_50MHz;
    g.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_Init(GPIOA, &g);
}

uint8_t IR_LeftBlocked(void)
{
    return (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_1) == IR_DETECTED);
}

uint8_t IR_RightBlocked(void)
{
    return (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == IR_DETECTED);
}
