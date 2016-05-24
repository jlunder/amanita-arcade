#include "midi_parser.h"


#define MIDI_PARSER_MESSAGE_QUEUE_LENGTH (128)


synth_util_timestamp_t midi_parser_cur_timestamp;
synth_util_timestamp_t midi_parser_last_message_timestamp;

int8_t midi_parser_pending_control_msbs[
	SYNTH_UTIL_MIDI_NUM_CHANNELS][
	SYNTH_UTIL_MIDI_NUM_FINE_CONTROLS];

size_t midi_parser_message_queue_head;
size_t midi_parser_message_queue_tail;
synth_util_message_t midi_parser_message_queue[
	MIDI_PARSER_MESSAGE_QUEUE_LENGTH];


size_t midi_parser_parse_midi_note_on(synth_util_timestamp_t timestamp,
	uint8_t const * data, size_t size);
size_t midi_parser_parse_midi_note_off(synth_util_timestamp_t timestamp,
	uint8_t const * data, size_t size);
size_t midi_parser_parse_midi_aftertouch(synth_util_timestamp_t timestamp,
	uint8_t const * data, size_t size);
size_t midi_parser_parse_midi_control_change(synth_util_timestamp_t timestamp,
	uint8_t const * data, size_t size);
size_t midi_parser_parse_midi_program_change(synth_util_timestamp_t timestamp,
	uint8_t const * data, size_t size);
bool midi_parser_check_midi_params(uint8_t const * data, size_t size,
	size_t paramsize);
void midi_parser_enqueue_message(synth_util_message_t message);


void midi_parser_init(void)
{
	midi_parser_cur_timestamp = 0;
	midi_parser_last_message_timestamp = midi_parser_cur_timestamp;
	
	for(size_t j = 0; j < SYNTH_UTIL_MIDI_NUM_CHANNELS; ++j) {
		for(size_t i = 0; i < SYNTH_UTIL_MIDI_NUM_FINE_CONTROLS; ++i) {
			midi_parser_pending_control_msbs[j][i] = -1;
		}
	}
	
	midi_parser_message_queue_head = 0;
	midi_parser_message_queue_tail = 0;
	memset(midi_parser_message_queue, 0, sizeof midi_parser_message_queue);
}


void midi_parser_shutdown(void)
{
	midi_parser_message_queue_head = 0;
	midi_parser_message_queue_tail = 0;
	memset(midi_parser_message_queue, 0, sizeof midi_parser_message_queue);
}


void midi_parser_parse_midi(uint32_t timestamp, uint8_t const * data,
	size_t size)
{
	for(size_t i = 0; i < size; ++i) {
		size_t result = 0;
		
		switch(data[i] & 0xF0) {
		case SYNTH_UTIL_MIDI_NOTE_ON:
			result = midi_parser_parse_midi_note_on(
				timestamp, data + i, size - i);
			break;
		case SYNTH_UTIL_MIDI_NOTE_OFF:
			result = midi_parser_parse_midi_note_off(
				timestamp, data + i, size - i);
			break;
		case SYNTH_UTIL_MIDI_AFTERTOUCH:
			result = midi_parser_parse_midi_aftertouch(
				timestamp, data + i, size - i);
			break;
		case SYNTH_UTIL_MIDI_CONTROL_CHANGE:
			result = midi_parser_parse_midi_control_change(
				timestamp, data + i, size - i);
			break;
		case SYNTH_UTIL_MIDI_PROGRAM_CHANGE:
			result = midi_parser_parse_midi_program_change(
				timestamp, data + i, size - i);
			break;
		default:
			break;
		}
		
		if(result > 0) {
			i += result - 1;
		}
	}
}


