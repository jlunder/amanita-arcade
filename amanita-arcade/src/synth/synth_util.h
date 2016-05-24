#ifndef SYNTH_UTIL_H
#define SYNTH_UTIL_H


#include "synth_core.h"


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


#define SYNTH_UTIL_TIMESPAN_MAX (32767)

/* the standard format is interleaved */

#define SYNTH_UTIL_SAMPLE_RATE (32768L)
#define SYNTH_UTIL_NUM_CHANNELS (2)
#define SYNTH_UTIL_SAMPLE_MAX (32767)
#define SYNTH_UTIL_SAMPLE_MIN (-32768)

#define SYNTH_UTIL_MIDI_NOTE_OFF 0x80
#define SYNTH_UTIL_MIDI_NOTE_ON 0x90
#define SYNTH_UTIL_MIDI_AFTERTOUCH 0xA0
#define SYNTH_UTIL_MIDI_CONTROL_CHANGE 0xB0
#define SYNTH_UTIL_MIDI_PROGRAM_CHANGE 0xC0

#define SYNTH_UTIL_MIDI_NOTE_A440 (69)

#define SYNTH_UTIL_MIDI_NUM_CHANNELS (16)
#define SYNTH_UTIL_MIDI_NUM_FINE_CONTROLS (32)
#define SYNTH_UTIL_MIDI_NUM_NOTES (128)
#define SYNTH_UTIL_MIDI_NUM_CONTROLS (128)
#define SYNTH_UTIL_MIDI_NUM_PROGRAMS (128)

#define SYNTH_UTIL_MIDI_FINE_CONTROL_RANGE (16384)
#define SYNTH_UTIL_MIDI_COARSE_CONTROL_RANGE (128)

#define SYNTH_UTIL_COMMAND_NULL 0
#define SYNTH_UTIL_COMMAND_NOTE_OFF 1
#define SYNTH_UTIL_COMMAND_NOTE_ON 2
#define SYNTH_UTIL_COMMAND_AFTERTOUCH 3
#define SYNTH_UTIL_COMMAND_CONTROL_CHANGE 4
#define SYNTH_UTIL_COMMAND_PROGRAM_CHANGE 5

#define SYNTH_UTIL_FRACTIONAL_MIDI_NOTE_FRACTIONAL_BITS (16)
#define SYNTH_UTIL_FRACTIONAL_MIDI_NOTE_1 (65536L)

#define SYNTH_UTIL_FRACTIONAL_PLAY_RATE_FRACTIONAL_BITS (32)
#define SYNTH_UTIL_FRACTIONAL_PLAY_RATE_1 (0x100000000L)

#define SYNTH_UTIL_PARAMETER_FRACTIONAL_BITS (16)
#define SYNTH_UTIL_PARAMETER_1 (65536L)


typedef uint16_t synth_util_timestamp_t;
typedef int16_t synth_util_timespan_t;

typedef int8_t synth_util_midi_note_t;
typedef int32_t synth_util_fractional_midi_note_t;
typedef float synth_util_play_rate_t;
typedef int64_t synth_util_fractional_play_rate_t;
typedef int32_t synth_util_parameter_t;

typedef int16_t synth_util_sample_t;

typedef struct synth_util_message_ {
	synth_util_timestamp_t timestamp;
	int8_t channel;
	int8_t command;
	union synth_util_message_data_ {
		struct synth_util_message_data_note_on_ {
			synth_util_midi_note_t note;
			int8_t velocity;
		} note_on;
		struct synth_util_message_data_note_off_ {
			synth_util_midi_note_t note;
			int8_t velocity;
		} note_off;
		struct synth_util_message_data_aftertouch_ {
			synth_util_midi_note_t note;
			int8_t velocity;
		} aftertouch;
		struct synth_util_message_data_control_change_ {
			int8_t control;
			synth_util_parameter_t value;
		} control_change;
		struct synth_util_message_data_program_change_ {
			int8_t program;
		} program_change;
	} data;
} synth_util_message_t;


extern synth_util_message_t const synth_util_message_null;

extern int32_t const synth_util_velocity_map[128];

extern region_allocator_t synth_util_temp_allocator;


extern void synth_util_init(void);
extern void synth_util_shutdown(void);

extern synth_util_play_rate_t synth_util_tuning_to_a440_play_rate(
	double recorded_midi_note, double sampling_rate);
extern synth_util_play_rate_t synth_util_midi_note_to_play_rate(
	synth_util_play_rate_t a440_play_rate, synth_util_midi_note_t midi_note);
extern synth_util_play_rate_t synth_util_fractional_midi_note_to_play_rate(
	synth_util_play_rate_t a440_play_rate,
	synth_util_fractional_midi_note_t fractional_midi_note);

/* returns x - y */
extern synth_util_timespan_t synth_util_subtract_timestamps(
	synth_util_timestamp_t x, synth_util_timestamp_t y);
extern synth_util_timestamp_t synth_util_add_timespan_to_timestamp(
	synth_util_timestamp_t x, synth_util_timespan_t y);
extern synth_util_timespan_t synth_util_add_timespans(
	synth_util_timespan_t x, synth_util_timespan_t y);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif /* SYNTH_UTIL_H */


