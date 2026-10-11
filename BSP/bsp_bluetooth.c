#include "bsp_bluetooth.h"

static volatile char    g_rx;
static volatile uint8_t g_flag;

void Bluetooth_Init(void)
{
    GPIO_InitTypeDef g;
    USART_InitTypeDef u;
    NVIC_InitTypeDef nvic;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_USART1, ENABLE);

    /* PA9 TX 复用推挽 */
    g.GPIO_Mode = GPIO_Mode_AF_PP;
    g.GPIO_Speed = GPIO_Speed_50MHz;
    g.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOA, &g);

    /* PA10 RX 上拉输入 */
    g.GPIO_Mode = GPIO_Mode_IPU;
    g.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOA, &g);

    u.USART_BaudRate = 9600;
    u.USART_WordLength = USART_WordLength_8b;
    u.USART_StopBits = USART_StopBits_1;
    u.USART_Parity = USART_Parity_No;
    u.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    u.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &u);

    nvic.NVIC_IRQChannel = USART1_IRQn;
    nvic.NVIC_IRQChannelPreemptionPriority = 2;
    nvic.NVIC_IRQChannelSubPriority = 0;
    nvic.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_Cmd(USART1, ENABLE);
}

uint8_t Bluetooth_Available(void) { return g_flag; }

char Bluetooth_Read(void)
{
    g_flag = 0;
    return g_rx;
}

void Bluetooth_SendChar(char c)
{
    USART_SendData(USART1, (uint8_t)c);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void Bluetooth_SendString(char *s)
{
    while (*s) Bluetooth_SendChar(*s++);
}

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        g_rx = (char)USART_ReceiveData(USART1);
        g_flag = 1;
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}
