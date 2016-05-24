#include "synth_core.h"
#include "instrument.h"


#define INSTRUMENT_MANAGER_REFERENCE_NULL INSTRUMENT_MANAGER_MAX_DEFINITIONS


instrument_definition_t instrument_definitions[SYNTH_UTIL_MIDI_NUM_CHANNELS];

synth_util_parameter_t instrument_manager_mix_level = SYNTH_UTIL_PARAMETER_1;

sync_signal_t instrument_manager_playing;
sync_signal_t instrument_manager_stopping;


void instrument_manager_dispatch_messages(void);
synth_util_timespan_t instrument_manager_time_to_next_disjoint(void);
synth_util_timespan_t instrument_manager_time_to_next_internal_event_all(
	void);
void instrument_manager_generate_all(synth_util_sample_t * dest,
	size_t num_frames);
bool instrument_definition_empty(instrument_definition_t const * definition);


void instrument_manager_init(void)
{
	memset(instrument_definitions, 0, sizeof instrument_definitions);
	instrument_manager_mix_level = SYNTH_UTIL_PARAMETER_1;
	
	sync_signal_init(&instrument_manager_playing, false);
	sync_signal_init(&instrument_manager_stopping, false);
}


void instrument_manager_shutdown(void)
{
	if(instrument_manager_get_playing()) {
		// this is a violation of contract, but if the output render has
		// already been stopped, it's actually okay. if it hasn't, the render
		// thread will continue accessing our statics which are being
		// destroyed below
		fprintf(stderr, "still playing during shutdown\n");
	}
	
	// belt and suspenders: if the render thread is still running, hopefully
	// this will tell it to do nothing and therefore prevent it from
	// crashing. on most architectures sync_signal_destroy() is a no-op and
	// the signal will be there, set, telling it to do nothing. (of course
	// this is implementation-dependent behaviour that we should not rely on
	// in any normal code paths)
	sync_signal_set(&instrument_manager_stopping);
	
	sync_signal_destroy(&instrument_manager_playing);
	sync_signal_destroy(&instrument_manager_stopping);
	
	memset(instrument_definitions, 0, sizeof instrument_definitions);
}


void instrument_manager_start_playback(void)
{
	sync_signal_reset(&instrument_manager_stopping);
}


void instrument_manager_stop_playback(void)
{
	sync_signal_set(&instrument_manager_stopping);
}


// stop_playback is asynchronous, so it is necessary to wait until
// get_playing returns false before actually manipulating the playback system
bool instrument_manager_get_playing(void)
{
	return sync_signal_check(&instrument_manager_playing);
}


void instrument_manager_add_instrument(int8_t midi_channel,
	instrument_definition_t * definition)
{
	if(instrument_definition_empty(definition)) {
		fprintf(stderr, "adding empty instrument\n");
		return;
	}
	if((midi_channel < 0) || (midi_channel >= SYNTH_UTIL_MIDI_NUM_CHANNELS)) {
		fprintf(stderr, "adding instrument to invalid midi channel\n");
		return;
	}
	if(!instrument_definition_empty(&instrument_definitions[midi_channel])) {
		fprintf(stderr, "overwriting existing instrument in midi channel\n");
		return;
	}
	
	instrument_definitions[midi_channel] = *definition;
}


void instrument_manager_remove_instrument(int8_t midi_channel)
{
	if((midi_channel < 0) || (midi_channel >= SYNTH_UTIL_MIDI_NUM_CHANNELS)) {
		fprintf(stderr, "removing instrument from invalid midi channel\n");
		return;
	}
	if(instrument_definition_empty(&instrument_definitions[midi_channel])) {
		fprintf(stderr, "removing null instrument reference\n");
		return;
	}
	
	memset(&instrument_definitions[midi_channel], 0,
		sizeof (instrument_definition_t));
}


void instrument_manager_set_mix_level(synth_util_parameter_t mix_level)
{
	instrument_manager_mix_level = mix_level;
}


