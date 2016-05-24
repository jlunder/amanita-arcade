#ifndef SYNTH_CORE_H
#define SYNTH_CORE_H


#include <math.h>
#if 0
#include <nds.h>
#endif
#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ssrm/ssrm.h"
#include "sync.h"

#include "region_allocator.h"
#include "synth_util.h"

#include "instrument.h"
#include "midi_parser.h"
#include "mixer.h"
#include "pwl.h"
#include "sequencer.h"
#include "sample_generator.h"
#include "sampler.h"
#include "sequencer.h"


#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


extern void synth_core_init(void);
extern void synth_core_shutdown(void);


#ifdef __cplusplus
}
#endif // __cplusplus


#endif // SYNTH_CORE_H


