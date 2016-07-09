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

	int32_t change_timeout;
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
static void aa_lights_advance_state(aa_lights_mushroom_t mushroom,
		aa_lights_layer_t layer, aa_time_t delta_time);
static void aa_lights_advance_state_change(aa_lights_layer_state_t * state,
		aa_lights_pattern_t const * last_pattern);

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

aa_lights_solid_t const aa_lights_solid_red = {
		.cap = AA_FIX16_ONE,
		.stalk = AA_LIGHTS_COLOR_PINK_INIT,
		.mycelium = AA_FIX16_ONE,
};

aa_lights_solid_t const aa_lights_solid_green = {
		.cap = AA_FIX16_ONE,
		.stalk = AA_LIGHTS_COLOR_GREEN_INIT,
		.mycelium = AA_FIX16_ONE,
};

aa_lights_solid_t const aa_lights_solid_blue = {
		.cap = AA_FIX16_ONE,
		.stalk = AA_LIGHTS_COLOR_BLUE_INIT,
		.mycelium = AA_FIX16_ONE,
};

aa_lights_solid_t const aa_lights_solid_pink = {
		.cap = AA_FIX16_ONE,
		.stalk = AA_LIGHTS_COLOR_PINK_INIT,
		.mycelium = AA_FIX16_ONE,
};

aa_lights_solid_t const aa_lights_solid_clear = {
		.cap = 0,
		.stalk = {{.r = 0, .g = 0, .b = 0, .a = 0}},
		.mycelium = 0,
};

aa_lights_solid_t const * aa_lights_default_bg_solid[AALM_COUNT] = {
		&aa_lights_solid_red,
		&aa_lights_solid_green,
		&aa_lights_solid_blue,
		&aa_lights_solid_pink,
};

aa_lights_pattern_t const aa_lights_default_bg_patterns[AALM_COUNT][3] = {
		{
				{
						.mycelium = {{0, 0, 0,}},
						.stalk = {
								{
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
								},
								{
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
								},
								{
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
								},
						},
						.cap = 0,
				},
				{
						.mycelium = {{0, 0, 0,}},
						.stalk = {
								{
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
								},
								{
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
								},
								{
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
								},
						},
						.cap = 0,
				},
				{
						.mycelium = {{0, 0, 0,}},
						.stalk = {
								{
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
								},
								{
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
								},
								{
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
										AA_LIGHTS_COLOR_RED_INIT,
										AA_LIGHTS_COLOR_RED_75_INIT,
										AA_LIGHTS_COLOR_RED_50_INIT,
								},
						},
						.cap = 0,
				},
		},
		{
				{
						.mycelium = {{0, 0, 0,}},
						.stalk = {
								{
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
								},
								{
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
								},
								{
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
								},
						},
						.cap = 0,
				},
				{
						.mycelium = {{0, 0, 0,}},
						.stalk = {
								{
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
								},
								{
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
								},
								{
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
								},
						},
						.cap = 0,
				},
				{
						.mycelium = {{0, 0, 0,}},
						.stalk = {
								{
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
								},
								{
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
								},
								{
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
										AA_LIGHTS_COLOR_GREEN_INIT,
										AA_LIGHTS_COLOR_GREEN_75_INIT,
										AA_LIGHTS_COLOR_GREEN_50_INIT,
								},
						},
						.cap = 0,
				},
		},
		{
				{
						.mycelium = {{0, 0, 0,}},
						.stalk = {
								{
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
								},
								{
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
								},
								{
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
								},
						},
						.cap = 0,
				},
				{
						.mycelium = {{0, 0, 0,}},
						.stalk = {
								{
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
								},
								{
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
								},
								{
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
								},
						},
						.cap = 0,
				},
				{
						.mycelium = {{0, 0, 0,}},
						.stalk = {
								{
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
								},
								{
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
								},
								{
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
										AA_LIGHTS_COLOR_BLUE_INIT,
										AA_LIGHTS_COLOR_BLUE_75_INIT,
										AA_LIGHTS_COLOR_BLUE_50_INIT,
								},
						},
						.cap = 0,
				},
		},
		{
				{
						.mycelium = {{0, 0, 0,}},
						.stalk = {
								{
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
								},
								{
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
								},
								{
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
								},
						},
						.cap = 0,
				},
				{
						.mycelium = {{0, 0, 0,}},
						.stalk = {
								{
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
								},
								{
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
								},
								{
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
								},
						},
						.cap = 0,
				},
				{
						.mycelium = {{0, 0, 0,}},
						.stalk = {
								{
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
								},
								{
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
								},
								{
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
										AA_LIGHTS_COLOR_PINK_INIT,
										AA_LIGHTS_COLOR_PINK_75_INIT,
										AA_LIGHTS_COLOR_PINK_50_INIT,
								},
						},
						.cap = 0,
				},
		},
};

