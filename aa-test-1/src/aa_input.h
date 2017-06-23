#ifndef AA_INPUT_H
#define AA_INPUT_H


#include "amanita_arcade.h"


namespace AA {
  class Input {
  public:
    enum Button {
      B_RED   = 0x01,
      B_GREEN = 0x02,
      B_BLUE  = 0x04,
      B_PINK  = 0x08,
    };

    static float button_pressure(Button b);

    static bool button_state(Button b);
    static bool button_pressed(Button b);
    static bool button_released(Button b);

    static void read_buttons();

  private:
    static uint32_t _lastButtons;
    static uint32_t _buttons;
  };
}


#endif // AA_INPUT_H
