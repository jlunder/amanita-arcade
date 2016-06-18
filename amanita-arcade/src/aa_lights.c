/*
 * aa_lights.c
 *
 *  Created on: Jun 9, 2016
 *      Author: jlunder
 */

#include "aa_lights.h"

#include "hardware.h"
#include "peripherals.h"

#include "aa_lights_cie_table.h"

static uint8_t aa_cie12_to_fix8(int32_t a);
static uint8_t aa_fix16_to_fix8(int32_t a);
static void aa_display_lights(void);
static void aa_generate_lights(int32_t power);

void aa_lights_init(void) {
	(void)aa_cie12_to_fix8;
	(void)aa_fix16_to_fix8;
}

void aa_lights_update(void) {
	(void)aa_display_lights;
	(void)aa_generate_lights;

	if(!ws2811_get_outputting()) {
		// color order is BRG
		static uint8_t buf[] = {
				0x00, 0xFF, 0x00,
				0x00, 0x00, 0xFF,
				0xFF, 0x00, 0x00,
				0x00, 0xFF, 0x60,
				0x00, 0xFF, 0x60,
				0x0F, 0x0F, 0x0F,
				0x00, 0x0F, 0x00,
				0x00, 0x00, 0x0F,
				0x0F, 0x00, 0x00,
		};
		ws2811_output_nb(buf, sizeof buf);
	}
}

void aa_lights_bg_static(aa_lights_mushroom_t mushroom, aa_time_t transition,
		aa_lights_solid_t const * solid) {
	(void)mushroom;
	(void)transition;
	(void)solid;
}

void aa_lights_bg_cycle(aa_lights_mushroom_t mushroom, aa_time_t transition,
		aa_lights_cycle_t const * cycle) {
	(void)mushroom;
	(void)transition;
	(void)cycle;
}

void aa_lights_fg_clear(aa_lights_mushroom_t mushroom,
		aa_time_t transition) {
	(void)mushroom;
	(void)transition;
}

void aa_lights_fg_solid(aa_lights_mushroom_t mushroom, aa_time_t transition,
		aa_lights_solid_t const * solid) {
	(void)mushroom;
	(void)transition;
	(void)solid;
}

void aa_lights_fg_pattern(aa_lights_mushroom_t mushroom,
		aa_time_t transition, aa_lights_pattern_t const * pattern) {
	(void)mushroom;
	(void)transition;
	(void)pattern;
}

void aa_lights_fg_cycle(aa_lights_mushroom_t mushroom,
		aa_time_t transition, aa_lights_cycle_t const * cycle) {
	(void)mushroom;
	(void)transition;
	(void)cycle;
}

void aa_lights_pulse_up(aa_lights_mushroom_t mushroom,
		aa_lights_pulse_t const * pulse) {
	(void)mushroom;
	(void)pulse;
}

void aa_lights_pulse_down(aa_lights_mushroom_t mushroom,
		aa_lights_pulse_t const * pulse) {
	(void)mushroom;
	(void)pulse;
}

uint8_t aa_cie12_to_fix8(int32_t a) {
	if(a > 4095) {
		return 255;
	} else if(a < 0) {
		return 0;
	} else {
		return aa_cie_table[a];
	}
}

uint8_t aa_fix16_to_fix8(int32_t a) {
	if(a > 65535) {
		return 255;
	} else if(a < 0) {
		return 0;
	} else {
		return (uint8_t)(a >> 8);
	}
}

void aa_display_lights(void) {
}

void aa_generate_lights(int32_t power) {
	(void)power;
}