aa_lights_cycle_t const aa_lights_default_bg_cycles[AALM_COUNT] = {
		{
				.step_count = 3,
				.loop_step = 0,
				.steps = {
						{
								.transition = AA_TIME_FROM_MILLIS_INIT(150),
								.pattern = &aa_lights_default_bg_patterns[0][0],
						},
						{
								.transition = AA_TIME_FROM_MILLIS_INIT(150),
								.pattern = &aa_lights_default_bg_patterns[0][1],
						},
						{
								.transition = AA_TIME_FROM_MILLIS_INIT(150),
								.pattern = &aa_lights_default_bg_patterns[0][2],
						},
				},
		},
		{
				.step_count = 3,
				.loop_step = 0,
				.steps = {
						{
								.transition = AA_TIME_FROM_MILLIS_INIT(150),
								.pattern = &aa_lights_default_bg_patterns[1][0],
						},
						{
								.transition = AA_TIME_FROM_MILLIS_INIT(150),
								.pattern = &aa_lights_default_bg_patterns[1][1],
						},
						{
								.transition = AA_TIME_FROM_MILLIS_INIT(150),
								.pattern = &aa_lights_default_bg_patterns[1][2],
						},
				},
		},
		{
				.step_count = 3,
				.loop_step = 0,
				.steps = {
						{
								.transition = AA_TIME_FROM_MILLIS_INIT(150),
								.pattern = &aa_lights_default_bg_patterns[2][0],
						},
						{
								.transition = AA_TIME_FROM_MILLIS_INIT(150),
								.pattern = &aa_lights_default_bg_patterns[2][1],
						},
						{
								.transition = AA_TIME_FROM_MILLIS_INIT(150),
								.pattern = &aa_lights_default_bg_patterns[2][2],
						},
				},
		},
		{
				.step_count = 3,
				.loop_step = 0,
				.steps = {
						{
								.transition = AA_TIME_FROM_MILLIS_INIT(150),
								.pattern = &aa_lights_default_bg_patterns[3][0],
						},
						{
								.transition = AA_TIME_FROM_MILLIS_INIT(150),
								.pattern = &aa_lights_default_bg_patterns[3][1],
						},
						{
								.transition = AA_TIME_FROM_MILLIS_INIT(150),
								.pattern = &aa_lights_default_bg_patterns[3][2],
						},
				},
		},
};


#undef N
#undef W

static aa_lights_pattern_t aa_lights_static_values[AALM_COUNT][AALL_COUNT];

static aa_lights_layer_state_t aa_lights_state[AALM_COUNT][AALL_COUNT];

static aa_lights_pattern_t aa_lights_cur[AALM_COUNT][AALL_COUNT];

static hw_assignment_id_t aa_lights_cap_pins[AALM_COUNT];

#ifdef AA_PRODUCTION_HARDWARE
#ifdef AA_WS2811_I2S
static uint8_t aa_lights_data_buf[AALM_COUNT][AA_LIGHTS_STALK_CIRC]
											  [AA_LIGHTS_STALK_HEIGHT][3];
#endif
#ifdef AA_WS2811_BITBANG
static uint8_t aa_lights_data_buf[AA_LIGHTS_STALK_CIRC]
								  [AA_LIGHTS_STALK_HEIGHT][3][8];
#endif
#else
#ifdef AA_WS2811_I2S
static uint8_t aa_lights_data_buf[AALM_COUNT][2][8][3];
#endif
#ifdef AA_WS2811_BITBANG
static uint8_t aa_lights_data_buf[AALM_COUNT][2][8][3][8];
#endif
#endif

