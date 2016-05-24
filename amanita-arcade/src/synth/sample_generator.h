#ifndef SAMPLE_GENERATOR_H
#define SAMPLE_GENERATOR_H


#include "synth_core.h"


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


typedef struct sample_ {
	int32_t length;
	int32_t loop_start;
	int32_t loop_end;
	uint8_t note_off_stop_looping;
	uint8_t note_off_stop_playing;
	int16_t data[1];
} sample_t;


typedef struct sample_generator_ {
	sample_t const * sample;
	int64_t play_pos;
	synth_util_fractional_play_rate_t play_pos_increment;
	synth_util_parameter_t volume;
	synth_util_parameter_t pan;
	bool looping;
} sample_generator_t;


extern void sample_generator_init(sample_generator_t * generator);
extern void sample_generator_destroy(sample_generator_t * generator);

extern void sample_generator_seek(sample_generator_t * generator,
	int32_t frame);
extern void sample_generator_set_parameters(sample_generator_t * generator,
	synth_util_parameter_t volume, synth_util_parameter_t pan);
extern void sample_generator_start(sample_generator_t * generator,
	sample_t const * sample,
	synth_util_fractional_play_rate_t play_pos_increment);
extern void sample_generator_note_off(sample_generator_t * generator);
extern void sample_generator_stop_looping(sample_generator_t * generator);
extern void sample_generator_stop_playing(sample_generator_t * generator);
extern bool sample_generator_get_playing(sample_generator_t * generator);
extern void sample_generator_generate(sample_generator_t * generator,
	synth_util_sample_t * dest, size_t num_frames);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif // SAMPLE_GENERATOR_H


