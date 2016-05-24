#ifndef SAMPLER_H
#define SAMPLER_H


#include "synth_core.h"


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


#define SAMPLER_MAX_VOICES (16)


struct sample_;
struct pwl_envelope_;


typedef struct sampler_split_ {
	struct pwl_envelope_ const * volume_envelope;
	struct pwl_envelope_ const * pitch_envelope;
	struct pwl_envelope_ const * pan_envelope;
	struct sample_ const * sample;
} sampler_split_t;


typedef struct sampler_note_setup_ {
	int64_t play_pos_increment;
	sampler_split_t const * split;
	int32_t pad;
} sampler_note_setup_t;


typedef struct sampler_program_ {
	sampler_note_setup_t notes[SYNTH_UTIL_MIDI_NUM_NOTES];
} sampler_program_t;


typedef struct sampler_voice_ {
	sample_generator_t sample_generator;
	pwl_generator_t volume_generator;
	pwl_generator_t pan_generator;
	pwl_generator_t pitch_generator;
	int8_t note;
	int8_t program;
} sampler_voice_t;


typedef struct sampler_ {
	sampler_program_t const * programs[SYNTH_UTIL_MIDI_NUM_PROGRAMS];
	sampler_voice_t voices[SAMPLER_MAX_VOICES];
	
	int8_t current_program;
} sampler_t;


extern void sampler_init(sampler_t * sampler);
extern void sampler_destroy(sampler_t * sampler);

extern void sampler_create_instrument(sampler_t * sampler,
	instrument_definition_t * instrument_defNinition);

extern void sampler_reset_all_voices(sampler_t * sampler);
extern void sampler_clear_all_programs(sampler_t * sampler);
extern void sampler_load_program(sampler_t * sampler, int8_t program,
	ssrm_resource_id_t resource_id);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif /* SAMPLER_H */


