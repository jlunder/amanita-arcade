#include "synth_core.h"
#include "sample_generator.h"


void sample_generator_init(sample_generator_t * generator)
{
	memset(generator, 0, sizeof (sample_generator_t));
	
	generator->sample = NULL;
	generator->play_pos_increment = 0;
	generator->play_pos = 0;
}


void sample_generator_destroy(sample_generator_t * generator)
{
	memset(generator, 0, sizeof (sample_generator_t));
}


void sample_generator_seek(sample_generator_t * generator,
	int32_t frame)
{
	if(frame < 0) {
		fprintf(stderr, "seek to negative frame\n");
		frame = 0;
	}
	
	if(generator->sample->length < frame) {
		sample_generator_stop_playing(generator);
	} else {
		generator->play_pos =
			((int64_t)frame) * SYNTH_UTIL_FRACTIONAL_PLAY_RATE_1;
	}
}


void sample_generator_set_parameters(sample_generator_t * generator,
	synth_util_parameter_t volume, synth_util_parameter_t pan)
{
	generator->volume = volume;
	generator->pan = pan;
}


void sample_generator_start(sample_generator_t * generator,
	sample_t const * sample,
	synth_util_fractional_play_rate_t play_pos_increment)
{
	if(sample == NULL) {
		fprintf(stderr, "starting generator with NULL sample\n");
		sample_generator_stop_playing(generator);
		return;
	}
	
	generator->sample = sample;
	generator->play_pos = 0;
	generator->play_pos_increment = play_pos_increment;
	generator->looping = sample->loop_end > 0;
}


void sample_generator_note_off(sample_generator_t * generator)
{
	if(generator->sample == NULL) {
		return;
	}
	
	if(generator->sample->note_off_stop_looping) {
		sample_generator_stop_looping(generator);
	}
	if(generator->sample->note_off_stop_playing) {
		sample_generator_stop_playing(generator);
	}
}


void sample_generator_stop_looping(sample_generator_t * generator)
{
	generator->looping = false;
}


void sample_generator_stop_playing(sample_generator_t * generator)
{
	generator->sample = NULL;
	generator->play_pos_increment = 0;
	generator->looping = false;
}


bool sample_generator_get_playing(sample_generator_t * generator)
{
	return (generator->sample != NULL)
		&& (generator->play_pos_increment != 0);
}


void sample_generator_generate(sample_generator_t * generator,
	synth_util_sample_t * dest, size_t num_frames)
{
	// todo: make this faster
	
	if(!sample_generator_get_playing(generator)) {
		memset(dest, 0, sizeof (synth_util_sample_t)
			* SYNTH_UTIL_NUM_CHANNELS * num_frames);
	} else {
		sample_t const * sample = generator->sample;
		int16_t const * sample_data = (int16_t *)generator->sample->data;
		
		for(size_t j = 0; j < num_frames; ++j) {
			int32_t pos = (int32_t)(generator->play_pos
				/ SYNTH_UTIL_FRACTIONAL_PLAY_RATE_1)
				* SYNTH_UTIL_NUM_CHANNELS;
			
			// assumes sample and output format match!
			*(dest++) = (sample_data[pos] * generator->volume
				+ SYNTH_UTIL_PARAMETER_1 / 2) / SYNTH_UTIL_PARAMETER_1;
			*(dest++) = (sample_data[pos + 1] * generator->volume
				+ SYNTH_UTIL_PARAMETER_1 / 2) / SYNTH_UTIL_PARAMETER_1;
			
			generator->play_pos += generator->play_pos_increment;
			
			if((int32_t)(generator->play_pos
				/ SYNTH_UTIL_FRACTIONAL_PLAY_RATE_1)
				>= sample->loop_end)
			{
				if(generator->looping) {
					generator->play_pos =
						(int64_t)((((int32_t)(generator->play_pos
						/ SYNTH_UTIL_FRACTIONAL_PLAY_RATE_1)
						- sample->loop_start) % (sample->loop_end
						- sample->loop_start) + sample->loop_start)
						* SYNTH_UTIL_FRACTIONAL_PLAY_RATE_1)
						| (generator->play_pos
						& (SYNTH_UTIL_FRACTIONAL_PLAY_RATE_1 - 1));
				} else {
					if((int32_t)(generator->play_pos
						/ SYNTH_UTIL_FRACTIONAL_PLAY_RATE_1)
						>= sample->length)
					{
						sample_generator_stop_playing(generator);
						if((j + 1) < num_frames) {
							memset(dest, 0, sizeof (synth_util_sample_t)
								* SYNTH_UTIL_NUM_CHANNELS
								* (num_frames - (j + 1)));
							j = num_frames - 1;
						}
					}
				}
			}
		}
	}
}


