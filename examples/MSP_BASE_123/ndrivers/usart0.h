#ifndef _USART0_H  
#define _USART0_H
#include "head.h"

void usart0_Init(void);
void uart0_send_char(char ch);
void uart0_send_string(char* str);
void UART_0_INST_IRQHandler(void);

#endif
