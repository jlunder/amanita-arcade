/*
 * amanita_arcade.h
 *
 *  Created on: Oct 13, 2015
 *      Author: jlunder
 */

#ifndef AMANITA_ARCADE_H_
#define AMANITA_ARCADE_H_


#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "cortexm/ExceptionHandlers.h"

#include "diag/Trace.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_i2c.h"
#include "stm32f4xx_hal_i2s.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_tim.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_usart.h"
#pragma GCC diagnostic pop

#include "core_util.h"


typedef struct {
	uint8_t b, r, g;
} aa_color_t;

typedef struct {
	int32_t r, g, b;
} aa_pal_color_t;


extern uint8_t const aa_cie_table[4096];
extern aa_pal_color_t const aa_orange_pink_pal[256];


#endif /* AMANITA_ARCADE_H_ */
