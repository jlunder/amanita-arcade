#include "amanita_arcade.h"

#include "aa_game.h"

#include "aa_input.h"


namespace aa {
  void Game::update(ShortTimeSpan dt) {
    /*
    Debug::tracef("%6d %c %c %c %c", dt.to_micros(),
      Input::button_state(Input::B_RED) ? 'R' : 'r',
      Input::button_state(Input::B_GREEN) ? 'G' : 'g',
      Input::button_state(Input::B_BLUE) ? 'B' : 'b',
      Input::button_state(Input::B_PINK) ? 'P' : 'p');
      */
    if(Input::button_pressed(Input::B_RED)) {
      Debug::tracef("Red pressed");
    }
    if(Input::button_pressed(Input::B_GREEN)) {
      Debug::tracef("Green pressed");
    }
    if(Input::button_pressed(Input::B_BLUE)) {
      Debug::tracef("Blue pressed");
    }
    if(Input::button_pressed(Input::B_PINK)) {
      Debug::tracef("Pink pressed");
    }
  }
}
