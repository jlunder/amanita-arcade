#include "synth_core.h"


void synth_core_init(void)
{
	synth_util_init();
	midi_parser_init();
	sequencer_init();
	instrument_manager_init();
}


void synth_core_shutdown(void)
{
	instrument_manager_shutdown();
	sequencer_shutdown();
	midi_parser_shutdown();
	synth_util_shutdown();
}


