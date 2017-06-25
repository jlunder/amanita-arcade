#include "amanita_arcade.h"

#include "aa_game.h"
#include "aa_input.h"
#include "aa_lights.h"


#define AA_MAX_INDENT 16
#define AA_FRAME_MICROS 50000


namespace aa {
  class Program {
  public:
    static void main();

  private:
    static DigitalOut _amber_led;
    static DigitalOut _green_led;
    static DigitalOut _red_led;
    static DigitalOut _blue_led;
  };

  namespace {
    static char const indent_chars[AA_MAX_INDENT] = {
      ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
      ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    };

    static char trace_buf[256];
  }

  namespace hardware {
    Serial debug_ser(PA_2, PA_3); // USART2 -- also accessible via stdio?
    Serial test_io_ser(PB_10, PB_11); // USART3 -- USART1 doesn't work?
    Serial lights_ws2812_ser(PA_0, PA_1); // USART4
  }

  int32_t Debug::_indent_depth;

  DigitalOut Program::_amber_led(LED3);
  DigitalOut Program::_green_led(LED4);
  DigitalOut Program::_red_led(LED5);
  DigitalOut Program::_blue_led(LED6);

  void Debug::pause() {
    // TODO debug breakpoint
    //__BKPT(0);
    trace("paused, any input to resume");
    getchar();
  }

  void Debug::abort() {
    // TODO reboot
    trace("aborting execution");
    for(;;) {
      // wait forever
    }
  }

  void Debug::assert(bool expr, char const * fail_message) {
    if(!expr) {
      error(fail_message);
    }
  }

  void Debug::assertf(bool expr, char const * fail_format, ...) {
    if(!expr) {
      va_list va;
      va_start(va, fail_format);
      verrorf(fail_format, va);
      va_end(va);
    }
  }

  void Debug::vassertf(bool expr, char const * fail_format, va_list va) {
    if(!expr) {
      verrorf(fail_format, va);
    }
  }

  void Debug::trace(char const * message) {
    char const * q = message;
    char const * p;
    for(;;) {
      fwrite(indent_chars,
        _indent_depth < AA_MAX_INDENT ? _indent_depth : AA_MAX_INDENT, 1,
        stdout);
      p = q;
      if(p == message) {
        putchar('>');
      } else {
        putchar(':');
      }
      while(*q != 0 && *q != '\n') {
        ++q;
      }
      fwrite(p, q - p, 1, stdout);
      fputs("\r\n", stdout);
      if(*q == 0) {
        break;
      }
      ++q;
    }
  }

  void Debug::tracef(char const * format, ...) {
    va_list va;
    va_start(va, format);
    vtracef(format, va);
    va_end(va);
  }

  void Debug::vtracef(char const * format, va_list va) {
    vsnprintf(trace_buf, sizeof trace_buf, format, va);
    trace(trace_buf);
  }

  void Debug::error(char const * message) {
    trace(message);
    abort();
  }

  void Debug::errorf(char const * format, ...) {
    va_list va;
    va_start(va, format);
    verrorf(format, va);
    va_end(va);
  }

  void Debug::verrorf(char const * format, va_list va) {
    vtracef(format, va);
    abort();
  }

  void Debug::push_context(char const * name) {
    /*
    fwrite(indent_chars,
      _indent_depth < AA_MAX_INDENT ? _indent_depth : AA_MAX_INDENT, 1,
      stdout);
    putchar('[');
    fputs(name, stdout);
    putchar("]\r\n");
    */
    assertf(AA_AUTO_ASSERT(_indent_depth < AA_MAX_INDENT));
    ++_indent_depth;
  }

  void Debug::pop_context() {
    assertf(AA_AUTO_ASSERT(_indent_depth > 0));
    if(_indent_depth > 0) {
      --_indent_depth;
    }
  }

  void Program::main() {
    mbed::Timer timer;

    // Somehow stdio knows to hook into USART2...?
    hardware::debug_ser.baud(115200);

    // Give external hardware time to wake up... some of it is sloooow
    wait_ms(500);

    Input::init();
    Lights::init();

    timer.start();

    uint32_t last_micros = timer.read_us();
    for(;;) {
      uint32_t micros = timer.read_us();
      uint32_t delta = micros - last_micros;

      if(delta >= AA_FRAME_MICROS) {
        if(delta < AA_FRAME_MICROS * 2) {
          last_micros += AA_FRAME_MICROS;
        } else {
          last_micros = micros;
        }

        LogContext c("frame");
        Lights::update(ShortTimeSpan(delta));
        Game::update(ShortTimeSpan(delta));
      }
    }
  }
}


int main() {
  aa::Program::main();
  return 0;
}
