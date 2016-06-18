/*
 * aa_lights.h
 *
 *  Created on: Jun 8, 2016
 *      Author: jlunder
 */

#ifndef AA_LIGHTS_H_
#define AA_LIGHTS_H_

#include "amanita_arcade.h"

#define AA_LIGHTS_STALK_HEIGHT 50
#define AA_LIGHTS_STALK_CIRC 3
#define AA_LIGHTS_MYCELIUM_BREADTH 3
#define AA_LIGHTS_MYCELIUM_DEPTH 1

// brightnesses -- cap, mycelium -- are FIX12

typedef enum {
	AALM_A,
	AALM_B,
	AALM_C,
	AALM_D,
} aa_lights_mushroom_t;

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
	aa_lights_pattern_t pattern;
} aa_lights_cycle_step_t;

typedef struct {
	size_t step_count;
	size_t loop_step;
	aa_lights_cycle_step_t const * steps;
} aa_lights_cycle_t;

typedef struct {
	aa_time_t mycelium_time;
	aa_time_t stalk_time;
	aa_time_t cap_time;
} aa_lights_pulse_t;

void aa_lights_init(void);
void aa_lights_update(void);

void aa_lights_bg_static(aa_lights_mushroom_t mushroom, aa_time_t transition,
		aa_lights_solid_t const * solid);
void aa_lights_bg_cycle(aa_lights_mushroom_t mushroom, aa_time_t transition,
		aa_lights_cycle_t const * cycle);
void aa_lights_fg_clear(aa_lights_mushroom_t mushroom, aa_time_t transition);
void aa_lights_fg_solid(aa_lights_mushroom_t mushroom, aa_time_t transition,
		aa_lights_solid_t const * solid);
void aa_lights_fg_pattern(aa_lights_mushroom_t mushroom,
		aa_time_t transition, aa_lights_pattern_t const * pattern);
void aa_lights_fg_cycle(aa_lights_mushroom_t mushroom,
		aa_time_t transition, aa_lights_cycle_t const * cycle);
void aa_lights_pulse_up(aa_lights_mushroom_t mushroom,
		aa_lights_pulse_t const * pulse);
void aa_lights_pulse_down(aa_lights_mushroom_t mushroom,
		aa_lights_pulse_t const * pulse);

#endif /* AA_LIGHTS_H_ */
