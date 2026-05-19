/*
 * stm_uart.h
 *
 *  Created on: 1 avr. 2021
 *      Author: bonnetst
 */

#ifndef UART_H_
#define UART_H_

#include <stdint.h>

#ifdef EMU

void cmsdk_usart_init(uint32_t baudrate);
void cmsdk_usart_write(char c);
int cmsdk_usart_read(void);

#define usart_init cmsdk_usart_init
#define usart_read cmsdk_usart_read
#define usart_write cmsdk_usart_write

#else

void stm_usart_init(uint32_t baudrate);
void stm_usart_write(char c);
int stm_usart_read(void);

#define usart_init stm_usart_init
#define usart_read stm_usart_read
#define usart_write stm_usart_write

#endif

#endif /* UART_H_ */