size_t midi_parser_parse_midi_note_on(synth_util_timestamp_t timestamp,
	uint8_t const * data, size_t size)
{
	synth_util_message_t message;
	
	message.timestamp = timestamp;
	message.channel = data[0] & 0x0F;
	
	if(midi_parser_check_midi_params(data, size, 2)) {
		if(data[2] > 0) {
			message.command = SYNTH_UTIL_COMMAND_NOTE_ON;
			message.data.note_on.note = (int8_t)data[1];
			message.data.note_on.velocity = (int8_t)data[2];
		} else {
			message.command = SYNTH_UTIL_COMMAND_NOTE_OFF;
			message.data.note_off.note = (int8_t)data[1];
			message.data.note_off.velocity = 0;
		}
		midi_parser_enqueue_message(message);
		return 3;
	} else {
		return 0;
	}
}


size_t midi_parser_parse_midi_note_off(synth_util_timestamp_t timestamp,
	uint8_t const * data, size_t size)
{
	synth_util_message_t message;
	
	message.timestamp = timestamp;
	message.channel = data[0] & 0x0F;
	
	if(midi_parser_check_midi_params(data, size, 2)) {
		message.command = SYNTH_UTIL_COMMAND_NOTE_OFF;
		message.data.note_off.note = (int8_t)data[1];
		message.data.note_off.velocity = (int8_t)data[2];
		midi_parser_enqueue_message(message);
		return 3;
	} else {
		return 0;
	}
}


size_t midi_parser_parse_midi_aftertouch(synth_util_timestamp_t timestamp,
	uint8_t const * data, size_t size)
{
	synth_util_message_t message;
	
	message.timestamp = timestamp;
	message.channel = data[0] & 0x0F;
	
	if(midi_parser_check_midi_params(data, size, 2)) {
		message.command = SYNTH_UTIL_COMMAND_AFTERTOUCH;
		message.data.note_off.note = (int8_t)data[1];
		message.data.note_off.velocity = (int8_t)data[2];
		midi_parser_enqueue_message(message);
		return 3;
	} else {
		return 0;
	}
}


size_t midi_parser_parse_midi_control_change(synth_util_timestamp_t timestamp,
	uint8_t const * data, size_t size)
{
	synth_util_message_t message;
	
	message.timestamp = timestamp;
	message.channel = data[0] & 0x0F;
	
	if(midi_parser_check_midi_params(data, size, 2)) {
		int8_t midi_control = (int8_t)data[1];
		int8_t midivalue = (int8_t)data[2];
		
		message.command = SYNTH_UTIL_COMMAND_CONTROL_CHANGE;
		message.data.control_change.control = midi_control;
		/* MSB changes to 14-bit controls get stored and applied
		 * after the LSB is received, to avoid spurious values. */
		if(midi_control < SYNTH_UTIL_MIDI_NUM_FINE_CONTROLS) {
			midi_parser_pending_control_msbs[message.channel][
				midi_control] = midivalue;
		} else if(midi_control < SYNTH_UTIL_MIDI_NUM_FINE_CONTROLS * 2)
		{
			int16_t pendingmsb =
				midi_parser_pending_control_msbs[message.channel][
				midi_control - SYNTH_UTIL_MIDI_NUM_FINE_CONTROLS];
			if(pendingmsb >= 0) {
				message.data.control_change.value = 
					((int16_t)pendingmsb << 8)
					| ((int16_t)data[2] << 1)
					| ((int16_t)pendingmsb >> 6);
				midi_parser_enqueue_message(message);
			}
		} else {
			message.data.control_change.value = 
				((int16_t)data[2] << 8)
				| ((int16_t)data[2] << 1)
				| ((int16_t)data[2] >> 6);
			midi_parser_enqueue_message(message);
		}
		return 3;
	} else {
		return 0;
	}
}


size_t midi_parser_parse_midi_program_change(synth_util_timestamp_t timestamp,
	uint8_t const * data, size_t size)
{
	synth_util_message_t message;
	
	message.timestamp = timestamp;
	message.channel = data[0] & 0x0F;
	
	if(midi_parser_check_midi_params(data, size, 1)) {
		message.command = SYNTH_UTIL_COMMAND_PROGRAM_CHANGE;
		message.data.program_change.program = (int8_t)data[1];
		midi_parser_enqueue_message(message);
		return 2;
	} else {
		return 0;
	}
}


