#include "amanita_arcade.h"

#include "aa_game.h"
#include "aa_input.h"
#include "aa_lights.h"
#include "aa_sound.h"
#include "hardware.h"
#include "peripherals.h"

int main(void) {
	uint32_t last_tick;

	// Send a greeting to the trace device (skipped on Release).
	cu_log("Hello ARM World!\n");

	// At this stage the system clock should have already been configured
	// at high speed.
	cu_log("System clock: %u Hz\n", SystemCoreClock);

	per_init();
	aa_input_init();
	aa_lights_init();
	aa_sound_init();

	last_tick = HAL_GetTick();

	for( ;; ) {
		uint32_t cur_tick = HAL_GetTick();

		if((uint32_t)(cur_tick - last_tick) < 5) {
			continue;
		}
		last_tick = cur_tick;

		aa_input_read_buttons();
		aa_game_loop();
		while((HAL_GetTick() - last_tick) < 1) {
			// wait for 1ms: timing consistency for animations...
		}
		aa_lights_update();
		aa_sound_update();
	}
}

void cu_abort(void) {
#if defined(DEBUG)
	__DEBUG_BKPT();
#endif
	for( ;; ) {
	}
}

void cu_log(char const * format, ...) {
	static char buf[1000];

	int written;
	va_list ap;

	va_start(ap, format);

	written = vsnprintf(buf, sizeof(buf), format, ap);
	if (written > 0) {
		trace_write(buf, (size_t) written);
	}

	va_end(ap);
}

