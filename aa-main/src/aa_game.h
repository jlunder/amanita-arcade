#ifndef AA_GAME_H
#define AA_GAME_H


#include "amanita_arcade.h"

#include "aa_time_span.h"


namespace aa {
  class Game {
  public:
    static void init();
    static void update(ShortTimeSpan dt);

  private:
    static bool is_button_pressed();
    static char get_pressed_button();
    static char get_random_button();
    static void trigger_bubble(char button);
  };
}


#endif // AA_GAME_H