bool midi_parser_check_midi_params(uint8_t const * data, size_t size,
	size_t paramsize)
{
	if(size < paramsize + 1) {
		return false;
	}
	for(size_t i = 1; i < paramsize + 1; ++i) {
		if((data[i] & 0x80) != 0) {
			return false;
		}
	}
	return true;
}


void midi_parser_enqueue_message(synth_util_message_t message)
{
	size_t next_message_queue_head;
	if(synth_util_subtract_timestamps(message.timestamp,
		midi_parser_get_play_time()) < 0)
	{
		fprintf(stderr, "queueing message in the past\n");
		return;
	}
	if((midi_parser_last_message_timestamp != 0) &&
		(synth_util_subtract_timestamps(message.timestamp,
		midi_parser_last_message_timestamp) < 0))
	{
		fprintf(stderr, "queueing message before last message\n");
		return;
	}
	next_message_queue_head = (midi_parser_message_queue_head + 1)
		% MIDI_PARSER_MESSAGE_QUEUE_LENGTH;
	if(next_message_queue_head == midi_parser_message_queue_tail) {
		fprintf(stderr, "message queue full");
		return;
	}
	midi_parser_message_queue[midi_parser_message_queue_head] =
		message;
	midi_parser_message_queue_head = next_message_queue_head;
	midi_parser_last_message_timestamp = message.timestamp;
}


synth_util_timestamp_t midi_parser_get_play_time(void)
{
	// funky volatile cast because we're doing a multithread-safe access
	// could have used a memory barrier but this is less of a bludgeon,
	// theoretically at least
	return *(synth_util_timestamp_t volatile *)&midi_parser_cur_timestamp;
}


synth_util_timespan_t midi_parser_get_time_to_next_message(void)
{
	if(midi_parser_message_queue_head != midi_parser_message_queue_tail)
	{
		return synth_util_subtract_timestamps(midi_parser_message_queue[
			midi_parser_message_queue_tail].timestamp,
			midi_parser_cur_timestamp);
	} else {
		return SYNTH_UTIL_TIMESPAN_MAX;
	}
}


void midi_parser_advance_time(synth_util_timespan_t timedelta)
{
	bool already_warned = false;
	
	while((midi_parser_get_time_to_next_message() < SYNTH_UTIL_TIMESPAN_MAX)
		&& (timedelta > midi_parser_get_time_to_next_message()))
	{
		if(!already_warned) {
			fprintf(stderr, "skipping messages -- this is wrong!\n");
			already_warned = true;
		}
		midi_parser_read_next_message();
	}
	midi_parser_cur_timestamp = synth_util_add_timespan_to_timestamp(
		midi_parser_cur_timestamp, timedelta);
	
	if((midi_parser_last_message_timestamp != 0)
		&& synth_util_subtract_timestamps(midi_parser_get_play_time(),
		midi_parser_last_message_timestamp) > (SYNTH_UTIL_SAMPLE_RATE / 4))
	{
		// 0 is a valid value, but statistically it won't come up often, and
		// invalidating the last_message_timestamp should be harmless anyway
		midi_parser_last_message_timestamp = 0;
	}

}


synth_util_message_t midi_parser_read_next_message(void)
{
	synth_util_message_t result = synth_util_message_null;
	
	result.timestamp = midi_parser_cur_timestamp;
	
	if(*(size_t const volatile *)&midi_parser_message_queue_head
		== midi_parser_message_queue_tail)
	{
		fprintf(stderr, "reading message but none in queue\n");
		return result;
	}
	
	if(midi_parser_message_queue[midi_parser_message_queue_tail].timestamp
		!= midi_parser_cur_timestamp)
	{
		fprintf(stderr, "reading message but no messages are current\n");
		return result;
	}
	
	result = midi_parser_message_queue[midi_parser_message_queue_tail];
	midi_parser_message_queue_tail = (midi_parser_message_queue_tail + 1)
		% MIDI_PARSER_MESSAGE_QUEUE_LENGTH;
	
	return result;
}


