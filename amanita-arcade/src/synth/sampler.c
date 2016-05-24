#include "synth_core.h"
#include "sampler.h"


void sampler_process_message(void * context,
	synth_util_message_t const * message);
void sampler_process_note_on(sampler_t * sampler,
	synth_util_midi_note_t note, synth_util_parameter_t velocity);
void sampler_process_note_off(sampler_t * sampler,
	synth_util_midi_note_t note, synth_util_parameter_t velocity);
void sampler_process_aftertouch(sampler_t * sampler,
	synth_util_midi_note_t note, synth_util_parameter_t velocity);
void sampler_process_control_change(sampler_t * sampler,
	int8_t control, synth_util_parameter_t parameter);
void sampler_process_program_change(sampler_t * sampler, int8_t program);
void sampler_generate(void * context, synth_util_sample_t * dest,
	size_t num_frames);
synth_util_timespan_t sampler_time_to_next_internal_event(void * context);
void sampler_clear_program(sampler_t * sampler, int8_t program);
void sampler_reset_voice(sampler_t * sampler, size_t voice);
synth_util_parameter_t sampler_velocity_to_volume(sampler_t * sampler,
	int8_t velocity);


void sampler_init(sampler_t * sampler)
{
	memset(sampler, 0, sizeof *sampler);
	sampler_clear_all_programs(sampler);
	for(size_t i = 0; i < SAMPLER_MAX_VOICES; ++i) {
		sample_generator_init(&sampler->voices[i].sample_generator);
	}
}


void sampler_destroy(sampler_t * sampler)
{
	for(size_t i = 0; i < SAMPLER_MAX_VOICES; ++i) {
		sample_generator_destroy(&sampler->voices[i].sample_generator);
	}
	memset(sampler, 0, sizeof *sampler);
}


void sampler_create_instrument(sampler_t * sampler,
	instrument_definition_t * instrument_definition)
{
	instrument_definition->context = sampler;
	instrument_definition->process_message = &sampler_process_message;
	instrument_definition->generate = &sampler_generate;
	instrument_definition->time_to_next_internal_event =
		&sampler_time_to_next_internal_event;
}


void sampler_process_message(void * context,
	synth_util_message_t const * message)
{
	sampler_t * sampler = (sampler_t *)context;
	
	switch(message->command) {
	default:
	case SYNTH_UTIL_COMMAND_NULL:
		break;
	case SYNTH_UTIL_COMMAND_NOTE_ON:
		sampler_process_note_on(sampler, message->data.note_on.note,
			message->data.note_on.velocity);
		break;
	case SYNTH_UTIL_COMMAND_NOTE_OFF:
		sampler_process_note_off(sampler, message->data.note_off.note,
			message->data.note_off.velocity);
		break;
	case SYNTH_UTIL_COMMAND_AFTERTOUCH:
		sampler_process_aftertouch(sampler, message->data.aftertouch.note,
			message->data.aftertouch.velocity);
		break;
	case SYNTH_UTIL_COMMAND_CONTROL_CHANGE:
		sampler_process_control_change(sampler,
			message->data.control_change.control,
			message->data.control_change.value);
		break;
	case SYNTH_UTIL_COMMAND_PROGRAM_CHANGE:
		sampler_process_program_change(sampler,
			message->data.program_change.program);
		break;
	}
}


void sampler_process_note_on(sampler_t * sampler,
	synth_util_midi_note_t note, synth_util_parameter_t velocity)
{
	sampler_voice_t * voice = NULL;
	sampler_note_setup_t const * note_assignment = NULL;
	
	for(size_t i = 0; i < SAMPLER_MAX_VOICES; ++i) {
		if(!sample_generator_get_playing(&sampler->voices[i].sample_generator)) {
			voice = &sampler->voices[i];
			break;
		}
	}
	if(voice == NULL) {
		// all voices still busy
		return;
	}
	
	voice->note = note;
	voice->program = sampler->current_program;
	sample_generator_set_parameters(&voice->sample_generator,
		sampler_velocity_to_volume(sampler, velocity),
		SYNTH_UTIL_PARAMETER_1 / 2);
	note_assignment = &sampler->programs[voice->program]->notes[note];
	sample_generator_start(&voice->sample_generator,
		note_assignment->split->sample, note_assignment->play_pos_increment);
}


