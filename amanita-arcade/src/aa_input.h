/*
 * aa_input.h
 *
 *  Created on: Jun 8, 2016
 *      Author: jlunder
 */

#ifndef AA_INPUT_H_
#define AA_INPUT_H_

#include "amanita_arcade.h"

typedef enum {
	AAIB_A0 = 0,
	AAIB_A1,
	AAIB_B0,
	AAIB_B1,
	AAIB_C0,
	AAIB_C1,
	AAIB_D0,
	AAIB_D1,

	AAIB_COUNT
} aa_input_button_id_t;

void aa_input_init(void);
void aa_input_read_buttons(void);

bool aa_input_button_state(aa_input_button_id_t button);
bool aa_input_button_press(aa_input_button_id_t button);

#endif /* AA_INPUT_H_ */
