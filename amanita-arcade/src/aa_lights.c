/*
 * aa_lights.c
 *
 *  Created on: Jun 9, 2016
 *      Author: jlunder
 */

#include "aa_lights.h"

#include "hardware.h"
#include "aa_lights_cie_table.h"
#include "aa_peripherals.h"

typedef struct {
	size_t current_step;
	int32_t time_since_step;

	int32_t transition_timeout;
	bool change_pending;

	uint8_t pad0[3];

	aa_lights_pattern_t last_pattern;

	aa_lights_cycle_t current_cycle;
	aa_lights_cycle_t pending_cycle;
} aa_lights_layer_state_t;

static uint8_t aa_cie16_to_fix8(int32_t a);
static uint8_t aa_fix16_to_fix8(int32_t a);
static void aa_lights_display(void);
static void aa_lights_generate(aa_time_t delta_time);
static void aa_lights_advance_state(aa_lights_layer_state_t * state,
		aa_time_t delta_time);

#define N {{0, 0, 0, 0}}
#define W {{65536, 65536, 65536, 65536}}

static const aa_lights_pattern_t aa_lights_pattern_pulse[10] = {
		{
				.cap = 0,
				.stalk = {
						{N, N, N, N, N, N, N, N,},
						{N, N, N, N, N, N, N, N,},
						{N, N, N, N, N, N, N, N,},
				},
				.mycelium = {
						{65536, 65536, 65536},
				},
		},
		{
				.cap = 0,
				.stalk = {
						{W, N, N, N, N, N, N, N,},
						{W, N, N, N, N, N, N, N,},
						{W, N, N, N, N, N, N, N,},
				},
				.mycelium = {
						{0, 0, 0},
				},
		},
		{
				.cap = 0,
				.stalk = {
						{N, W, N, N, N, N, N, N,},
						{N, W, N, N, N, N, N, N,},
						{N, W, N, N, N, N, N, N,},
				},
				.mycelium = {
						{0, 0, 0},
				},
		},
		{
				.cap = 0,
				.stalk = {
						{N, N, W, N, N, N, N, N,},
						{N, N, W, N, N, N, N, N,},
						{N, N, W, N, N, N, N, N,},
				},
				.mycelium = {
						{0, 0, 0},
				},
		},
		{
				.cap = 0,
				.stalk = {
						{N, N, N, W, N, N, N, N,},
						{N, N, N, W, N, N, N, N,},
						{N, N, N, W, N, N, N, N,},
				},
				.mycelium = {
						{0, 0, 0},
				},
		},
		{
				.cap = 0,
				.stalk = {
						{N, N, N, N, W, N, N, N,},
						{N, N, N, N, W, N, N, N,},
						{N, N, N, N, W, N, N, N,},
				},
				.mycelium = {
						{0, 0, 0},
				},
		},
		{
				.cap = 0,
				.stalk = {
						{N, N, N, N, N, W, N, N,},
						{N, N, N, N, N, W, N, N,},
						{N, N, N, N, N, W, N, N,},
				},
				.mycelium = {
						{0, 0, 0},
				},
		},
		{
				.cap = 0,
				.stalk = {
						{N, N, N, N, N, N, W, N,},
						{N, N, N, N, N, N, W, N,},
						{N, N, N, N, N, N, W, N,},
				},
				.mycelium = {
						{0, 0, 0},
				},
		},
		{
				.cap = 0,
				.stalk = {
						{N, N, N, N, N, N, N, W,},
						{N, N, N, N, N, N, N, W,},
						{N, N, N, N, N, N, N, W,},
				},
				.mycelium = {
						{0, 0, 0},
				},
		},
		{
				.cap = 65536,
				.stalk = {
						{N, N, N, N, N, N, N, N,},
						{N, N, N, N, N, N, N, N,},
						{N, N, N, N, N, N, N, N,},
				},
				.mycelium = {
						{0, 0, 0},
				},
		},
};

static const aa_lights_pattern_t aa_lights_pattern_neutral = {
		.cap = 0,
		.stalk = {
				{N, N, N, N, N, N, N, N,},
				{N, N, N, N, N, N, N, N,},
				{N, N, N, N, N, N, N, N,},
		},
		.mycelium = {
				{0, 0, 0},
		},
};

#undef N
#undef W

aa_lights_pattern_t aa_lights_static_values[AALL_COUNT][AALM_COUNT];

static aa_lights_layer_state_t aa_lights_state[AALM_COUNT][AALL_COUNT];

static aa_lights_pattern_t aa_lights_cur[AALM_COUNT];

