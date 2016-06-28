/*
 * aa_input.c
 *
 *  Created on: Jun 8, 2016
 *      Author: jlunder
 */

#include "aa_input.h"

#include "aa_peripherals.h"
#include "hardware.h"

#define AA_INPUT_THRESHOLD_PRESS 160
#define AA_INPUT_THRESHOLD_RELEASE 180

static bool aa_input_buttons[AAIB_COUNT];
static bool aa_input_buttons_last[AAIB_COUNT];

static hw_assignment_id_t aa_input_debug_leds[4];


void aa_input_init(void) {
	mpr121_auto_configure(4);

	memset(aa_input_buttons, 0, sizeof aa_input_buttons);
	memset(aa_input_buttons_last, 0, sizeof aa_input_buttons_last);

	aa_input_debug_leds[0] = hw_pin_assign(HWR_PD12);
	aa_input_debug_leds[1] = hw_pin_assign(HWR_PD13);
	aa_input_debug_leds[2] = hw_pin_assign(HWR_PD14);
	aa_input_debug_leds[3] = hw_pin_assign(HWR_PD15);

	for(size_t i = 0; i < 4; ++i) {
		hw_pin_configure(aa_input_debug_leds[i], HWPM_OUT_PP);
	}
}

void aa_input_read_buttons(void) {
#ifdef AA_TEST_HARDWARE
	uint16_t touch_analog[AAIB_COUNT / 2];
	mpr121_get_analog_values(0, touch_analog, AAIB_COUNT / 2);
#else
	uint16_t touch_analog[AAIB_COUNT];
	mpr121_get_analog_values(0, touch_analog, AAIB_COUNT);
#endif
	memcpy(aa_input_buttons_last, aa_input_buttons,
			sizeof aa_input_buttons_last);
	for(size_t i = 0; i < AAIB_COUNT; ++i) {
		if(!aa_input_buttons_last[i]) {
#ifdef AA_TEST_HARDWARE
			aa_input_buttons[i] = touch_analog[i / 2] <
					AA_INPUT_THRESHOLD_PRESS;
#else
			aa_input_buttons[i] = touch_analog[i] <
					AA_INPUT_THRESHOLD_PRESS;
#endif
		} else {
#ifdef AA_TEST_HARDWARE
			aa_input_buttons[i] = touch_analog[i / 2] <
					AA_INPUT_THRESHOLD_RELEASE;
#else
			aa_input_buttons[i] = touch_analog[i] <
					AA_INPUT_THRESHOLD_RELEASE;
#endif
		}
	}

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

