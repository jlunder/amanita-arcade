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
#include "stm32f4xx_hal_rcc_ex.h"
#include "stm32f4xx_hal_spi.h"
#include "stm32f4xx_hal_tim.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpadded"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_usart.h"
#pragma GCC diagnostic pop

#include "core_util.h"

#ifdef DEBUG
#define AA_TEST_HARDWARE
#else
#define AA_PRODUCTION_HARDWARE
#endif

#define AA_FIX12_ONE 4096
#define AA_FIX16_ONE 65536
#define AA_COLOR_ONE AA_FIX16_ONE

#define AA_TIME_FROM_MILLIS_INIT(x) (x)


typedef union {
	struct {
		int32_t r, g, b, a;
	};
	// No NEON on Cortex-M4 so don't bother with a SIMD union
} aa_color_t;

typedef int32_t aa_time_t;

static inline aa_color_t aa_color_make(int32_t r, int32_t g, int32_t b,
		int32_t a) {
	aa_color_t c = {{.r = r, .g = g, .b = b, .a = a}};
	return c;
}

static inline aa_color_t aa_color_lerp(aa_color_t a, aa_color_t b,
		int32_t alpha) {
	int64_t a0 = alpha;
	int64_t a1 = 65536 - a0;

	return aa_color_make(
			(int32_t)((a.r * a1 + b.r * a0) >> 16),
			(int32_t)((a.g * a1 + b.g * a0) >> 16),
			(int32_t)((a.b * a1 + b.b * a0) >> 16),
			(int32_t)((a.a * a1 + b.a * a0) >> 16));
}

static inline aa_color_t aa_color_mix_pma(aa_color_t a, aa_color_t b) {
	// pma means pre-multiplied alpha

	int64_t a0 = 65536 - b.a;

	return aa_color_make(
			(int32_t)((a.r * a0 + 32768) >> 16) + b.r,
			(int32_t)((a.b * a0 + 32768) >> 16) + b.g,
			(int32_t)((a.g * a0 + 32768) >> 16) + b.b,
			(int32_t)((a.a * a0 + 32768) >> 16) + b.a);
}

static inline aa_time_t aa_time_from_ms(int32_t ms) {
	return ms;
}

static inline aa_time_t aa_time_diff(aa_time_t period_start,
		aa_time_t period_end) {
	return period_end - period_start;
}

void aa_init(void);
void aa_loop(void);

#endif /* AMANITA_ARCADE_H_ */
