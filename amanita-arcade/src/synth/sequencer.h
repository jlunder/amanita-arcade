#ifndef SEQUENCER_H
#define SEQUENCER_H


#include "synth_core.h"


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


#define SEQUENCER_TIMESPAN_MAX SYNTH_UTIL_TIMESPAN_MAX
// the number of sequencer_timespan_t units per beat. conventionally, a beat
// is a quarter note; you can use the tempo in beats per minute to calculate
// the conversion to real time.
#define SEQUENCER_TIMESPAN_BEAT (12)

#define SEQUENCER_MAX_DATA_SIZE (256)

typedef uint16_t sequencer_timestamp_t;
typedef int16_t sequencer_timespan_t;


extern void sequencer_init(void);
extern void sequencer_shutdown(void);

extern void sequencer_feed_midi_parser(void);
extern void sequencer_synchronize_to_external_time(
	synth_util_timestamp_t timestamp);
extern void sequencer_advance_time(synth_util_timespan_t timespan);

extern void sequencer_enqueue_data(sequencer_timestamp_t timestamp,
	uint8_t const * data, size_t data_size);
extern sequencer_timestamp_t sequencer_get_play_time(void);
// sets the timebase, i.e. the number of ticks per timespan.
extern void sequencer_set_timebase(double ticks_per_second);

/* returns x - y */
extern sequencer_timespan_t sequencer_subtract_timestamps(
	sequencer_timestamp_t x, sequencer_timestamp_t y);
extern sequencer_timestamp_t sequencer_add_timespan_to_timestamp(
	sequencer_timestamp_t x, sequencer_timespan_t y);
extern sequencer_timespan_t sequencer_add_timespans(
	sequencer_timespan_t x, sequencer_timespan_t y);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif /* SEQUENCER_H */