void sampler_process_note_off(sampler_t * sampler,
	synth_util_midi_note_t note, synth_util_parameter_t velocity)
{
	(void)velocity;
	
	for(size_t i = 0; i < SAMPLER_MAX_VOICES; ++i) {
		if(sampler->voices[i].note == note) {
			sample_generator_note_off(&sampler->voices[i].sample_generator);
		}
	}
}


void sampler_process_aftertouch(sampler_t * sampler,
	synth_util_midi_note_t note, synth_util_parameter_t velocity)
{
	for(size_t i = 0; i < SAMPLER_MAX_VOICES; ++i) {
		if(sampler->voices[i].note == note) {
			sample_generator_set_parameters(
				&sampler->voices[i].sample_generator,
				sampler_velocity_to_volume(sampler, velocity),
				SYNTH_UTIL_PARAMETER_1 / 2);
		}
	}
}


void sampler_process_control_change(sampler_t * sampler,
	int8_t control, synth_util_parameter_t parameter)
{
	(void)sampler;
	(void)control;
	(void)parameter;
	// do nothing
}


void sampler_process_program_change(sampler_t * sampler, int8_t program)
{
	sampler->current_program = program;
}


void sampler_generate(void * context, synth_util_sample_t * dest,
	size_t num_frames)
{
	sampler_t * sampler = (sampler_t *)context;
	intptr_t mark = region_allocator_mark(&synth_util_temp_allocator);
	size_t dest_size = sizeof (synth_util_sample_t) * SYNTH_UTIL_NUM_CHANNELS
		* num_frames;
	synth_util_sample_t * buf =
		region_allocator_allocate(&synth_util_temp_allocator, dest_size);
	
	memset(dest, 0, dest_size);
	
	for(size_t j = 0; j < SAMPLER_MAX_VOICES; ++j) {
		if(!sample_generator_get_playing(
			&sampler->voices[j].sample_generator))
		{
			continue;
		}
		
		sample_generator_generate(&sampler->voices[j].sample_generator, buf,
			num_frames);
		for(size_t i = 0; i < num_frames * SYNTH_UTIL_NUM_CHANNELS; ++i) {
			dest[i] += buf[i];
		}
	}
	
	region_allocator_release(&synth_util_temp_allocator, mark);
}


synth_util_timespan_t sampler_time_to_next_internal_event(void * context)
{
	(void)context;
	
	return SYNTH_UTIL_TIMESPAN_MAX;
}


void sampler_reset_all_voices(sampler_t * sampler)
{
	for(size_t i = 0; i < SAMPLER_MAX_VOICES; ++i) {
		sampler_reset_voice(sampler, i);
	}
}


void sampler_clear_all_programs(sampler_t * sampler)
{
	for(size_t i = 0; i < SYNTH_UTIL_MIDI_NUM_PROGRAMS; ++i) {
		sampler_clear_program(sampler, i);
	}
}


void sampler_load_program(sampler_t * sampler, int8_t program,
	ssrm_resource_id_t resource_id)
{
	if(sampler->programs[program] != NULL) {
		sampler_clear_program(sampler, program);
	}
	sampler->programs[program] =
		(sampler_program_t const *)ssrm_lock_resource(resource_id);
}


void sampler_clear_program(sampler_t * sampler, int8_t program)
{
	for(size_t i = 0; i < SAMPLER_MAX_VOICES; ++i) {
		if(sampler->voices[i].program == program) {
			sampler_reset_voice(sampler, i);
		}
	}
	
	sampler->programs[program] = NULL;
}


void sampler_reset_voice(sampler_t * sampler, size_t voice)
{
	sample_generator_stop_playing(&sampler->voices[voice].sample_generator);
	sampler->voices[voice].note = -1;
	sampler->voices[voice].program = -1;
}


synth_util_parameter_t sampler_velocity_to_volume(sampler_t * sampler,
	int8_t velocity)
{
	(void)sampler;
	return synth_util_velocity_map[velocity] / 4;
}


