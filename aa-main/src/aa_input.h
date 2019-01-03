#ifndef AA_INPUT_H
#define AA_INPUT_H


#include "amanita_arcade.h"

#include "aa_time_span.h"


namespace aa {
  class Input {
  public:
    static const uint32_t B_RED   = 0x01;
    static const uint32_t B_GREEN = 0x02;
    static const uint32_t B_BLUE  = 0x04;
    static const uint32_t B_PINK  = 0x08;

    static void init();

    static bool connected();

    static uint32_t all_state() { return _buttons; }
    static uint32_t all_pressed() {
      return _buttons & (_last_buttons ^ _buttons);
    }
    static uint32_t all_released() {
      return ~_buttons & (_last_buttons ^ _buttons);
    }

    static bool button_state(uint32_t b) { return (_buttons & b) != 0; }
    static bool button_pressed(uint32_t b) {
      return (all_pressed() & b) != 0;
    }
    static bool button_released(uint32_t b) {
      return (all_released() & b) != 0;
    }

    static ShortTimeSpan get_remote_dt() {
      return _remote_dt;
    }

    static void read_buttons(ShortTimeSpan dt);

  private:
    static ShortTimeSpan _remote_dt;
    static uint32_t _last_buttons;
    static uint32_t _buttons;

    static void read_remote_buttons();
  };
}


#endif // AA_INPUT_H
