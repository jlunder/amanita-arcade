#include "amanita_arcade.h"

#include "aa_input.h"

#include "aa_timer.h"


namespace aa {
  enum InputParserState {
    IPS_IDLE,
    IPS_TYPE,
    IPS_SEPARATOR_A,
    IPS_MICROS_0,
    IPS_MICROS_1,
    IPS_MICROS_2,
    IPS_MICROS_3,
    IPS_MICROS_4,
    IPS_MICROS_5,
    IPS_MICROS_6,
    IPS_MICROS_7,
    IPS_SEPARATOR_B,
    IPS_STATUS_0,
    IPS_STATUS_1,
    IPS_SEPARATOR_C,
    IPS_BUTTONS_0,
    IPS_BUTTONS_1,
    IPS_BUTTONS_2,
    IPS_BUTTONS_3,
    IPS_EOL,
  };

  namespace {
    static bool new_sample_available;
    static uint32_t sample_micros;
    static uint32_t sample_buttons;

    static uint32_t last_sample_micros;

    static InputParserState parser_state;
    static uint32_t parser_micros;
    static uint32_t parser_status;
    static uint32_t parser_buttons;

    static void input_test_io_isr();
    static void input_test_io_isr_parse(char in_c);
    static void input_test_io_isr_parse_reset();
    static void input_test_io_isr_parse_begin();
    static void input_test_io_isr_parse_literal(char in_c, char c);
    static void input_test_io_isr_parse_digit(char in_c, uint32_t * v,
        int pos);

    static void input_test_io_isr() {
      for(int i = 0; i < 10; ++i) {
        while(hardware::test_io_ser.readable()) {
          int c = hardware::test_io_ser.getc();
          if(c >= 0) {
            input_test_io_isr_parse(c);
            //hardware::test_io_ser.putc('A' + (char)parser_state);
          } else {
            parser_state = IPS_IDLE;
          }
          //Debug::tracef("%2d %08X %08X %08X %c", parser_state, parser_micros, parser_status, parser_buttons, new_sample_available ? 'T' : 'f');
        }
      }

      __disable_irq();
      __sync_synchronize();
      if(!new_sample_available && (parser_state == IPS_EOL)) {
        sample_micros = parser_micros;
        sample_buttons = parser_buttons;
        parser_state = IPS_IDLE;
        __sync_synchronize();
        new_sample_available = true;
        __sync_synchronize();
      }
      __enable_irq();
    }

    static void input_test_io_isr_parse(char in_c) {
      switch(in_c) {
      case '!': parser_state = IPS_TYPE; return;
      case '\r': case '\n': input_test_io_isr_parse_reset(); return;
      }

      switch(parser_state) {
      case IPS_IDLE:
        if(in_c == 'P') {
          input_test_io_isr_parse_begin();
        }
        break;
      case IPS_TYPE:
        if(in_c == 'P') {
          input_test_io_isr_parse_begin();
          return;
        }
        break;
      case IPS_SEPARATOR_A: input_test_io_isr_parse_literal(in_c, ' '); break;
      case IPS_MICROS_0: input_test_io_isr_parse_digit(in_c, &parser_micros, 28); break;
      case IPS_MICROS_1: input_test_io_isr_parse_digit(in_c, &parser_micros, 24); break;
      case IPS_MICROS_2: input_test_io_isr_parse_digit(in_c, &parser_micros, 20); break;
      case IPS_MICROS_3: input_test_io_isr_parse_digit(in_c, &parser_micros, 16); break;
      case IPS_MICROS_4: input_test_io_isr_parse_digit(in_c, &parser_micros, 12); break;
      case IPS_MICROS_5: input_test_io_isr_parse_digit(in_c, &parser_micros,  8); break;
      case IPS_MICROS_6: input_test_io_isr_parse_digit(in_c, &parser_micros,  4); break;
      case IPS_MICROS_7: input_test_io_isr_parse_digit(in_c, &parser_micros,  0); break;
      case IPS_SEPARATOR_B: input_test_io_isr_parse_literal(in_c, ':'); break;
      case IPS_STATUS_0: input_test_io_isr_parse_digit(in_c, &parser_status, 4); break;
      case IPS_STATUS_1: input_test_io_isr_parse_digit(in_c, &parser_status, 0); break;
      case IPS_SEPARATOR_C: input_test_io_isr_parse_literal(in_c, ':'); break;
      case IPS_BUTTONS_0: input_test_io_isr_parse_digit(in_c, &parser_buttons, 12); break;
      case IPS_BUTTONS_1: input_test_io_isr_parse_digit(in_c, &parser_buttons,  8); break;
      case IPS_BUTTONS_2: input_test_io_isr_parse_digit(in_c, &parser_buttons,  4); break;
      case IPS_BUTTONS_3: input_test_io_isr_parse_digit(in_c, &parser_buttons,  0); break;
      case IPS_EOL:
      default:
        break;
      }
    }

