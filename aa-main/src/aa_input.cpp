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
    static int32_t const REMOTE_READ_TIMEOUT_MICROS = 100000; // 100ms
    static int32_t const REMOTE_QUERY_THROTTLE_TIMEOUT_MILLIS = 10; // 10ms
    static int32_t const REMOTE_QUERY_MAX_MISSED_SAMPLES = 100; // 10ms

    static uint32_t last_sample_micros;
    static uint32_t missed_samples = 0;

    static InputParserState parser_state;
    static uint32_t parser_micros;
    static uint32_t parser_status;
    static uint32_t parser_buttons;

    static uint8_t remote_io_thread_stack[1024];
    static Thread remote_io_thread(osPriorityNormal,
      sizeof remote_io_thread_stack, remote_io_thread_stack, "IO");

    static Mutex remote_io_sample_mutex;
    static bool remote_io_is_alive = false;
    static uint32_t remote_io_sample_micros;
    static uint32_t remote_io_sample_buttons;

    static ConditionVariable remote_io_want_new_sample(remote_io_sample_mutex);
    static ConditionVariable remote_io_new_sample_available(remote_io_sample_mutex);


    static void remote_io_thread_main();
    static void remote_io_thread_parse(char in_c);
    static void remote_io_thread_parse_reset();
    static void remote_io_thread_parse_begin();
    static void remote_io_thread_parse_literal(char in_c, char c);
    static void remote_io_thread_parse_digit(char in_c, uint32_t * v,
        int pos);


    static void remote_io_thread_main() {
      Debug::tracef("Input thread started");
      for(;;) {
        remote_io_sample_mutex.lock();
        remote_io_want_new_sample.wait();
        remote_io_sample_mutex.unlock();
        //Debug::tracef("Input thread acquiring sample");

        if(!remote_io_is_alive) {
          hw::input_ser.write("A0M0V000\n", 9);
        }
        hw::input_ser.write("P\n", 2);

        mbed::Timer response_timeout;
        response_timeout.start();

        for(;;) {
          if(response_timeout.read_high_resolution_us()
              >= REMOTE_READ_TIMEOUT_MICROS) {
            Debug::tracef("Input thread timeout");
            remote_io_sample_mutex.lock();
            remote_io_is_alive = false;
            remote_io_new_sample_available.notify_one();
            remote_io_sample_mutex.unlock();
            break;
          }
          if(hw::input_ser.readable()) {
            char c;
            hw::input_ser.read(&c, 1);
            remote_io_thread_parse(c);
            if(parser_state == IPS_EOL) {
              //Debug::tracef("Input thread finished parsing");
              remote_io_sample_mutex.lock();
              remote_io_sample_micros = parser_micros;
              remote_io_sample_buttons = parser_buttons;
              parser_state = IPS_IDLE;
              remote_io_is_alive = true;
              remote_io_new_sample_available.notify_one();
              remote_io_sample_mutex.unlock();
              ThisThread::sleep_for(REMOTE_QUERY_THROTTLE_TIMEOUT_MILLIS);
              break;
            }
          }
          else {
            ThisThread::sleep_for(1);
          }
        }
      }
    }


    static void remote_io_thread_parse(char in_c) {
      switch(in_c) {
      case '!': parser_state = IPS_TYPE; return;
      case '\r': case '\n': remote_io_thread_parse_reset(); return;
      }

      switch(parser_state) {
      case IPS_IDLE:
      case IPS_TYPE:
        if(in_c == 'P') {
          remote_io_thread_parse_begin();
          return;
        }
        break;
      case IPS_SEPARATOR_A: remote_io_thread_parse_literal(in_c, ' '); break;
      case IPS_MICROS_0: remote_io_thread_parse_digit(in_c, &parser_micros, 28); break;
      case IPS_MICROS_1: remote_io_thread_parse_digit(in_c, &parser_micros, 24); break;
      case IPS_MICROS_2: remote_io_thread_parse_digit(in_c, &parser_micros, 20); break;
      case IPS_MICROS_3: remote_io_thread_parse_digit(in_c, &parser_micros, 16); break;
      case IPS_MICROS_4: remote_io_thread_parse_digit(in_c, &parser_micros, 12); break;
      case IPS_MICROS_5: remote_io_thread_parse_digit(in_c, &parser_micros,  8); break;
      case IPS_MICROS_6: remote_io_thread_parse_digit(in_c, &parser_micros,  4); break;
      case IPS_MICROS_7: remote_io_thread_parse_digit(in_c, &parser_micros,  0); break;
      case IPS_SEPARATOR_B: remote_io_thread_parse_literal(in_c, ':'); break;
      case IPS_STATUS_0: remote_io_thread_parse_digit(in_c, &parser_status, 4); break;
      case IPS_STATUS_1: remote_io_thread_parse_digit(in_c, &parser_status, 0); break;
      case IPS_SEPARATOR_C: remote_io_thread_parse_literal(in_c, ':'); break;
      case IPS_BUTTONS_0: remote_io_thread_parse_digit(in_c, &parser_buttons, 12); break;
      case IPS_BUTTONS_1: remote_io_thread_parse_digit(in_c, &parser_buttons,  8); break;
      case IPS_BUTTONS_2: remote_io_thread_parse_digit(in_c, &parser_buttons,  4); break;
      case IPS_BUTTONS_3: remote_io_thread_parse_digit(in_c, &parser_buttons,  0); break;
      case IPS_EOL:
      default:
        break;
      }
    }


    static void remote_io_thread_parse_reset() {
      parser_state = IPS_IDLE;
    }


    static void remote_io_thread_parse_begin() {
      parser_state = IPS_SEPARATOR_A;
      parser_micros = 0;
      parser_status = 0;
      parser_buttons = 0;
    }


    static void remote_io_thread_parse_literal(char in_c, char c) {
      if(in_c == c) {
        parser_state = static_cast<InputParserState>(parser_state + 1);
      }
      else {
        remote_io_thread_parse_reset();
      }
    }


    static void remote_io_thread_parse_digit(char in_c, uint32_t * v,
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
        remote_io_thread_parse_reset();
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
    //hw::input_ser.attach(&remote_io_thread_isr, UARTSerial::RxIrq);
    remote_io_thread.start(&remote_io_thread_main);
  }


  bool Input::connected() {
    return remote_io_is_alive;
  }


  void Input::read_buttons(ShortTimeSpan dt) {
    _remote_dt = TimeSpan::zero;

    uint32_t remote_buttons = 0;

    remote_io_sample_mutex.lock();
    bool got_new_sample = remote_io_new_sample_available.wait_for(0);
    if(got_new_sample) {
      // Only service the watchdog if input thread is responding -- if the
      // thread falls over something went dramatically wrong with the
      // software and we want to reset!
      System::service_watchdog();

      if(remote_io_is_alive) {
        if((remote_io_sample_buttons & 0x1000) == 0) {
          remote_buttons |= B_GREEN;
        }
        if((remote_io_sample_buttons & 0x2000) == 0) {
          remote_buttons |= B_RED;
        }
        if((remote_io_sample_buttons & 0x4000) == 0) {
          remote_buttons |= B_BLUE;
        }
        if((remote_io_sample_buttons & 0x8000) == 0) {
          remote_buttons |= B_PINK;
        }
        _remote_dt = ShortTimeSpan::from_micros(
          static_cast<int32_t>(remote_io_sample_micros - last_sample_micros));
        last_sample_micros = remote_io_sample_micros;
      }
    }
    else {
      ++missed_samples;
      if(missed_samples >= REMOTE_QUERY_MAX_MISSED_SAMPLES) {
        Debug::tracef("Input thread not responding");
        missed_samples = 0;
      }
    }
    // Ask for a new sample even if we didn't get one, just in case there's a
    // timing issue it doesn't hurt
    remote_io_want_new_sample.notify_one();
    //Debug::tracef("Buttons: %04X %c", _buttons, got_new ? 'N' : ' ');
    remote_io_sample_mutex.unlock();

    _last_buttons = _buttons;

    _buttons = remote_buttons | _debug_buttons;
    _debug_buttons = 0;
  }

  void Input::debug_sim_buttons(uint32_t debug_buttons) {
    _debug_buttons |= debug_buttons;
  }
}
