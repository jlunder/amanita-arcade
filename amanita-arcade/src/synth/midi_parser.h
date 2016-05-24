#ifndef MIDI_PARSER_H
#define MIDI_PARSER_H


#include "synth_core.h"


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


extern void midi_parser_init(void);
extern void midi_parser_shutdown(void);

// the parse_midi function must be called from the non-realtime thread to
// inject new messages; the realtime thread can't call this itself
extern void midi_parser_parse_midi(uint32_t timestamp, uint8_t const * data,
	size_t size);

// get_play_time can be called from the non-realtime thread to get a lower
// bound on the current playback pointer
extern synth_util_timestamp_t midi_parser_get_play_time(void);

extern synth_util_timespan_t midi_parser_get_time_to_next_message(void);
extern void midi_parser_advance_time(synth_util_timespan_t timedelta);
extern synth_util_message_t midi_parser_read_next_message(void);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif /* MIDI_PARSER_H */


