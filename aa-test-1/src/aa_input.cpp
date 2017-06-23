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
          } else {
            parser_state = IPS_IDLE;
          }
        }
      }

      __disable_irq();
      __sync_synchronize();
      if(!new_sample_available && (parser_state == IPS_EOL)) {
        sample_micros = parser_micros;
        sample_buttons = parser_buttons;
        parser_state = IPS_IDLE;
      }
      __sync_synchronize();
      __enable_irq();
      new_sample_available = true;
    }

    static void input_test_io_isr_parse(char in_c) {
      switch(in_c) {
      case '!': input_test_io_isr_parse_begin(); return;
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
    hardware::test_io_ser.puts("A0M0V00");

    mbed::Timer tm;
    tm.start();
    while(tm.read_ms() < 30) {
      if(hardware::test_io_ser.readable()) {
        hardware::test_io_ser.getc();
      }
    }

    hardware::test_io_ser.putc('P');
    tm.reset();
    bool alive = false;
    while(tm.read_ms() < 100) {
      if(new_sample_available) {
        alive = true;
        break;
      }
    }

    if(alive) {
      read_buttons();
      _last_buttons = _buttons;
      Debug::tracef("Test input alive, first sample: %08X %04X",
        last_sample_micros, _buttons);
    } else {
      Debug::trace("Test input not responding");
    }
  }

  void Input::read_buttons() {
    if(new_sample_available) {
      _last_buttons = _buttons;
      __sync_synchronize();
      __disable_irq();
      _buttons = sample_buttons;
      _time_since_last_sample = ShortTimeSpan::from_micros(
        static_cast<int32_t>(sample_micros - last_sample_micros));
      last_sample_micros = sample_micros;
      new_sample_available = false;
      __enable_irq();
      __sync_synchronize();
    }
  }
}
