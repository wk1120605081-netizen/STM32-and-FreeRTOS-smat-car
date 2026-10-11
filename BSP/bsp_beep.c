#include "bsp_beep.h"
#include "Delay.h"

#define BEEP_PORT   GPIOA
#define BEEP_PIN    GPIO_Pin_11

void Beep_Init(void)
{
    GPIO_InitTypeDef g;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    g.GPIO_Mode = GPIO_Mode_Out_PP;
    g.GPIO_Speed = GPIO_Speed_50MHz;
    g.GPIO_Pin = BEEP_PIN;
    GPIO_Init(BEEP_PORT, &g);

    GPIO_SetBits(BEEP_PORT, BEEP_PIN);   /* 高电平=不响 */
}

void Beep_On(void)  { GPIO_ResetBits(BEEP_PORT, BEEP_PIN); }  /* 低=响 */
void Beep_Off(void) { GPIO_SetBits(BEEP_PORT, BEEP_PIN); }

void Beep_Blip(uint16_t ms)
{
    Beep_On();
    Delay_ms(ms);
    Beep_Off();
}
