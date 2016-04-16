/*
 * heart_control_peripherals.h
 *
 *  Created on: Oct 17, 2015
 *      Author: jlunder
 */

#ifndef PERIPHERALS_H_
#define PERIPHERALS_H_


#include "hardware.h"

void per_init(void);

void cs43l22_init(void);
void cs43l22_write_register(uint8_t address, uint8_t value);
uint8_t cs43l22_read_register(uint8_t address);
void cs43l22_start(hw_i2s_fill_func_t fill_func);
void cs43l22_stop(void);

void ws2801_init(void);
void ws2801_output(void * buf, size_t buf_len);


#endif /* PERIPHERALS_H_ */
