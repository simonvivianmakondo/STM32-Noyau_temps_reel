/*
 * stm32h7xx.h
 *
 *  Created on: 25 févr. 2021
 *      Author: bonnetst
 */

#ifndef STM32H7XX_H_
#define STM32H7XX_H_

#ifdef EMU

#define CORE_CLK (25000000)
#define SYSPR_CLK (25000000)

#else

#define CORE_CLK (64000000)
#define SYSPR_CLK (64000000)

#endif

#include "cortex.h"
#include "stm_gpio.h"
#include "uart.h"
#include "stm_rcc.h"

#endif /* STM32H7XX_H_ */
