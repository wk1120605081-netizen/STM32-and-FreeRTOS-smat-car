#ifndef __BSP_BLUETOOTH_H
#define __BSP_BLUETOOTH_H

#include "stm32f10x.h"

/* HC-05: 模块TXD -> PA10(USART1_RX), 模块RXD -> PA9(USART1_TX)
   VCC=5V GND, 默认波特率9600, 中断接收 */

void Bluetooth_Init(void);
uint8_t Bluetooth_Available(void);
char    Bluetooth_Read(void);
void    Bluetooth_SendChar(char c);
void    Bluetooth_SendString(char *s);

#endif
