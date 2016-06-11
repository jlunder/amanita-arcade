/*
 * aa_lights.c
 *
 *  Created on: Jun 9, 2016
 *      Author: jlunder
 */

#include "aa_lights.h"

#include "hardware.h"
#include "peripherals.h"

#define HC_NUM_LIGHTS 200

static aa_color_t aa_lights[HC_NUM_LIGHTS];
static aa_color_t aa_lights_next[HC_NUM_LIGHTS];
static int32_t aa_powers[HC_NUM_LIGHTS];
static aa_pal_color_t aa_background;

static uint8_t aa_fix12_to_cie8(int32_t a);
static uint8_t aa_fix16_to_uint8(int32_t a);
static void aa_display_lights(void);
static void aa_generate_lights(int32_t power);

void aa_lights_init(void) {

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

uint8_t aa_fix12_to_cie8(int32_t a) {
	if(a > 4095) {
		return 255;
	} else if(a < 0) {
		return 0;
	} else {
		return aa_cie_table[a];
	}
}

uint8_t aa_fix16_to_uint8(int32_t a) {
	if(a > 65535) {
		return 255;
	} else if(a < 0) {
		return 0;
	} else {
		return (uint8_t)(a >> 8);
	}
}

void aa_display_lights(void) {
	memcpy(&aa_lights, &aa_lights_next, sizeof aa_lights);
	ws2801_output(&aa_lights, sizeof aa_lights);
}

void aa_generate_lights(int32_t power) {
	int32_t aa_powers_next[HC_NUM_LIGHTS];
	int r = rand();

	aa_powers[r % 5] += power * 6;

	for(size_t i = 0; i < HC_NUM_LIGHTS; ++i) {
		int32_t p = aa_powers[i] * 10;
		if(i > 0) {
			p += aa_powers[i - 1] * 22;
		}
		p = p / 32;
		aa_powers_next[i] = p;
	}
	memcpy(aa_powers, aa_powers_next, sizeof aa_powers);
	aa_background.r += (int32_t)((r & 1) >> 0);
	aa_background.r -= (int32_t)((r & 2) >> 1);
	aa_background.g += (int32_t)((r & 4) >> 2);
	aa_background.g -= (int32_t)((r & 8) >> 3);
	aa_background.b += (int32_t)((r & 16) >> 4);
	aa_background.b -= (int32_t)((r & 32) >> 5);
	if(aa_background.r > 1023) {
		aa_background.r = 1023;
	}
	if(aa_background.g > 1023) {
		aa_background.g = 1023;
	}
	if(aa_background.b > 1023) {
		aa_background.b = 1023;
	}
	if(aa_background.r < 0) {
		aa_background.r = 0;
	}
	if(aa_background.g < 0) {
		aa_background.g = 0;
	}
	if(aa_background.b < 0) {
		aa_background.b = 0;
	}

	for(size_t i = 0; i < HC_NUM_LIGHTS; ++i) {
		int32_t p = aa_powers[i];
		int32_t cr = aa_orange_pink_pal[aa_fix16_to_uint8(p)].r;
		int32_t cg = aa_orange_pink_pal[aa_fix16_to_uint8(p)].g;
		int32_t cb = aa_orange_pink_pal[aa_fix16_to_uint8(p)].b;
		aa_color_t c;

		c.r = aa_fix12_to_cie8(cr + aa_background.r);
		c.g = aa_fix12_to_cie8(cg + aa_background.g);
		c.b = aa_fix12_to_cie8(cb + aa_background.b);

		aa_lights_next[i] = c;
	}
}