#ifdef AA_PRODUCTION_HARDWARE
static uint8_t aa_lights_data_buf[AALM_COUNT][AA_LIGHTS_STALK_CIRC]
											  [AA_LIGHTS_STALK_HEIGHT][3];
#else
static uint8_t aa_lights_data_buf[AALM_COUNT][2][8][3];
#endif

void aa_lights_init(void) {
	memset(aa_lights_data_buf, 0, sizeof aa_lights_data_buf);
	memset(aa_lights_cur, 0, sizeof aa_lights_cur);
	memset(aa_lights_state, 0, sizeof aa_lights_state);
	(void)aa_fix16_to_fix8;
	(void)aa_lights_pattern_pulse;
	(void)aa_lights_pattern_neutral;
}

void aa_lights_update(aa_time_t delta_time) {
	aa_lights_display();
	aa_lights_generate(delta_time);
}

void aa_lights_clear(aa_lights_mushroom_t mushroom, aa_lights_layer_t layer,
		aa_time_t transition) {
	(void)mushroom;
	(void)layer;
	(void)transition;
}

void aa_lights_solid(aa_lights_mushroom_t mushroom, aa_lights_layer_t layer,
		aa_time_t transition, aa_lights_solid_t const * solid) {
	aa_lights_cycle_t cycle;

	aa_lights_static_values[mushroom][layer].cap = solid->cap;
	for(size_t i = 0; i < AA_LIGHTS_STALK_CIRC; ++i) {
		for(size_t j = 0; j < AA_LIGHTS_STALK_HEIGHT; ++j) {
			aa_lights_static_values[mushroom][layer].stalk[i][j] =
					solid->stalk;
		}
	}
	for(size_t i = 0; i < AA_LIGHTS_MYCELIUM_DEPTH; ++i) {
		for(size_t j = 0; j < AA_LIGHTS_MYCELIUM_DEPTH; ++j) {
			aa_lights_static_values[mushroom][layer].mycelium[i][j] =
					solid->mycelium;
		}
	}
	cycle.step_count = 1;
	cycle.loop_step = SIZE_MAX;
	cycle.steps[0].transition = transition;
	cycle.steps[0].pattern = &aa_lights_static_values[mushroom][layer];

	aa_lights_cycle(mushroom, layer, 0, &cycle);
}

void aa_lights_pulse_up(aa_lights_mushroom_t mushroom,
		aa_lights_layer_t layer, aa_lights_pulse_t const * pulse) {
	(void)mushroom;
	(void)layer;
	(void)pulse;
}

void aa_lights_pulse_down(aa_lights_mushroom_t mushroom,
		aa_lights_layer_t layer, aa_lights_pulse_t const * pulse) {
	(void)mushroom;
	(void)layer;
	(void)pulse;
}

void aa_lights_pattern(aa_lights_mushroom_t mushroom,
		aa_lights_layer_t layer, aa_time_t transition,
		aa_lights_pattern_t const * pattern) {
	(void)mushroom;
	(void)layer;
	(void)transition;
	(void)pattern;
}

void aa_lights_cycle(aa_lights_mushroom_t mushroom, aa_lights_layer_t layer,
		aa_time_t transition, aa_lights_cycle_t const * cycle) {
	aa_lights_layer_state_t * state = &aa_lights_state[mushroom][layer];
	memcpy(&state->pending_cycle, cycle, sizeof state->pending_cycle);
	if(!state->change_pending) {
		state->change_pending = true;
		state->transition_timeout = transition;
	} else {
		if(transition < state->transition_timeout) {
			state->transition_timeout = transition;
		}
	}
}