void aa_lights_init(void) {
	memset(aa_lights_data_buf, 0, sizeof aa_lights_data_buf);
	memset(aa_lights_cur, 0, sizeof aa_lights_cur);
	memset(aa_lights_state, 0, sizeof aa_lights_state);

	aa_lights_solid(AALM_A, AALL_BG, 0, &aa_lights_solid_red);
	aa_lights_solid(AALM_A, AALL_FG, 0, &aa_lights_solid_red);
	aa_lights_solid(AALM_B, AALL_BG, 0, &aa_lights_solid_green);
	aa_lights_solid(AALM_B, AALL_FG, 0, &aa_lights_solid_green);
	aa_lights_solid(AALM_C, AALL_BG, 0, &aa_lights_solid_blue);
	aa_lights_solid(AALM_C, AALL_FG, 0, &aa_lights_solid_blue);
	aa_lights_solid(AALM_D, AALL_BG, 0, &aa_lights_solid_pink);
	aa_lights_solid(AALM_D, AALL_FG, 0, &aa_lights_solid_pink);

	ws2811_start();

	aa_lights_cap_pins[0] = hw_pin_assign(HWR_PE4);
	aa_lights_cap_pins[1] = hw_pin_assign(HWR_PE5);
	aa_lights_cap_pins[2] = hw_pin_assign(HWR_PE6);
	aa_lights_cap_pins[3] = hw_pin_assign(HWR_PE7);

	for(size_t i = 0; i < AALM_COUNT; ++i) {
		hw_pin_configure(aa_lights_cap_pins[i], HWPM_OUT_PP | HWPM_SPEED_MIN);
	}

	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_7, GPIO_PIN_RESET);

	(void)aa_fix16_to_fix8;
	(void)aa_lights_pattern_pulse;
	(void)aa_lights_pattern_neutral;
}

void aa_lights_update(aa_time_t delta_time) {
#if 0
	for(size_t i = 0; i < AALM_COUNT; ++i) {
		for(size_t l = 0; l < AALL_COUNT; ++l) {
			for(size_t j = 0; j < AA_LIGHTS_STALK_CIRC; ++j) {
				for(size_t k = 0; k < AA_LIGHTS_STALK_HEIGHT; ++k) {
					aa_lights_cur[i][l].stalk[j][k] = aa_color_make(
							(((int32_t)i + 1) << 14) / AALM_COUNT,
							(((int32_t)j + 1) << 14) / AA_LIGHTS_STALK_CIRC,
							(((int32_t)k + 1) << 14) /
							AA_LIGHTS_STALK_HEIGHT,
							AA_FIX16_ONE);
				}
			}
		}
	}
#endif
	aa_lights_display();
	aa_lights_generate(delta_time);
}

void aa_lights_clear(aa_lights_mushroom_t mushroom, aa_lights_layer_t layer,
		aa_time_t transition) {
	aa_lights_solid(mushroom, layer, transition, &aa_lights_solid_clear);
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
		state->change_timeout = transition;
	} else {
		if(transition < state->change_timeout) {
			state->change_timeout = transition;
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
		for(aa_lights_layer_t l = 0; l < AALL_COUNT; ++l) {
			aa_lights_advance_state(m, l, delta_time);
		}
		for(aa_lights_layer_t l = 0; l < AALL_COUNT; ++l) {
			aa_lights_pattern_t * layer = &aa_lights_cur[m][l];
			aa_lights_layer_state_t * state = &aa_lights_state[m][l];
			aa_lights_cycle_step_t const * next_step =
					&state->current_cycle.steps[state->current_step];
			int64_t a0;
			int64_t a1;

			if(next_step->transition != 0) {
				a0 = (int64_t)state->time_since_step * 65536 /
						next_step->transition;
			} else {
				a0 = 65536;
			}
			a1 = 65536 - a0;
			layer->cap = (int32_t)((a1 * state->last_pattern.cap +
					a0 * next_step->pattern->cap) >> 16);
			for(size_t i = 0; i < AA_LIGHTS_STALK_CIRC; ++i) {
				for(size_t j = 0; j < AA_LIGHTS_STALK_HEIGHT; ++j) {
					layer->stalk[i][j] = aa_color_lerp(
							state->last_pattern.stalk[i][j],
							next_step->pattern->stalk[i][j], (int32_t)a0);
				}
			}
			for(size_t i = 0; i < AA_LIGHTS_MYCELIUM_DEPTH; ++i) {
				for(size_t j = 0; j < AA_LIGHTS_MYCELIUM_BREADTH; ++j) {
					layer->mycelium[i][j] =
							(int32_t)((a1 *
									state->last_pattern.mycelium[i][j] +
									a0 * next_step->pattern->mycelium
									[i][j]) >> 16);
				}
			}
		}
	}
}

