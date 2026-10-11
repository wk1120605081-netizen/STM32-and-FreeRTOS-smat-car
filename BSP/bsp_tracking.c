#include "bsp_tracking.h"

/* 引脚与位序对应表 */
static GPIO_TypeDef * const TR_PORT[5] = {GPIOB, GPIOB, GPIOB, GPIOB, GPIOA};
static const uint16_t TR_PIN[5] = {
    GPIO_Pin_12, GPIO_Pin_13, GPIO_Pin_14, GPIO_Pin_15, GPIO_Pin_12
};
static const int8_t TR_WEIGHT[5] = {-2, -1, 0, 1, 2};

void Tracking_Init(void)
{
    GPIO_InitTypeDef g;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

    g.GPIO_Mode = GPIO_Mode_IPU;
    g.GPIO_Speed = GPIO_Speed_50MHz;

    g.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOB, &g);
    g.GPIO_Pin = GPIO_Pin_12;
    GPIO_Init(GPIOA, &g);
}

uint8_t Tracking_GetRaw(void)
{
    uint8_t raw = 0, i;
    for (i = 0; i < 5; i++) {
        if (GPIO_ReadInputDataBit(TR_PORT[i], TR_PIN[i]) == Bit_SET)
            raw |= (1 << i);
    }
    return raw;
}

uint8_t Tracking_GetLine(void)
{
    uint8_t line = 0, i;
    for (i = 0; i < 5; i++) {
        if (GPIO_ReadInputDataBit(TR_PORT[i], TR_PIN[i]) == LINE_DETECTED)
            line |= (1 << i);
    }
    return line;
}

int8_t Tracking_GetError(void)
{
    static int8_t last = 0;
    int32_t wsum = 0, cnt = 0;
    int8_t i;

    for (i = 0; i < 5; i++) {
        if (GPIO_ReadInputDataBit(TR_PORT[i], TR_PIN[i]) == LINE_DETECTED) {
            wsum += TR_WEIGHT[i];
            cnt++;
        }
    }

    if (cnt == 0) return last;      /* 丢线保持, 由上层决定急停/找线 */

    last = (int8_t)(wsum / cnt);
    return last;
}
