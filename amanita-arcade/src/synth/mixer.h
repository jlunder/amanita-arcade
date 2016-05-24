#ifndef MIXER_H
#define MIXER_H


#include "synth_core.h"


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


#define MIXER_BUFFERLENGTH 1024
#define MIXER_NUMINPUTS 16
#define MIXER_NUMCHANNELS 2

#define MIXER_SAMPLERINPUTSSTART 0
#define MIXER_NUMSAMPLERINPUTS 12


extern void mixer_init(void);
extern void mixer_shutdown(void);

extern int16_t * mixer_getinputbuffer(int input);
extern void mixer_process(int16_t * outputbuffer);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif /* MIXER_H */


