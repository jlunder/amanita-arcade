/*
 * aa_game.c
 *
 *  Created on: Jun 11, 2016
 *      Author: jlunder
 */

#include "aa_game.h"
#include "aa_input.h"
#include "aa_lights.h"
#include "aa_sound.h"

#include "aa_game_palettes.h"

aa_lights_solid_t const aa_game_bg_error = {
		.mycelium = AA_COLOR_ONE,
		.stalk = {{.r = AA_COLOR_ONE, .g = 0, .b = AA_COLOR_ONE,
				.a = AA_COLOR_ONE,}},
		.cap = AA_COLOR_ONE,
};

aa_game_tune_t const * aa_game_tunes[AA_GAME_TUNE_COUNT] = {

};

static aa_game_mode_t aa_game_mode;
static aa_game_mode_t aa_game_mode_requested;

void aa_game_init(void) {
	aa_game_mode = AAGM_INVALID;
	aa_game_mode_requested = AAGM_ATTRACT;
}

void aa_game_loop(aa_time_t delta_time) {
	(void)delta_time;
	switch(aa_game_mode) {
	case AAGM_DEBUG:
		break;
	case AAGM_ATTRACT:
		aa_sound_quiet();
		for(aa_lights_mushroom_t i = AALM_A; i <= AALM_D; ++i) {
			aa_lights_solid(i, AALL_BG, aa_time_from_ms(0),
					&aa_game_bg_error);
			aa_lights_clear(i, AALL_FG, aa_time_from_ms(0));
		}
		break;
	case AAGM_SIMON:
		break;
	case AAGM_HARMONIZE:
		break;
	case AAGM_PLAY_ALONG:
		break;
	case AAGM_FREE_PLAY:
		break;
	default:
		aa_sound_quiet();
		for(aa_lights_mushroom_t i = AALM_A; i <= AALM_D; ++i) {
			aa_lights_solid(i, AALL_BG, aa_time_from_ms(0),
					&aa_game_bg_error);
			aa_lights_clear(i, AALL_FG, aa_time_from_ms(0));
		}
		break;
	}
}

void aa_game_reset_timeout(void) {

}

void aa_game_change_mode(aa_game_mode_t mode) {
	(void)mode;
}

void aa_game_attract_init(void) {

}

void aa_game_attract_loop(void) {

}

void aa_game_attract_shutdown(void) {

}

