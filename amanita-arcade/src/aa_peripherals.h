/*
 * heart_control_peripherals.h
 *
 *  Created on: Oct 17, 2015
 *      Author: jlunder
 */

#ifndef AA_PERIPHERALS_H_
#define AA_PERIPHERALS_H_


#include "hardware.h"

#define AA_WS2811_BITBANG

typedef struct {
	uint8_t b, r, g;
} ws2811_color_t;

void aa_peripherals_init(void);

void cs43l22_init(void);
void cs43l22_write_register(uint8_t address, uint8_t value);
uint8_t cs43l22_read_register(uint8_t address);
void cs43l22_start(hw_i2s_fill_func_t fill_func);
void cs43l22_stop(void);

void mpr121_init(void);
void mpr121_write_register(uint8_t address, uint8_t value);
void mpr121_write_registers(uint8_t address, void const * value, size_t size);
void mpr121_read_registers(uint8_t address, void * value, size_t size);
void mpr121_auto_configure(size_t num_sensors);
void mpr121_set_thresholds(uint8_t start, uint8_t * thresholds, size_t count);
uint16_t mpr121_get_touch_states(void);
void mpr121_get_analog_baselines(uint8_t start, uint16_t * baselines, size_t count);
void mpr121_get_analog_values(uint8_t start, uint16_t * values, size_t count);

#ifdef AA_WS2811_SPI
void ws2801_init(void);
void ws2801_output(void const * buf, size_t buf_len);
#endif

void ws2811_init(void);
void ws2811_start(void);
void ws2811_stop(void);
#ifdef AA_WS2811_BITBANG
void ws2811_output(void const * buf, size_t buf_len);
#endif
#ifdef AA_WS2811_I2S
void ws2811_output_nb(void const * buf, size_t buf_len);
bool ws2811_get_outputting(void);
#endif


#endif /* AA_PERIPHERALS_H_ */
