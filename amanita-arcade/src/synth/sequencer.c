#include "sequencer.h"


//#define SEQUENCER_BUFFER_SIZE (32L*(1L<<20))
#define SEQUENCER_BUFFER_SIZE (65536L)
// 500 ms @ 120BPM
#define SEQUENCER_FEED_WINDOW \
	((120 * SEQUENCER_TIMESPAN_BEAT * 500L) / (60 * 1000))

#define SEQUENCER_FRACTIONAL_TIMESPAN_1 (1LL << 32)


typedef uint64_t sequencer_fractional_timestamp_t;
typedef int64_t sequencer_fractional_timespan_t;


static uint8_t sequencer_buffer[SEQUENCER_BUFFER_SIZE];
static size_t sequencer_buffer_head;
static size_t sequencer_buffer_tail;
static synth_util_timestamp_t sequencer_external_play_time;
static sequencer_fractional_timestamp_t sequencer_fractional_play_time;
static sequencer_fractional_timespan_t sequencer_fractional_play_rate;


static synth_util_timestamp_t sequencer_timestamp_to_synth_util_timestamp(
	sequencer_timestamp_t sequencer_timestamp);


void sequencer_init(void)
{
	sequencer_buffer_head = 0;
	sequencer_buffer_tail = 0;
	sequencer_external_play_time = 0;
	sequencer_fractional_play_time = 0;
	sequencer_fractional_play_rate =
		(120 * SEQUENCER_TIMESPAN_BEAT * SEQUENCER_FRACTIONAL_TIMESPAN_1
		+ (60 * SYNTH_UTIL_SAMPLE_RATE) / 2) / (60 * SYNTH_UTIL_SAMPLE_RATE);
	memset(sequencer_buffer, 0, sizeof sequencer_buffer);
}


void sequencer_shutdown(void)
{
}


void sequencer_feed_midi_parser(void)
{
	sequencer_timestamp_t message_timestamp;
	sequencer_timespan_t time_to_next_message;
	size_t next_buffer_tail;
	uint8_t message[SEQUENCER_MAX_DATA_SIZE];
	size_t message_size;
	
	while(sequencer_buffer_tail != sequencer_buffer_head) {
		next_buffer_tail = sequencer_buffer_tail;
		message_timestamp = 0;
		for(size_t i = 0; i < sizeof (sequencer_timestamp_t); ++i) {
			if(next_buffer_tail == sequencer_buffer_head) {
				break;
			}
			message_timestamp = message_timestamp << 8;
			message_timestamp |= sequencer_buffer[next_buffer_tail];
			next_buffer_tail = (next_buffer_tail + 1)
				% SEQUENCER_BUFFER_SIZE;
		}
		time_to_next_message = sequencer_subtract_timestamps(
			message_timestamp,
			sequencer_fractional_play_time / SEQUENCER_FRACTIONAL_TIMESPAN_1);
		if(time_to_next_message > SEQUENCER_FEED_WINDOW) {
			break;
		}
		if(time_to_next_message < 0) {
			fprintf(stderr, "data read past deadline, skipping!\n");
			sequencer_buffer_tail = next_buffer_tail;
			continue;
		}
		
		if(next_buffer_tail == sequencer_buffer_head) {
			goto data_error;
		}
		message_size = sequencer_buffer[next_buffer_tail];
		next_buffer_tail = (next_buffer_tail + 1)
			% SEQUENCER_BUFFER_SIZE;
		for(size_t i = 0; i < message_size; ++i) {
			if(next_buffer_tail == sequencer_buffer_head) {
				goto data_error;
			}
			message[i] = sequencer_buffer[next_buffer_tail];
			next_buffer_tail = (next_buffer_tail + 1)
				% SEQUENCER_BUFFER_SIZE;
		}
		midi_parser_parse_midi(
			sequencer_timestamp_to_synth_util_timestamp(message_timestamp),
			message, message_size);
		sequencer_buffer_tail = next_buffer_tail;
	}
	return;
data_error:
	fprintf(stderr, "bad data in sequencer queue!\n");
	sequencer_buffer_tail = sequencer_buffer_head;
}


void sequencer_synchronize_to_external_time(synth_util_timestamp_t timestamp)
{
	sequencer_external_play_time = timestamp;
}


void sequencer_advance_time(synth_util_timespan_t timespan)
{
	sequencer_external_play_time = synth_util_add_timespan_to_timestamp(
		sequencer_external_play_time, timespan);
	sequencer_fractional_play_time +=
		timespan * sequencer_fractional_play_rate;
}


void sequencer_enqueue_data(sequencer_timestamp_t timestamp,
	uint8_t const * data, size_t data_size)
{
	size_t remaining = (sequencer_buffer_tail + SEQUENCER_BUFFER_SIZE
		- sequencer_buffer_head - 1) % SEQUENCER_BUFFER_SIZE;
	sequencer_timestamp_t timestamp_temp = timestamp;
	
	if(remaining < (data_size + sizeof (sequencer_timestamp_t) + 1)) {
		fprintf(stderr, "not enough room in sequencer queue to add data\n");
		return;
	}
	if(data_size >= SEQUENCER_MAX_DATA_SIZE) {
		fprintf(stderr, "enqueueing too much data at once\n");
		return;
	}
	
	for(size_t i = 0; i < sizeof (sequencer_timestamp_t); ++i) {
		sequencer_buffer[sequencer_buffer_head++] = (timestamp_temp >>
			((sizeof (sequencer_timestamp_t) - 1) * 8)) & 0xFF;
		timestamp_temp = timestamp_temp << 8;
		sequencer_buffer_head %= SEQUENCER_BUFFER_SIZE;
	}
	sequencer_buffer[sequencer_buffer_head++] = data_size;
	sequencer_buffer_head %= SEQUENCER_BUFFER_SIZE;
	for(size_t i = 0; i < data_size; ++i) {
		sequencer_buffer[sequencer_buffer_head++] = *(data++);
		sequencer_buffer_head %= SEQUENCER_BUFFER_SIZE;
	}
}


sequencer_timestamp_t sequencer_get_play_time(void)
{
	return sequencer_fractional_play_time / SEQUENCER_FRACTIONAL_TIMESPAN_1
		 + SEQUENCER_FEED_WINDOW;
}


sequencer_timespan_t sequencer_subtract_timestamps(sequencer_timestamp_t x,
	sequencer_timestamp_t y)
{
	return (sequencer_timespan_t)(x - y);
}


sequencer_timestamp_t sequencer_add_timespan_to_timestamp(
	sequencer_timestamp_t x, sequencer_timespan_t y)
{
	return x + (sequencer_timestamp_t)y;
}


sequencer_timespan_t sequencer_add_timespans(sequencer_timespan_t x,
	sequencer_timespan_t y)
{
	return x + y;
}


synth_util_timestamp_t sequencer_timestamp_to_synth_util_timestamp(
	sequencer_timestamp_t sequencer_timestamp)
{
	return (((sequencer_timestamp * SEQUENCER_FRACTIONAL_TIMESPAN_1)
		- sequencer_fractional_play_time) / sequencer_fractional_play_rate)
		+ sequencer_external_play_time;
}


