#include "amanita_arcade.h"


#define AA_MAX_INDENT 16


namespace AA {
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

    static Serial debug_ser(PA_2, PA_3);
    static Serial test_io_ser(PA_9, PA_10);
    static SPI stalk_lights_spi();

    static char trace_buf[256];
  }

  int32_t Debug::_indent_depth;

  DigitalOut Program::_amber_led(LED3);
  DigitalOut Program::_green_led(LED4);
  DigitalOut Program::_red_led(LED5);
  DigitalOut Program::_blue_led(LED6);

  void Debug::pause() {
    // TODO debug breakpoint
    while(debug_ser.readable()) {
      debug_ser.getc();
    }
    trace("paused, any input to resume");
    for(;;) {
      if(debug_ser.readable()) {
        debug_ser.getc();
        break;
      }
    }
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
      static_cast<FileLike *>(&debug_ser)->write(indent_chars,
        _indent_depth < AA_MAX_INDENT ? _indent_depth : AA_MAX_INDENT);
      p = q;
      if(p == message) {
        debug_ser.putc('>');
      } else {
        debug_ser.putc(':');
      }
      while(*q != 0 && *q != '\n') {
        ++q;
      }
      static_cast<FileLike *>(&debug_ser)->write(p, q - p);
      debug_ser.puts("\r\n");
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
    static_cast<FileLike *>(&debug_ser)->write(indent_chars,
      _indent_depth < AA_MAX_INDENT ? _indent_depth : AA_MAX_INDENT);
    debug_ser.putc('[');
    debug_ser.puts(name);
    debug_ser.puts("]\r\n");
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
    debug_ser.baud(115200);
    test_io_ser.baud(115200);

    LogContext c("main");
    for(;;) {
      Debug::tracef("Hello World!!!\nThis message is multiline.");
      _amber_led = 1;
      wait(0.2);
      _amber_led = 0;
      wait(0.2);
    }
  }
}


int main() {
  AA::Program::main();
  return 0;
}
