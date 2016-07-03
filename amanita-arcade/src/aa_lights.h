/*
 * aa_lights.h
 *
 *  Created on: Jun 8, 2016
 *      Author: jlunder
 */

#ifndef AA_LIGHTS_H_
#define AA_LIGHTS_H_

#include "amanita_arcade.h"

#define AA_LIGHTS_STALK_HEIGHT 8
#define AA_LIGHTS_STALK_CIRC 3

#define AA_LIGHTS_MYCELIUM_BREADTH 3
#define AA_LIGHTS_MYCELIUM_DEPTH 1

#define AA_LIGHTS_CYCLE_STEPS_MAX 16

#define AA_LIGHTS_COLOR_RED_INIT \
	{{.r = AA_FIX16_ONE, .g = 0, .b = 0, .a = AA_FIX16_ONE}}
#define AA_LIGHTS_COLOR_GREEN_INIT \
	{{.r = 0, .g = AA_FIX16_ONE, .b = 0, .a = AA_FIX16_ONE}}
#define AA_LIGHTS_COLOR_BLUE_INIT \
	{{.r = 0, .g = 0, .b = AA_FIX16_ONE, .a = AA_FIX16_ONE}}
#define AA_LIGHTS_COLOR_PINK_INIT \
	{{.r = AA_FIX16_ONE, .g = AA_FIX16_ONE / 2, .b = AA_FIX16_ONE / 2, \
		.a = AA_FIX16_ONE}}

// brightnesses -- cap, mycelium -- are FIX12

typedef enum {
	AALM_A,
	AALM_B,
	AALM_C,
	AALM_D,
	AALM_COUNT
} aa_lights_mushroom_t;

typedef enum {
	AALL_BG,
	AALL_FG,
	AALL_COUNT
} aa_lights_layer_t;

typedef struct {
	int32_t mycelium;
	aa_color_t stalk;
	int32_t cap;
} aa_lights_solid_t;

typedef struct {
	int32_t mycelium[AA_LIGHTS_MYCELIUM_DEPTH][AA_LIGHTS_MYCELIUM_BREADTH];
	aa_color_t stalk[AA_LIGHTS_STALK_CIRC][AA_LIGHTS_STALK_HEIGHT];
	int32_t cap;
} aa_lights_pattern_t;

typedef struct {
	aa_time_t transition;
	aa_lights_pattern_t const * pattern;
} aa_lights_cycle_step_t;

typedef struct {
	size_t step_count;
	size_t loop_step;
	aa_lights_cycle_step_t steps[AA_LIGHTS_CYCLE_STEPS_MAX];
} aa_lights_cycle_t;

typedef struct {
	aa_time_t mycelium_time;
	aa_time_t stalk_time;
	aa_time_t cap_time;
} aa_lights_pulse_t;

void aa_lights_init(void);
void aa_lights_update(aa_time_t delta_time);

void aa_lights_clear(aa_lights_mushroom_t mushroom, aa_lights_layer_t layer,
		aa_time_t transition);
void aa_lights_solid(aa_lights_mushroom_t mushroom, aa_lights_layer_t layer,
		aa_time_t transition, aa_lights_solid_t const * solid);
void aa_lights_pattern(aa_lights_mushroom_t mushroom,
		aa_lights_layer_t layer, aa_time_t transition,
		aa_lights_pattern_t const * pattern);
void aa_lights_cycle(aa_lights_mushroom_t mushroom, aa_lights_layer_t layer,
		aa_time_t transition, aa_lights_cycle_t const * cycle);
void aa_lights_pulse_up(aa_lights_mushroom_t mushroom,
		aa_lights_layer_t layer, aa_lights_pulse_t const * pulse);
void aa_lights_pulse_down(aa_lights_mushroom_t mushroom,
		aa_lights_layer_t layer, aa_lights_pulse_t const * pulse);

extern aa_lights_solid_t const aa_lights_solid_red;
extern aa_lights_solid_t const aa_lights_solid_green;
extern aa_lights_solid_t const aa_lights_solid_blue;
extern aa_lights_solid_t const aa_lights_solid_pink;
extern aa_lights_solid_t const aa_lights_solid_clear;

#endif /* AA_LIGHTS_H_ */
