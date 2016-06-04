#ifndef AA_GAME_H_
#define AA_GAME_H_


#include "amanita_arcade.h"


typedef enum {
	AAGM_ATTRACT,
	AAGM_DEBUG,
	AAGM_SIMON,
	AAGM_HARMONIZE,
	AAGM_PLAY_ALONG,
	AAGM_FREE_PLAY,
} aa_game_mode_t;


typedef struct {
	size_t length;
	struct {
		uint16_t beats_delay;
		uint8_t melody;
		uint8_t pad0;
		uint8_t harmonies[4];
	} notes[0];
} aa_game_tune_t;


aa_game_tune_t const * tunes;


void aa_game_initialize(void);
void aa_game_loop(void);
void aa_game_reset_timeout(void);
void aa_game_change_mode(aa_game_mode_t mode);

void aa_game_attract_initialize(void);
void aa_game_attract_loop(void);
void aa_game_attract_shutdown(void);

void aa_game_simon_initialize(void);
void aa_game_simon_loop(void);
void aa_game_simon_shutdown(void);

void aa_game_harmonize_initialize(void);
void aa_game_harmonize_loop(void);
void aa_game_harmonize_shutdown(void);

void aa_game_play_along_initialize(void);
void aa_game_play_along_loop(void);
void aa_game_play_along_shutdown(void);

void aa_game_free_play_initialize(void);
void aa_game_free_play_loop(void);
void aa_game_free_play_shutdown(void);

#endif // AA_GAME_H_

