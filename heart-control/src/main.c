#include "heart_control.h"

#include "hardware.h"

int main(void) {
	hw_assignment_id_t ld3, ld4, ld5, ld6;

	// Send a greeting to the trace device (skipped on Release).
	cu_log("Hello ARM World!\n");

	// At this stage the system clock should have already been configured
	// at high speed.
	cu_log("System clock: %u Hz\n", SystemCoreClock);

	uint32_t seconds = 0;

	ld3 = hw_pin_assign(HWR_PD12);
	ld4 = hw_pin_assign(HWR_PD13);
	ld5 = hw_pin_assign(HWR_PD14);
	ld6 = hw_pin_assign(HWR_PD15);

	hw_pin_configure(ld3, HWPM_OUT_PP);
	hw_pin_configure(ld4, HWPM_OUT_PP);
	hw_pin_configure(ld5, HWPM_OUT_PP);
	hw_pin_configure(ld6, HWPM_OUT_PP);


	// Infinite loop
	while (1) {
		++seconds;
		// Count seconds on the trace device.

		GPIOD->BSRR = 1LU << (12 * 1 + 0);
		GPIOD->BSRR = 1LU << (13 * 1 + 0);
		GPIOD->BSRR = 1LU << (14 * 1 + 16);
		GPIOD->BSRR = 1LU << (15 * 1 + 16);
		cu_log("Second %u -> A\n", seconds);
		GPIOD->BSRR = 1LU << (12 * 1 + 16);
		GPIOD->BSRR = 1LU << (13 * 1 + 16);
		GPIOD->BSRR = 1LU << (14 * 1 + 0);
		GPIOD->BSRR = 1LU << (15 * 1 + 0);
		cu_log("Second %u -> B\n", seconds);
	}
	// Infinite loop, never return.
}

void cu_abort(void) {
	static size_t volatile counter = 0;

	for (;;) {
		++counter;
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

