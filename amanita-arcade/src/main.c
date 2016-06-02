#include "amanita_arcade.h"
#include "hardware.h"
#include "peripherals.h"

#define HC_NUM_LIGHTS 200

static size_t aa_audio_pos = 0;

static aa_color_t aa_lights[HC_NUM_LIGHTS];
static aa_color_t aa_lights_next[HC_NUM_LIGHTS];
static int32_t aa_powers[HC_NUM_LIGHTS];
static aa_pal_color_t aa_background;

static void aa_fill_i2s(void * buf, size_t buf_len);
static int32_t aa_compute_power(size_t sample_from, size_t sample_to);
static uint8_t aa_fix12_to_cie8(int32_t a);
static uint8_t aa_fix16_to_uint8(int32_t a);
static void aa_display_lights(void);
static void aa_generate_lights(int32_t power);

int main(void) {
	hw_assignment_id_t ld3, ld4, ld5, ld6;
	size_t last_audio_pos, cur_audio_pos;
	uint32_t last_tick;

	// Send a greeting to the trace device (skipped on Release).
	cu_log("Hello ARM World!\n");

	// At this stage the system clock should have already been configured
	// at high speed.
	cu_log("System clock: %u Hz\n", SystemCoreClock);

	ld3 = hw_pin_assign(HWR_PD12);
	ld4 = hw_pin_assign(HWR_PD13);
	ld5 = hw_pin_assign(HWR_PD14);
	ld6 = hw_pin_assign(HWR_PD15);

	hw_pin_configure(ld3, HWPM_OUT_PP);
	hw_pin_configure(ld4, HWPM_OUT_PP);
	hw_pin_configure(ld5, HWPM_OUT_PP);
	hw_pin_configure(ld6, HWPM_OUT_PP);

	per_init();

	mpr121_auto_configure();

	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, 1);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, 1);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, 1);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, 1);

	//cs43l22_start(aa_fill_i2s);

	__sync_synchronize();
	last_audio_pos = aa_audio_pos;
	last_tick = HAL_GetTick();

	bool touch_state[4] = {false, false, false, false};
	for( ;; ) {
		uint32_t cur_tick = HAL_GetTick();
		int32_t power;
		uint16_t touch_analog[4];

		if((uint32_t)(cur_tick - last_tick) < 5) {
			continue;
		}
		last_tick = cur_tick;

		//mpr121_get_analog_baselines(0, touch_analog, 4);
		//cu_log("%d %d %d %d\n", touch_analog[0], touch_analog[1], touch_analog[2], touch_analog[3]);
		//aa_display_lights();
		mpr121_get_analog_values(0, touch_analog, 4);

		for(size_t i = 0; i < 4; ++i) {
			if(touch_state[i]) {
				if(touch_analog[i] > 180) {
					touch_state[i] = false;
				}
			} else {
				if(touch_analog[i] < 160) {
					touch_state[i] = true;
				}
			}
		}

		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, touch_state[0]);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, touch_state[1]);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, touch_state[2]);
		HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, touch_state[3]);

		__sync_synchronize();

		cur_audio_pos = aa_audio_pos;
		power = aa_compute_power(last_audio_pos, cur_audio_pos);
		last_audio_pos = cur_audio_pos;

		(void)aa_fill_i2s;
		(void)aa_display_lights;
		(void)aa_generate_lights;
		(void)power;
		//aa_generate_lights(power);
	}
}

void aa_fill_i2s(void * buf, size_t buf_len) {
	uint16_t * sample_buf = (uint16_t *)buf;
	size_t frames = buf_len / (sizeof (uint16_t) * 2);
	size_t audio_pos_temp = aa_audio_pos;

	for(size_t i = 0; i < frames; ++i) {
		//uint16_t s = (uint16_t)hd_heartbeat_data[audio_pos_temp];
		uint16_t s = 0;
		*sample_buf++ = s;
		*sample_buf++ = s;
		++audio_pos_temp;
		//if(audio_pos_temp >= cu_lengthof(hd_heartbeat_data)) {
		if(audio_pos_temp >= 10000) {
			audio_pos_temp = 0;
		}
	}
	aa_audio_pos = audio_pos_temp;
	__sync_synchronize();
}

int32_t aa_compute_power(size_t sample_from, size_t sample_to) {
	int32_t maxs = 0;

	//cu_verify(sample_from < cu_lengthof(hd_heartbeat_data));
	//cu_verify(sample_to < cu_lengthof(hd_heartbeat_data));

	for(size_t i = sample_from; i != sample_to; ) {
		//int32_t as = abs((int32_t)hd_heartbeat_data[i]);
		int32_t as = 0;
		if(as > maxs) {
			maxs = as;
		}
		++i;
		//if(i >= cu_lengthof(hd_heartbeat_data)) {
		if(i >= 10000) {
			i = 0;
		}
	}
	return maxs;
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

void cu_abort(void) {
#if defined(DEBUG)
	__DEBUG_BKPT();
#endif
	for( ;; ) {
	}
}

void cu_log(char const * format, ...) {
	static char buf[1000];

	int written;
	va_list ap;

	va_start(ap, format);

	written = vsnprintf(buf, sizeof(buf), format, ap);
	if (written > 0) {
		trace_write(buf, (size_t) written);
	}

	va_end(ap);
}

