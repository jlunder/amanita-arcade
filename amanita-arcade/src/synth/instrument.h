#ifndef INSTRUMENT_H
#define INSTRUMENT_H


#include "synth_core.h"


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


typedef void (* instrument_process_message_t)(void * context,
	synth_util_message_t const * message);
typedef void (* instrument_generate_t)(void * context,
	synth_util_sample_t * dest, size_t num_frames);
typedef synth_util_timespan_t (* instrument_time_to_next_internal_event_t)(
	void * context);

typedef struct instrument_definition_ {
	void * context;
	instrument_process_message_t process_message;
	instrument_generate_t generate;
	instrument_time_to_next_internal_event_t time_to_next_internal_event;
} instrument_definition_t;


extern void instrument_manager_init(void);
extern void instrument_manager_shutdown(void);

// the start/stop_playback functions can be used to temporarily shut down
// the realtime playback system so the instruments can be modified from the
// non-realtime thread; e.g., to swap sample banks, add/remove instruments,
// alter the effects chain
extern void instrument_manager_start_playback(void);
extern void instrument_manager_stop_playback(void);

// stop_playback is asynchronous, so it is necessary to wait until
// get_playing returns false before actually manipulating the playback system
extern bool instrument_manager_get_playing(void);

extern void instrument_manager_set_mix_level(
	synth_util_parameter_t mix_level);

extern void instrument_manager_add_instrument(int8_t midi_channel,
	instrument_definition_t * instrument_definition);
extern void instrument_manager_remove_instrument(int8_t midi_channel);

extern void instrument_manager_render(synth_util_sample_t * dest,
	size_t num_frames);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif // INSTRUMENT_H


