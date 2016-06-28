#include "amanita_arcade.h"

#include "aa_game.h"
#include "aa_input.h"
#include "aa_lights.h"
#include "aa_peripherals.h"
#include "aa_sound.h"
#include "hardware.h"

int main(void) {
	cu_log("Starting Amanita Arcade\n");

	aa_init();

	for( ;; ) {
		aa_loop();
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