void instrument_manager_render(synth_util_sample_t * dest, size_t num_frames)
{
	size_t frames_generated = 0;
	
	if(sync_signal_check(&instrument_manager_stopping)) {
		sync_signal_reset(&instrument_manager_playing);
	} else {
		sync_signal_set(&instrument_manager_playing);
	}
	
	while(frames_generated < num_frames) {
		size_t frames_left = num_frames - frames_generated;
		size_t frames_to_generate = 0;
		synth_util_timespan_t time_to_next_disjoint =
			SYNTH_UTIL_TIMESPAN_MAX;
		
		instrument_manager_dispatch_messages();
		time_to_next_disjoint = instrument_manager_time_to_next_disjoint();
		
		if((time_to_next_disjoint <= 0)
			|| ((size_t)time_to_next_disjoint > frames_left))
		{
			frames_to_generate = frames_left;
		} else {
			frames_to_generate = (size_t)time_to_next_disjoint;
		}
		
		instrument_manager_generate_all(
			dest + frames_generated * SYNTH_UTIL_NUM_CHANNELS,
			frames_to_generate);
		midi_parser_advance_time(frames_to_generate);
		frames_generated += frames_to_generate;
	}
}


void instrument_manager_dispatch_messages(void)
{
	while(midi_parser_get_time_to_next_message() == 0) {
		synth_util_message_t message = midi_parser_read_next_message();
		
		if(instrument_definitions[message.channel].process_message != NULL)
		{
			(*instrument_definitions[message.channel].process_message)(
				instrument_definitions[message.channel].context, &message);
		}
	}
}


synth_util_timespan_t instrument_manager_time_to_next_disjoint(void)
{
	synth_util_timespan_t result = SYNTH_UTIL_TIMESPAN_MAX;
	synth_util_timespan_t time_to_next_message;
	synth_util_timespan_t time_to_next_internal_event;
	
	time_to_next_message = midi_parser_get_time_to_next_message();
	time_to_next_internal_event =
		instrument_manager_time_to_next_internal_event_all();
	if(time_to_next_internal_event > 0) {
		result = time_to_next_internal_event;
	}
	if(time_to_next_message < result) {
		result = time_to_next_message;
	}
	if(result <= 0) {
		fprintf(stderr, "insane time to next disjoint\n");
	}
	
	return result;
}


synth_util_timespan_t instrument_manager_time_to_next_internal_event_all(
	void)
{
	synth_util_timespan_t min_time = SYNTH_UTIL_TIMESPAN_MAX;
	
	for(int8_t i = 0; i < SYNTH_UTIL_MIDI_NUM_CHANNELS; ++i) {
		if(instrument_definitions[i].time_to_next_internal_event != NULL) {
			synth_util_timespan_t this_time =
				(*instrument_definitions[i].time_to_next_internal_event)(
				instrument_definitions[i].context);
			if(this_time < 0) {
				fprintf(stderr, "insane time to next internal event\n");
				continue;
			}
			if(this_time < min_time) {
				min_time = this_time;
			}
		}
	}
	
	return min_time;
}


void instrument_manager_generate_all(synth_util_sample_t * dest,
	size_t num_frames)
{
	intptr_t mark = region_allocator_mark(&synth_util_temp_allocator);
	size_t num_samples = num_frames * SYNTH_UTIL_NUM_CHANNELS;
	synth_util_sample_t * instrument_buf =
		(synth_util_sample_t *)region_allocator_allocate(
		&synth_util_temp_allocator,
		sizeof (synth_util_sample_t) * num_samples);
	int32_t * accum_buf = (int32_t *)region_allocator_allocate(
		&synth_util_temp_allocator, sizeof (int32_t) * num_samples);
	
	memset(accum_buf, 0, sizeof (int32_t) * num_samples);
	
	for(size_t j = 0; j < SYNTH_UTIL_MIDI_NUM_CHANNELS; ++j) {
		if(instrument_definitions[j].generate != NULL) {
			(*instrument_definitions[j].generate)(
				instrument_definitions[j].context, instrument_buf,
				num_frames);
			for(size_t i = 0; i < num_samples; ++i) {
				accum_buf[i] += instrument_buf[i];
			}
		}
	}
	for(size_t i = 0; i < num_samples; ++i) {
		dest[i] = (accum_buf[i] * instrument_manager_mix_level)
			/ SYNTH_UTIL_PARAMETER_1;
	}
	region_allocator_release(&synth_util_temp_allocator, mark);
}


bool instrument_definition_empty(instrument_definition_t const * definition)
{
	return definition->generate == NULL;
}


