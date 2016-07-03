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

static void aa_game_mode_init(void);
static void aa_game_mode_loop(aa_time_t delta_time);
static void aa_game_mode_shutdown(void);

static void aa_game_debug_init(void);
static void aa_game_debug_loop(aa_time_t delta_time);
static void aa_game_debug_shutdown(void);

aa_lights_solid_t const aa_game_bg_error = {
		.mycelium = AA_COLOR_ONE,
		.stalk = {{.r = AA_COLOR_ONE, .g = AA_COLOR_ONE, .b = 0,
				.a = AA_COLOR_ONE,}},
		.cap = AA_COLOR_ONE,
};

aa_game_tune_t const * aa_game_tunes[AA_GAME_TUNE_COUNT] = {
};

#define AA_GAME_RESET_TIMEOUT aa_time_from_ms(30000)

static aa_game_mode_t aa_game_mode;
static aa_game_mode_t aa_game_mode_requested;

static aa_time_t aa_game_reset_time;

void aa_game_init(void) {
	aa_game_mode = AAGM_INVALID;
	aa_game_mode_requested = AAGM_ATTRACT;
}

void aa_game_loop(aa_time_t delta_time) {
	if(aa_game_reset_time <= 0) {
		aa_game_change_mode(AAGM_ATTRACT);
		aa_game_reset_time = 0;
	} else {
		aa_game_reset_time -= delta_time;
	}

	if(aa_game_mode_requested != aa_game_mode) {
		aa_game_mode_shutdown();
		aa_game_mode = aa_game_mode_requested;
		aa_game_mode_init();
	}

	aa_game_mode_loop(delta_time);
}

void aa_game_reset_timeout(void) {
	aa_game_reset_time = AA_GAME_RESET_TIMEOUT;
}

void aa_game_change_mode(aa_game_mode_t mode) {
	aa_game_mode_requested = mode;
}

void aa_game_mode_init(void) {
	switch(aa_game_mode) {
	case AAGM_DEBUG:
		aa_game_debug_init();
		break;
	case AAGM_ATTRACT:
		aa_game_attract_init();
		break;
	case AAGM_SIMON:
		aa_game_simon_init();
		break;
	case AAGM_HARMONIZE:
		aa_game_harmonize_init();
		break;
	case AAGM_PLAY_ALONG:
		aa_game_play_along_init();
		break;
	case AAGM_FREE_PLAY:
		aa_game_free_play_init();
		break;
	default:
		break;
	}
}

void aa_game_mode_loop(aa_time_t delta_time) {
	switch(aa_game_mode) {
	case AAGM_DEBUG:
		aa_game_debug_loop(delta_time);
		break;
	case AAGM_ATTRACT:
		aa_game_attract_loop(delta_time);
		break;
	case AAGM_SIMON:
		aa_game_simon_loop(delta_time);
		break;
	case AAGM_HARMONIZE:
		aa_game_harmonize_loop(delta_time);
		break;
	case AAGM_PLAY_ALONG:
		aa_game_play_along_loop(delta_time);
		break;
	case AAGM_FREE_PLAY:
		aa_game_free_play_loop(delta_time);
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

void aa_game_mode_shutdown(void) {
	switch(aa_game_mode) {
	case AAGM_DEBUG:
		aa_game_debug_shutdown();
		break;
	case AAGM_ATTRACT:
		aa_game_attract_shutdown();
		break;
	case AAGM_SIMON:
		aa_game_simon_shutdown();
		break;
	case AAGM_HARMONIZE:
		aa_game_harmonize_shutdown();
		break;
	case AAGM_PLAY_ALONG:
		aa_game_play_along_shutdown();
		break;
	case AAGM_FREE_PLAY:
		aa_game_free_play_shutdown();
		break;
	default:
		break;
	}
}

void aa_game_debug_init(void) {
}

void aa_game_debug_loop(aa_time_t delta_time) {
	(void)delta_time;
}

void aa_game_debug_shutdown(void) {
}

void aa_game_attract_init(void) {
}

void aa_game_attract_loop(aa_time_t delta_time) {
	(void)delta_time;

	aa_sound_quiet();
	for(aa_lights_mushroom_t i = 0; i < AALM_COUNT; ++i) {
		aa_lights_solid(i, AALL_BG, aa_time_from_ms(0),
				&aa_game_bg_error);
		aa_lights_clear(i, AALL_FG, aa_time_from_ms(0));
	}
}

void aa_game_attract_shutdown(void) {

}

void aa_game_harmonize_init(void) {
}

void aa_game_harmonize_loop(aa_time_t delta_time) {
	(void)delta_time;

	aa_sound_quiet();
	for(aa_lights_mushroom_t i = 0; i < AALM_COUNT; ++i) {
		aa_lights_solid(i, AALL_BG, aa_time_from_ms(0),
				&aa_game_bg_error);
		aa_lights_clear(i, AALL_FG, aa_time_from_ms(0));
	}
}

void aa_game_harmonize_shutdown(void) {
}

void aa_game_play_along_init(void) {
}

void aa_game_play_along_loop(aa_time_t delta_time) {
	(void)delta_time;

	aa_sound_quiet();
	for(aa_lights_mushroom_t i = 0; i < AALM_COUNT; ++i) {
		aa_lights_solid(i, AALL_BG, aa_time_from_ms(0),
				&aa_game_bg_error);
		aa_lights_clear(i, AALL_FG, aa_time_from_ms(0));
	}
}

void aa_game_play_along_shutdown(void) {
}

void aa_game_free_play_init(void) {
}

void aa_game_free_play_loop(aa_time_t delta_time) {
	(void)delta_time;

	aa_sound_quiet();
	for(aa_lights_mushroom_t i = 0; i < AALM_COUNT; ++i) {
		aa_lights_solid(i, AALL_BG, aa_time_from_ms(0),
				&aa_game_bg_error);
		aa_lights_clear(i, AALL_FG, aa_time_from_ms(0));
	}
}

void aa_game_free_play_shutdown(void) {
}

