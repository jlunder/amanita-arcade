#include "heart_control.h"

#include "hardware.h"
#include "heart_control_peripherals.h"
#include "heartbeat_data.h"

static size_t hc_audio_pos = 0;

static void hc_fill_i2s(void * buf, size_t buf_len);

void hc_fill_i2s(void * buf, size_t buf_len) {
	uint16_t * sample_buf = (uint16_t *)buf;
	size_t frames = buf_len / (sizeof (uint16_t) * 2);
	size_t audio_pos_temp = hc_audio_pos;

	for(size_t i = 0; i < frames; ++i) {
		uint16_t s = (uint16_t)hd_heartbeat_data[audio_pos_temp];
		*sample_buf++ = s;
		*sample_buf++ = s;
		++audio_pos_temp;
		if(audio_pos_temp >= cu_lengthof(hd_heartbeat_data)) {
			audio_pos_temp = 0;
		}
	}
	hc_audio_pos = audio_pos_temp;
	__sync_synchronize();
}

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

	hcp_init();

	cs43l22_start(hc_fill_i2s);

	// Infinite loop
	while (1) {
		++seconds;
		// Count seconds on the trace device.

		//GPIOD->BSRR = 1LU << (12 * 1 + 0);
		//GPIOD->BSRR = 1LU << (13 * 1 + 0);
		//GPIOD->BSRR = 1LU << (14 * 1 + 16);
		//GPIOD->BSRR = 1LU << (15 * 1 + 16);
		//cu_log("Second %u -> A\n", seconds);
		//GPIOD->BSRR = 1LU << (12 * 1 + 16);
		//GPIOD->BSRR = 1LU << (13 * 1 + 16);
		//GPIOD->BSRR = 1LU << (14 * 1 + 0);
		//GPIOD->BSRR = 1LU << (15 * 1 + 0);
		//cu_log("Second %u -> B\n", seconds);
	}
	// Infinite loop, never return.

	cs43l22_stop();
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

