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

typedef struct {
	bool buttons_pressed[8];
	bool buttons_held[8];
} aa_game_input_t;


extern aa_game_tune_t const * aa_game_tunes;


void aa_game_init(void);
void aa_game_loop(void);
void aa_game_reset_timeout(void);
void aa_game_change_mode(aa_game_mode_t mode);

void aa_game_attract_init(void);
void aa_game_attract_loop(void);
void aa_game_attract_shutdown(void);

void aa_game_simon_init(void);
void aa_game_simon_loop(void);
void aa_game_simon_shutdown(void);

void aa_game_harmonize_init(void);
void aa_game_harmonize_loop(void);
void aa_game_harmonize_shutdown(void);

void aa_game_play_along_init(void);
void aa_game_play_along_loop(void);
void aa_game_play_along_shutdown(void);

void aa_game_free_play_init(void);
void aa_game_free_play_loop(void);
void aa_game_free_play_shutdown(void);


#endif // AA_GAME_H_

