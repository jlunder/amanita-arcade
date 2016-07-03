#include "aa_game.h"

#include "aa_input.h"
#include "aa_lights.h"
#include "aa_sound.h"

aa_lights_solid_t const aa_game_bg_simon = {
		.mycelium = AA_COLOR_ONE,
		.stalk = {{.r = AA_COLOR_ONE, .g = AA_COLOR_ONE, .b = AA_COLOR_ONE,
				.a = AA_COLOR_ONE,}},
		.cap = AA_COLOR_ONE,
};

void aa_game_simon_init(void) {
}

void aa_game_simon_loop(aa_time_t delta_time) {
	(void)delta_time;

	aa_sound_quiet();
	for(aa_lights_mushroom_t i = 0; i < AALM_COUNT; ++i) {
		aa_lights_solid(i, AALL_BG, aa_time_from_ms(0),
				&aa_game_bg_simon);
		aa_lights_clear(i, AALL_FG, aa_time_from_ms(0));
	}
}

void aa_game_simon_shutdown(void) {
}

