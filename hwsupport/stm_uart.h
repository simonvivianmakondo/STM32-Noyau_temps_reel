/*
 * stm_uart.h
 *
 *  Created on: 1 avr. 2021
 *      Author: bonnetst
 */

#ifndef STM_UART_H_
#define STM_UART_H_

#include <stdint.h>

/**
 * @brief Initialise l'USART1 sur les broches PA9 (TX) et PA10 (RX)
 *        en mode 8N1, sans interruption.
 *
 * @param baudrate  Vitesse de communication souhaitée (ex: 115200)
 */
void stm_usart_init(uint32_t baudrate);

/**
 * @brief Envoie un caractère sur l'USART1 (bloquant).
 *
 * @param c  Caractère à transmettre
 */
void stm_usart_write(char c);

/**
 * @brief Lit un caractère depuis l'USART1 (bloquant).
 *
 * @return  Caractère reçu (dans les 8 bits de poids faible)
 */
int stm_usart_read(void);

#endif /* STM_UART_H_ */