void aa_lights_advance_state(aa_lights_mushroom_t mushroom,
		aa_lights_layer_t layer, aa_time_t delta_time) {
	aa_lights_layer_state_t * state = &aa_lights_state[mushroom][layer];

	state->time_since_step += delta_time;

	if((state->change_timeout < state->time_since_step) &&
			state->change_pending) {
		aa_lights_advance_state_change(state,
				&aa_lights_cur[mushroom][layer]);
		state->time_since_step = 0;
	}

	for(;;) {
		aa_lights_cycle_step_t const * step =
				&state->current_cycle.steps[state->current_step];

		if(state->time_since_step >= step->transition) {
			memcpy(&state->last_pattern, step->pattern,
					sizeof state->last_pattern);
			state->time_since_step -= step->transition;
			if(state->current_step >= state->current_cycle.step_count - 1) {
				if(state->change_pending) {
					aa_lights_advance_state_change(state, step->pattern);
				} else if(state->current_cycle.loop_step != SIZE_MAX) {
					state->current_step = state->current_cycle.loop_step;
				} else {
					// remain stuck at end of current step
					state->time_since_step = step->transition;
					break;
				}
			} else {
				++state->current_step;
			}
		} else {
			break;
		}
	}
}

void aa_lights_advance_state_change(aa_lights_layer_state_t * state,
		aa_lights_pattern_t const * last_pattern) {
	memcpy(&state->last_pattern, last_pattern,
			sizeof state->last_pattern);
	memcpy(&state->current_cycle, &state->pending_cycle,
			sizeof state->current_cycle);
	state->current_step = 0;
	state->change_pending = false;
	state->change_timeout = 0;
}