    static void input_test_io_isr_parse_reset() {
      parser_state = IPS_IDLE;
    }

    static void input_test_io_isr_parse_begin() {
      parser_state = IPS_SEPARATOR_A;
      parser_micros = 0;
      parser_status = 0;
      parser_buttons = 0;
    }

    static void input_test_io_isr_parse_literal(char in_c, char c) {
      if(in_c == c) {
        parser_state = static_cast<InputParserState>(parser_state + 1);
      } else {
        input_test_io_isr_parse_reset();
      }
    }

    static void input_test_io_isr_parse_digit(char in_c, uint32_t * v,
        int pos) {
      if(in_c >= '0' && in_c <= '9') {
        *v |= (in_c - '0') << pos;
        parser_state = static_cast<InputParserState>(parser_state + 1);
      } else if(in_c >= 'A' && in_c <= 'F') {
        *v |= (in_c - 'A' + 10) << pos;
        parser_state = static_cast<InputParserState>(parser_state + 1);
      } else if(in_c >= 'a' && in_c <= 'f') {
        *v |= (in_c - 'a' + 10) << pos;
        parser_state = static_cast<InputParserState>(parser_state + 1);
      } else {
        input_test_io_isr_parse_reset();
        return;
      }
    }
  }

  uint32_t Input::_last_buttons;
  uint32_t Input::_buttons;
  ShortTimeSpan Input::_time_since_last_sample;

  void Input::init() {
    _last_buttons = 0;
    _buttons = 0;

    Debug::trace("Initializing input");

    hardware::test_io_ser.baud(115200);
    hardware::test_io_ser.attach(&input_test_io_isr, Serial::RxIrq);

    bool alive = false;

    for(int i = 0; !alive && i < 20; ++i) {
      hardware::test_io_ser.puts("A0M0V00");
      wait_ms(100);

      for(int j = 0; !alive && j < 20; ++j) {
        hardware::test_io_ser.putc('P');
        wait_ms(10);
        if(new_sample_available) {
          alive = true;
        }
      }
    }

    if(alive) {
      read_buttons();
      _last_buttons = _buttons;
      Debug::tracef("Test input alive, first sample: %08X %04X",
        last_sample_micros, _buttons);
    } else {
      Debug::trace("Test input not responding");
      Debug::pause();
    }
  }

  void Input::read_buttons() {
    bool got_new = new_sample_available;
    if(got_new) {
      _last_buttons = _buttons;
      __disable_irq();
      __sync_synchronize();
      _buttons = 0;
      if((sample_buttons & 0x1000) == 0) {
        _buttons |= B_GREEN;
      }
      if((sample_buttons & 0x2000) == 0) {
        _buttons |= B_RED;
      }
      if((sample_buttons & 0x4000) == 0) {
        _buttons |= B_BLUE;
      }
      if((sample_buttons & 0x8000) == 0) {
        _buttons |= B_PINK;
      }
      _time_since_last_sample = ShortTimeSpan::from_micros(
        static_cast<int32_t>(sample_micros - last_sample_micros));
      last_sample_micros = sample_micros;
      new_sample_available = false;
      __sync_synchronize();
      __enable_irq();
    }
    //Debug::tracef("Buttons: %04X %c", _buttons, got_new ? 'N' : ' ');
    hardware::test_io_ser.putc('P');
  }
}
