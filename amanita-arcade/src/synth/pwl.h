#ifndef PWL_H
#define PWL_H


#include "synth_core.h"


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


#define PWL_PARAMETER_MAX_COMPONENTS (16)


typedef struct pwl_point_ {
	int32_t fractional_time;
	int32_t fractional_value;
} pwl_point_t;


typedef struct pwl_envelope_ {
	int16_t length;
	int16_t loop_start;
	int16_t loop_end;
	uint8_t note_off_stop_looping;
	uint8_t pad;
	pwl_point_t points[1];
} pwl_envelope_t;


typedef struct pwl_generator_ {
	pwl_envelope_t * pwl_envelope;
	int16_t last_point;
	int32_t time;
	synth_util_parameter_t value;
	bool looping;
} pwl_generator_t;


void pwl_generator_init(pwl_generator_t * pwl_generator,
	pwl_envelope_t * pwl_envelope);
void pwl_generator_destroy(pwl_generator_t * pwl_generator);

void pwl_generator_advance_time(pwl_generator_t * pwl_generator,
	synth_util_timespan_t time);
synth_util_parameter_t pwl_generator_get_value(
	pwl_generator_t * pwl_generator);
bool pwl_generator_get_looping(pwl_generator_t * pwl_generator);
void pwl_generator_note_off(pwl_generator_t * pwl_generator);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif /* PWL_H */