uint8_t aa_cie16_to_fix8(int32_t a) {
	if(a > 65535) {
		return 255;
	} else if(a < 0) {
		return 0;
	} else {
		return aa_cie_table[a >> 4];
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

void aa_lights_generate(aa_time_t delta_time) {
	for(aa_lights_mushroom_t m = 0; m < AALM_COUNT; ++m) {
		aa_lights_pattern_t * cur = &aa_lights_cur[m];
		memset(cur, 0, sizeof *cur);
		for(aa_lights_layer_t l = 0; l < AALL_COUNT; ++l) {
			aa_lights_layer_state_t * state = &aa_lights_state[m][l];
			aa_lights_advance_state(state, delta_time);
		}
		for(aa_lights_layer_t l = 0; l < AALL_COUNT; ++l) {
			aa_lights_layer_state_t * state = &aa_lights_state[m][l];
			aa_lights_cycle_step_t const * next_step =
					&state->current_cycle.steps[state->current_step];
			int64_t a0 = (int64_t)state->time_since_step * 65536 /
					next_step->transition;
			int64_t a1 = 65536 - a0;
			cur->cap += (int32_t)((a1 * state->last_pattern.cap +
					a0 * next_step->pattern->cap) >> 16);
			for(size_t i = 0; i < AA_LIGHTS_STALK_CIRC; ++i) {
				for(size_t j = 0; j < AA_LIGHTS_STALK_HEIGHT; ++j) {
					cur->stalk[i][j] = aa_color_lerp(
							state->last_pattern.stalk[i][j],
							next_step->pattern->stalk[i][j], (int32_t)a0);
				}
			}
			for(size_t i = 0; i < AA_LIGHTS_MYCELIUM_DEPTH; ++i) {
				for(size_t j = 0; j < AA_LIGHTS_MYCELIUM_BREADTH; ++j) {
					cur->mycelium[i][j] =
							(int32_t)((a1 *
									state->last_pattern.mycelium[i][j] +
									a0 * next_step->pattern->mycelium
									[i][j]) >> 16);
				}
			}
		}
	}
}

void aa_lights_advance_state(aa_lights_layer_state_t * state,
		aa_time_t delta_time) {
	if(state->current_cycle.step_count < SIZE_MAX) {
		aa_lights_cycle_step_t const * step =
				&state->current_cycle.steps[state->current_step];
		int32_t step_time = step->transition;

		state->time_since_step += delta_time;
		while(state->time_since_step >= step_time) {
			memcpy(&state->last_pattern, step->pattern,
					sizeof state->last_pattern);
			state->time_since_step -= step_time;
			++state->current_step;
			if(state->current_step >= state->current_cycle.step_count) {
				state->current_step = state->current_cycle.loop_step;
			}
		}
	}
}

void aa_lights_display(void) {

	if(!ws2811_get_outputting()) {
#ifdef AA_PRODUCTION_HARDWARE
		// color order is BRG for AA_PRODUCTION_HARDWARE
		for(size_t i = 0; i < AALM_COUNT; ++i) {
			for(size_t j = 0; j < AA_LIGHTS_STALK_CIRC; ++j) {
				for(size_t k = 0; k < AA_LIGHTS_STALK_HEIGHT; ++k) {
					aa_lights_data_buf[i][j][k][0] =
							aa_cie16_to_fix8(aa_lights_cur[i].stalk[j][k].b);
					aa_lights_data_buf[i][j][k][1] =
							aa_cie16_to_fix8(aa_lights_cur[i].stalk[j][k].r);
					aa_lights_data_buf[i][j][k][2] =
							aa_cie16_to_fix8(aa_lights_cur[i].stalk[j][k].g);
				}
			}
		}
#else // AA_TEST_HARDWARE
		// color order is GRB for AA_TEST_HARDWARE
		for(size_t i = 0; i < AALM_COUNT; ++i) {
			for(size_t k = 0; k < 8; ++k) {
				aa_lights_data_buf[i][0][k][0] =
						aa_cie16_to_fix8(
								(aa_lights_cur[i].stalk[0][k].g * 2 +
										aa_lights_cur[i].stalk[2][k].g) / 3);
				aa_lights_data_buf[i][0][k][1] =
						aa_cie16_to_fix8(
								(aa_lights_cur[i].stalk[0][k].r * 2 +
										aa_lights_cur[i].stalk[2][k].r) / 3);
				aa_lights_data_buf[i][0][k][2] =
						aa_cie16_to_fix8(
								(aa_lights_cur[i].stalk[0][k].b * 2 +
										aa_lights_cur[i].stalk[2][k].b) / 3);
			}
			for(size_t k = 0; k < 8; ++k) {
				aa_lights_data_buf[i][1][k][0] =
						aa_cie16_to_fix8(
								(aa_lights_cur[i].stalk[1][k].g * 2 +
										aa_lights_cur[i].stalk[2][k].g) / 3);
				aa_lights_data_buf[i][1][k][1] =
						aa_cie16_to_fix8(
								(aa_lights_cur[i].stalk[1][k].r * 2 +
										aa_lights_cur[i].stalk[2][k].r) / 3);
				aa_lights_data_buf[i][1][k][2] =
						aa_cie16_to_fix8(
								(aa_lights_cur[i].stalk[1][k].b * 2 +
										aa_lights_cur[i].stalk[2][k].b) / 3);
			}
		}
#endif
		ws2811_output_nb(aa_lights_data_buf, sizeof aa_lights_data_buf);
	}
}

