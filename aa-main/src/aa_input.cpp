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
    static int32_t const REMOTE_READ_TIMEOUT_MICROS = 10000000; // 10s
    static int32_t const REMOTE_QUERY_THROTTLE_TIMEOUT_MICROS = 10000; // 10ms

    static Timer remote_read_timeout(
      ShortTimeSpan::from_micros(REMOTE_READ_TIMEOUT_MICROS));
    static Timer remote_query_throttle_timeout(
      ShortTimeSpan::from_micros(REMOTE_QUERY_THROTTLE_TIMEOUT_MICROS));
    static uint32_t remote_buttons;

    static bool got_new_sample;
    static bool new_sample_available;
    static uint32_t sample_micros;
    static uint32_t sample_buttons;

    static uint32_t last_sample_micros;

    static InputParserState parser_state;
    static uint32_t parser_micros;
    static uint32_t parser_status;
    static uint32_t parser_buttons;

    static void remote_io_isr();
    static void remote_io_isr_parse(char in_c);
    static void remote_io_isr_parse_reset();
    static void remote_io_isr_parse_begin();
    static void remote_io_isr_parse_literal(char in_c, char c);
    static void remote_io_isr_parse_digit(char in_c, uint32_t * v,
        int pos);


    static void remote_io_isr() {
      for(int i = 0; i < 10; ++i) {
        while(hw::input_ser.readable()) {
          int c = hw::input_ser.getc();
          remote_io_isr_parse((char)c);
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


    static void remote_io_isr_parse(char in_c) {
      switch(in_c) {
      case '!': parser_state = IPS_TYPE; return;
      case '\r': case '\n': remote_io_isr_parse_reset(); return;
      }

      switch(parser_state) {
      case IPS_IDLE:
      case IPS_TYPE:
        if(in_c == 'P') {
          remote_io_isr_parse_begin();
          return;
        }
        break;
      case IPS_SEPARATOR_A: remote_io_isr_parse_literal(in_c, ' '); break;
      case IPS_MICROS_0: remote_io_isr_parse_digit(in_c, &parser_micros, 28); break;
      case IPS_MICROS_1: remote_io_isr_parse_digit(in_c, &parser_micros, 24); break;
      case IPS_MICROS_2: remote_io_isr_parse_digit(in_c, &parser_micros, 20); break;
      case IPS_MICROS_3: remote_io_isr_parse_digit(in_c, &parser_micros, 16); break;
      case IPS_MICROS_4: remote_io_isr_parse_digit(in_c, &parser_micros, 12); break;
      case IPS_MICROS_5: remote_io_isr_parse_digit(in_c, &parser_micros,  8); break;
      case IPS_MICROS_6: remote_io_isr_parse_digit(in_c, &parser_micros,  4); break;
      case IPS_MICROS_7: remote_io_isr_parse_digit(in_c, &parser_micros,  0); break;
      case IPS_SEPARATOR_B: remote_io_isr_parse_literal(in_c, ':'); break;
      case IPS_STATUS_0: remote_io_isr_parse_digit(in_c, &parser_status, 4); break;
      case IPS_STATUS_1: remote_io_isr_parse_digit(in_c, &parser_status, 0); break;
      case IPS_SEPARATOR_C: remote_io_isr_parse_literal(in_c, ':'); break;
      case IPS_BUTTONS_0: remote_io_isr_parse_digit(in_c, &parser_buttons, 12); break;
      case IPS_BUTTONS_1: remote_io_isr_parse_digit(in_c, &parser_buttons,  8); break;
      case IPS_BUTTONS_2: remote_io_isr_parse_digit(in_c, &parser_buttons,  4); break;
      case IPS_BUTTONS_3: remote_io_isr_parse_digit(in_c, &parser_buttons,  0); break;
      case IPS_EOL:
      default:
        break;
      }
    }


    static void remote_io_isr_parse_reset() {
      parser_state = IPS_IDLE;
    }


    static void remote_io_isr_parse_begin() {
      parser_state = IPS_SEPARATOR_A;
      parser_micros = 0;
      parser_status = 0;
      parser_buttons = 0;
    }


    static void remote_io_isr_parse_literal(char in_c, char c) {
      if(in_c == c) {
        parser_state = static_cast<InputParserState>(parser_state + 1);
      }
      else {
        remote_io_isr_parse_reset();
      }
    }


    static void remote_io_isr_parse_digit(char in_c, uint32_t * v,
        int pos) {
      if(in_c >= '0' && in_c <= '9') {
        *v |= (in_c - '0') << pos;
        parser_state = static_cast<InputParserState>(parser_state + 1);
      }
      else if(in_c >= 'A' && in_c <= 'F') {
        *v |= (in_c - 'A' + 10) << pos;
        parser_state = static_cast<InputParserState>(parser_state + 1);
      }
      else if(in_c >= 'a' && in_c <= 'f') {
        *v |= (in_c - 'a' + 10) << pos;
        parser_state = static_cast<InputParserState>(parser_state + 1);
      }
      else {
        remote_io_isr_parse_reset();
        return;
      }
    }
  }


  ShortTimeSpan Input::_remote_dt;
  uint32_t Input::_last_buttons;
  uint32_t Input::_buttons;
  uint32_t Input::_debug_buttons;


  void Input::init() {
    _remote_dt = TimeSpan::zero;
    _last_buttons = 0;
    _buttons = 0;

    Debug::trace("Initializing input");

    hw::input_ser.baud(115200);
    hw::input_ser.attach(&remote_io_isr, Serial::RxIrq);

    bool alive = false;

    for(int i = 0; !alive && i < 20; ++i) {
      hw::input_ser.puts("A0M0V00");
      wait_ms(100);
      System::service_watchdog();

      for(int j = 0; !alive && j < 20; ++j) {
        hw::input_ser.putc('P');
        wait_ms(10);
        System::service_watchdog();
        if(new_sample_available) {
          alive = true;
        }
      }
    }

    if(alive) {
      read_buttons(TimeSpan::zero);
      _last_buttons = _buttons;
      Debug::tracef("Input module alive, first sample: %08X %04X",
        last_sample_micros, _buttons);
    }
    else {
      Debug::trace("Input module not responding");
    }
  }


  bool Input::connected() {
    return got_new_sample;
  }


  void Input::read_buttons(ShortTimeSpan dt) {
    remote_read_timeout.update(dt);
    remote_query_throttle_timeout.update(dt);

    _remote_dt = TimeSpan::zero;

    if(remote_query_throttle_timeout.is_done()) {
      read_remote_buttons();
    }

    _last_buttons = _buttons;

    _buttons = remote_buttons | _debug_buttons;
    _debug_buttons = 0;
  }

  void Input::debug_sim_buttons(uint32_t debug_buttons) {
    _debug_buttons |= debug_buttons;
  }

  void Input::read_remote_buttons()
  {
    __sync_synchronize();
    got_new_sample = new_sample_available;
    if(got_new_sample) {
      __disable_irq();
      remote_buttons = 0;
      if((sample_buttons & 0x1000) == 0) {
        remote_buttons |= B_GREEN;
      }
      if((sample_buttons & 0x2000) == 0) {
        remote_buttons |= B_RED;
      }
      if((sample_buttons & 0x4000) == 0) {
        remote_buttons |= B_BLUE;
      }
      if((sample_buttons & 0x8000) == 0) {
        remote_buttons |= B_PINK;
      }
      _remote_dt = ShortTimeSpan::from_micros(
        static_cast<int32_t>(sample_micros - last_sample_micros));
      last_sample_micros = sample_micros;
      new_sample_available = false;
      __enable_irq();

      hw::input_ser.putc('P');
      remote_read_timeout.restart();
      remote_query_throttle_timeout = Timer(TimeSpan::from_micros(
        REMOTE_QUERY_THROTTLE_TIMEOUT_MICROS), false);
    }
    else {
      // Hmmm, input not responding?
      hw::input_ser.putc('R');
      hw::input_ser.puts("A0M0V00");
      wait_ms(10);
      hw::input_ser.putc('P');

      remote_query_throttle_timeout =
        Timer(TimeSpan::from_millis(100), false);

      // If input persists in not responding for a LONG time, unstick any
      // buttons automatically which were pressed at the time we lost contact
      if(remote_read_timeout.is_done()) {
        remote_buttons = 0;
      }

      Debug::tracef("Input not responding, resetting");
    }
    //Debug::tracef("Buttons: %04X %c", _buttons, got_new ? 'N' : ' ');
  }
}
