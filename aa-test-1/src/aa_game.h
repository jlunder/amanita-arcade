#ifndef AA_GAME_H
#define AA_GAME_H


#include "amanita_arcade.h"

#include "aa_time_span.h"


namespace aa {
  class Game {
  public:
    static void update(ShortTimeSpan dt);
  };
}


#endif // AA_GAME_H
