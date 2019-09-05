#ifndef AA_GAME_VIS_H
#define AA_GAME_VIS_H


#include "amanita_arcade.h"

#include "aa_time_span.h"


namespace aa {
  class Vis {
  public:
    static void init();
    static Vis & vis();

    virtual ~Vis() { }

    virtual void attract_start() { }
    virtual void update(ShortTimeSpan dt) { }
    virtual void game_start() { }
    virtual void play_pattern() { }
    virtual void play_color(char id) { }
    virtual void await_press() { }
    virtual void press_color(char id, bool correct) { }
    virtual void score_change(int32_t score) { }
    virtual void game_over(int32_t final_score, uint16_t rating, bool win) { }
    virtual void enter_name(char const * name, size_t cursor_pos) { }
  };
}


#endif // AA_GAME_H
