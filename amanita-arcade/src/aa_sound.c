/*
 * aa_sound.c
 *
 *  Created on: Jun 9, 2016
 *      Author: jlunder
 */

#include "aa_sound.h"

#include "aa_peripherals.h"
#include "hardware.h"

static size_t last_audio_pos, cur_audio_pos;

static size_t aa_audio_pos = 0;

static void aa_fill_i2s(void * buf, size_t buf_len);

void aa_sound_init(void) {
#ifdef AA_PRODUCTION_HARDWARE
	cs43l22_start(aa_fill_i2s);

	__sync_synchronize();
	last_audio_pos = aa_audio_pos;
#else
	aa_fill_i2s(NULL, 0);
#endif
}

void aa_sound_update(void) {
	cur_audio_pos = aa_audio_pos;
	last_audio_pos = cur_audio_pos;
}

void aa_sound_quiet(void) {

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