void aa_lights_display(void) {
	aa_lights_pattern_t accum[AALM_COUNT];
	for(aa_lights_mushroom_t m = 0; m < AALM_COUNT; ++m) {
		memset(&accum[m], 0, sizeof accum[m]);
		for(aa_lights_layer_t l = 0; l < AALL_COUNT; ++l) {
			aa_lights_pattern_t * layer = &aa_lights_cur[m][l];
			accum[m].cap += layer->cap;
			for(size_t i = 0; i < AA_LIGHTS_STALK_CIRC; ++i) {
				for(size_t j = 0; j < AA_LIGHTS_STALK_HEIGHT; ++j) {
					accum[m].stalk[i][j] = aa_color_mix_pma(
							accum[m].stalk[i][j], layer->stalk[i][j]);
				}
			}
			for(size_t i = 0; i < AA_LIGHTS_MYCELIUM_DEPTH; ++i) {
				for(size_t j = 0; j < AA_LIGHTS_MYCELIUM_BREADTH; ++j) {
					accum[m].mycelium[i][j] += layer->mycelium[i][j];
				}
			}
		}
	}

#ifdef AA_WS2811_I2S
	if(ws2811_get_outputting()) {
		return;
	}
#endif
	memset(aa_lights_data_buf, 0, sizeof aa_lights_data_buf);
#if defined(AA_WS2811_I2S) && defined(AA_PRODUCTION_HARDWARE)
	// color order is BRG for AA_PRODUCTION_HARDWARE
	for(size_t i = 0; i < AALM_COUNT; ++i) {
		for(size_t j = 0; j < AA_LIGHTS_STALK_CIRC; ++j) {
			for(size_t k = 0; k < AA_LIGHTS_STALK_HEIGHT; ++k) {
				aa_lights_data_buf[i][j][k][0] =
						aa_cie16_to_fix8(accum[i].stalk[j][k].b);
				aa_lights_data_buf[i][j][k][1] =
						aa_cie16_to_fix8(accum[i].stalk[j][k].r);
				aa_lights_data_buf[i][j][k][2] =
						aa_cie16_to_fix8(accum[i].stalk[j][k].g);
			}
		}
	}
#elif defined(AA_WS2811_I2S) && !defined(AA_PRODUCTION_HARDWARE) // AA_TEST_HARDWARE
	// color order is GRB for AA_TEST_HARDWARE
	for(size_t i = 0; i < AALM_COUNT; ++i) {
		for(size_t k = 0; k < 8; ++k) {
			size_t l = k * AA_LIGHTS_STALK_HEIGHT / 8;
			aa_lights_data_buf[i][0][k][0] =
					aa_cie16_to_fix8(
							(accum[i].stalk[0][l].g * 2 +
									accum[i].stalk[2][l].g) / 3);
			aa_lights_data_buf[i][0][k][1] =
					aa_cie16_to_fix8(
							(accum[i].stalk[0][l].r * 2 +
									accum[i].stalk[2][l].r) / 3);
			aa_lights_data_buf[i][0][k][2] =
					aa_cie16_to_fix8(
							(accum[i].stalk[0][l].b * 2 +
									accum[i].stalk[2][l].b) / 3);
		}
		for(size_t k = 0; k < 8; ++k) {
			size_t l = k * AA_LIGHTS_STALK_HEIGHT / 8;
			aa_lights_data_buf[i][1][7 - k][0] =
					aa_cie16_to_fix8(
							(accum[i].stalk[1][l].g * 2 +
									accum[i].stalk[2][l].g) / 3);
			aa_lights_data_buf[i][1][7 - k][1] =
					aa_cie16_to_fix8(
							(accum[i].stalk[1][l].r * 2 +
									accum[i].stalk[2][l].r) / 3);
			aa_lights_data_buf[i][1][7 - k][2] =
					aa_cie16_to_fix8(
							(accum[i].stalk[1][l].b * 2 +
									accum[i].stalk[2][l].b) / 3);
		}
	}
#elif defined(AA_WS2811_BITBANG) && defined(AA_PRODUCTION_HARDWARE)
	// color order is BRG for AA_PRODUCTION_HARDWARE
	for(size_t j = 0; j < AA_LIGHTS_STALK_CIRC; ++j) {
		for(size_t k = 0; k < AA_LIGHTS_STALK_HEIGHT; ++k) {
			for(size_t i = 0; i < AALM_COUNT; ++i) {
				aa_lights_data_buf[j][k][0][i] =
						aa_cie16_to_fix8(accum[i].stalk[j][k].b);
			}
			for(size_t i = 0; i < AALM_COUNT; ++i) {
				aa_lights_data_buf[j][k][1][i] =
						aa_cie16_to_fix8(accum[i].stalk[j][k].r);
			}
			for(size_t i = 0; i < AALM_COUNT; ++i) {
				aa_lights_data_buf[j][k][2][i] =
						aa_cie16_to_fix8(accum[i].stalk[j][k].g);
			}
		}
	}
#elif defined(AA_WS2811_BITBANG) && !defined(AA_PRODUCTION_HARDWARE) // AA_TEST_HARDWARE
	// color order is GRB for AA_TEST_HARDWARE
	for(size_t k = 0; k < 8; ++k) {
		size_t l = k * AA_LIGHTS_STALK_HEIGHT / 8;
		for(size_t i = 0; i < AALM_COUNT; ++i) {
			aa_lights_data_buf[i][0][k][0][0] =
					aa_cie16_to_fix8(
							(accum[i].stalk[0][l].g * 2 +
									accum[i].stalk[2][l].g) / 3);
		}
		for(size_t i = 0; i < AALM_COUNT; ++i) {
			aa_lights_data_buf[i][0][k][1][0] =
					aa_cie16_to_fix8(
							(accum[i].stalk[0][l].r * 2 +
									accum[i].stalk[2][l].r) / 3);
		}
		for(size_t i = 0; i < AALM_COUNT; ++i) {
			aa_lights_data_buf[i][0][k][2][0] =
					aa_cie16_to_fix8(
							(accum[i].stalk[0][l].b * 2 +
									accum[i].stalk[2][l].b) / 3);
		}
	}
	for(size_t k = 0; k < 8; ++k) {
		size_t l = k * AA_LIGHTS_STALK_HEIGHT / 8;
		for(size_t i = 0; i < AALM_COUNT; ++i) {
			aa_lights_data_buf[i][1][7 - k][0][0] =
					aa_cie16_to_fix8(
							(accum[i].stalk[1][l].g * 2 +
									accum[i].stalk[2][l].g) / 3);
		}
		for(size_t i = 0; i < AALM_COUNT; ++i) {
			aa_lights_data_buf[i][1][7 - k][1][0] =
					aa_cie16_to_fix8(
							(accum[i].stalk[1][l].r * 2 +
									accum[i].stalk[2][l].r) / 3);
		}
		for(size_t i = 0; i < AALM_COUNT; ++i) {
			aa_lights_data_buf[i][1][7 - k][2][0] =
					aa_cie16_to_fix8(
							(accum[i].stalk[1][l].b * 2 +
									accum[i].stalk[2][l].b) / 3);
		}
	}
#endif
#ifdef AA_WS2811_BITBANG
	ws2811_output(aa_lights_data_buf, sizeof aa_lights_data_buf);
#endif
#ifdef AA_WS2811_I2S
	ws2811_output_nb(aa_lights_data_buf, sizeof aa_lights_data_buf);
#endif
}

