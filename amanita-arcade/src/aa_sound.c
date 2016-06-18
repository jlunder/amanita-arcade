/*
 * aa_sound.c
 *
 *  Created on: Jun 9, 2016
 *      Author: jlunder
 */

#include "aa_sound.h"

#include "hardware.h"
#include "peripherals.h"

static size_t last_audio_pos, cur_audio_pos;

static size_t aa_audio_pos = 0;

static void aa_fill_i2s(void * buf, size_t buf_len);
static int32_t aa_compute_power(size_t sample_from, size_t sample_to);

void aa_sound_init(void) {
	cs43l22_start(aa_fill_i2s);

	__sync_synchronize();
	last_audio_pos = aa_audio_pos;
}

void aa_sound_update(void) {
	int32_t power;

	cur_audio_pos = aa_audio_pos;
	power = aa_compute_power(last_audio_pos, cur_audio_pos);
	last_audio_pos = cur_audio_pos;

	(void)aa_fill_i2s;
	(void)power;
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
