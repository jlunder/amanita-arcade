#ifndef AA_INPUT_H
#define AA_INPUT_H


#include "amanita_arcade.h"

#include "aa_time_span.h"


namespace aa {
  class Input {
  public:
    enum Button {
      B_RED   = 0x01,
      B_GREEN = 0x02,
      B_BLUE  = 0x04,
      B_PINK  = 0x08,
    };

    static void init();

    static bool button_state(Button b) { return (_buttons & b) != 0; }
    static bool button_pressed(Button b) {
      return (_buttons & (_last_buttons ^ _buttons) & b) != 0;
    }
    static bool button_released(Button b) {
      return (~_buttons & (_last_buttons ^ _buttons) & b) != 0;
    }

    static ShortTimeSpan get_time_since_last_sample() {
      return _time_since_last_sample;
    }

    static void read_buttons();

  private:
    static uint32_t _last_buttons;
    static uint32_t _buttons;
    static ShortTimeSpan _time_since_last_sample;
  };
}


#endif // AA_INPUT_H
