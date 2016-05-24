#include "mixer.h"


int16_t mixer_inputbuffers[MIXER_NUMINPUTS][
	MIXER_BUFFERLENGTH * MIXER_NUMCHANNELS];


void mixer_init(void)
{
	memset(mixer_inputbuffers, 0, sizeof mixer_inputbuffers);
}


void mixer_shutdown(void)
{
}


int16_t * mixer_getinputbuffer(int input)
{
	return mixer_inputbuffers[input];
}


void mixer_process(int16_t * outputbuffer)
{
	int32_t accumulator[MIXER_BUFFERLENGTH * MIXER_NUMCHANNELS];
	
	memset(accumulator, 0,
		sizeof (int32_t) * MIXER_BUFFERLENGTH * MIXER_NUMCHANNELS);
	
	for(size_t i = 0; i < MIXER_NUMINPUTS; ++i) {
		for(size_t j = 0; j < MIXER_BUFFERLENGTH * MIXER_NUMCHANNELS; ++j) {
			accumulator[j] += mixer_inputbuffers[i][j];
		}
	}
	for(size_t j = 0; j < MIXER_BUFFERLENGTH * MIXER_NUMCHANNELS; ++j) {
		outputbuffer[j] = accumulator[j] / MIXER_NUMINPUTS;
	}
}


