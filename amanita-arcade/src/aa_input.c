/*
 * aa_input.c
 *
 *  Created on: Jun 8, 2016
 *      Author: jlunder
 */

#include "aa_input.h"

#include "aa_peripherals.h"
#include "hardware.h"

#if 1
#define AA_INPUT_SWITCHES
#else
#define AA_INPUT_MPR121
#endif

#define AA_INPUT_THRESHOLD_PRESS 160
#define AA_INPUT_THRESHOLD_RELEASE 180

static bool aa_input_buttons[AAIB_COUNT];
static bool aa_input_buttons_last[AAIB_COUNT];

#ifdef AA_INPUT_SWITCHES
static hw_assignment_id_t aa_input_switches[4];
#endif

static hw_assignment_id_t aa_input_debug_leds[4];


void aa_input_init(void) {
	//mpr121_auto_configure(4);

	memset(aa_input_buttons, 0, sizeof aa_input_buttons);
	memset(aa_input_buttons_last, 0, sizeof aa_input_buttons_last);

	aa_input_debug_leds[0] = hw_pin_assign(HWR_PD12);
	aa_input_debug_leds[1] = hw_pin_assign(HWR_PD13);
	aa_input_debug_leds[2] = hw_pin_assign(HWR_PD14);
	aa_input_debug_leds[3] = hw_pin_assign(HWR_PD15);

	aa_input_switches[0] = hw_pin_assign(HWR_PE0);
	aa_input_switches[1] = hw_pin_assign(HWR_PE1);
	aa_input_switches[2] = hw_pin_assign(HWR_PE2);
	aa_input_switches[3] = hw_pin_assign(HWR_PE3);

	for(size_t i = 0; i < 4; ++i) {
		hw_pin_configure(aa_input_switches[i], HWPM_IN_PU);
	}

	for(size_t i = 0; i < 4; ++i) {
		hw_pin_configure(aa_input_debug_leds[i], HWPM_OUT_PP);
	}
}

void aa_input_read_buttons(void) {
#ifdef AA_TEST_HARDWARE
	uint16_t touch_analog[4];
	memcpy(aa_input_buttons_last, aa_input_buttons,
			sizeof aa_input_buttons_last);
	//mpr121_get_analog_values(0, touch_analog, 4);
	memset(touch_analog, 0, sizeof touch_analog);
	for(size_t i = 0; i < AAIB_COUNT; ++i) {
		if(!aa_input_buttons_last[i]) {
			aa_input_buttons[i] = touch_analog[i / 2] <
					AA_INPUT_THRESHOLD_PRESS;
		} else {
			aa_input_buttons[i] = touch_analog[i / 2] <
					AA_INPUT_THRESHOLD_RELEASE;
		}
	}
#else
	//uint16_t touch_analog[AAIB_COUNT];
	memcpy(aa_input_buttons_last, aa_input_buttons,
			sizeof aa_input_buttons_last);
	//mpr121_get_analog_values(0, touch_analog, AAIB_COUNT);
	aa_input_buttons[0] =
			(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_0) != GPIO_PIN_RESET);
	aa_input_buttons[1] =
			(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_1) != GPIO_PIN_RESET);
	aa_input_buttons[2] =
			(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2) != GPIO_PIN_RESET);
	aa_input_buttons[3] =
			(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3) != GPIO_PIN_RESET);
#endif

	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12,
			aa_input_buttons[AAIB_A0] || aa_input_buttons[AAIB_A1]);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13,
			aa_input_buttons[AAIB_B0] || aa_input_buttons[AAIB_B1]);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14,
			aa_input_buttons[AAIB_C0] || aa_input_buttons[AAIB_C1]);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15,
			aa_input_buttons[AAIB_D0] || aa_input_buttons[AAIB_D1]);
}

bool aa_input_button_state(aa_input_button_id_t button) {
	return aa_input_buttons[button];
}

bool aa_input_button_press(aa_input_button_id_t button) {
	return aa_input_buttons[button] && !aa_input_buttons_last[button];
}

