/*
 * amanita_arcade.c
 *
 *  Created on: Oct 17, 2015
 *      Author: jlunder
 */

#include "amanita_arcade.h"

#include "aa_game.h"
#include "aa_input.h"
#include "aa_lights.h"
#include "aa_peripherals.h"
#include "aa_sound.h"

static uint32_t aa_last_tick;

void aa_init(void) {
	aa_peripherals_init();
	aa_input_init();
	aa_lights_init();
	aa_sound_init();

	aa_last_tick = HAL_GetTick();
}

void aa_loop(void) {
	uint32_t cur_tick = HAL_GetTick();
	aa_time_t delta_time = aa_time_from_ms(
			(int32_t)(cur_tick - aa_last_tick));

	if(delta_time < aa_time_from_ms(5)) {
		return;
	}
	aa_last_tick = cur_tick;

	aa_input_read_buttons();
	aa_game_loop(delta_time);
	while((HAL_GetTick() - aa_last_tick) < 2) {
		// wait for 1ms to pass after frame start
		// this improves timing consistency for animations
	}
	aa_lights_update(delta_time);
	aa_sound_update();
}
