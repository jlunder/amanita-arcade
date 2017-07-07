#ifndef AMANITA_ARCADE_H
#define AMANITA_ARCADE_H


#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include <mbed.h>


#define AA_AUTO_ASSERT(expr) \
  expr, "%s: %d: expression not true: %s", __FILE__, __LINE__, #expr


namespace aa {
  enum MushroomID {
    M_RED,   // Stalk labeled "1"
    M_GREEN, // Stalk labeled "B"
    M_BLUE,  // Stalk labeled "A"
    M_PINK,  // Stalk labeled "2"
    M_COUNT
  };

  #if 0
  static const size_t STALK_HEIGHT_RED   = 12;
  static const size_t STALK_HEIGHT_GREEN = 12;
  static const size_t STALK_HEIGHT_BLUE  = 12;
  static const size_t STALK_HEIGHT_PINK  = 12;
  static const size_t STALK_HEIGHT_MAX   = 12;
  static const size_t STALK_WIDTH = 1;
  static const size_t SCOREBOARD_WIDTH  = 8;
  static const size_t SCOREBOARD_HEIGHT = 8;
  static const float STALK_BRIGHTNESS = 0.25f;
  static const float SCOREBOARD_BRIGHTNESS = 0.25f;
  #else
  static const size_t STALK_HEIGHT_RED   = 36;
  static const size_t STALK_HEIGHT_GREEN = 41;
  static const size_t STALK_HEIGHT_BLUE  = 32;
  static const size_t STALK_HEIGHT_PINK  = 24;
  static const size_t STALK_HEIGHT_MAX   = 41;
  static const size_t STALK_WIDTH = 3;
  static const size_t SCOREBOARD_WIDTH = 30;
  static const size_t SCOREBOARD_HEIGHT = 30;
  static const float STALK_BRIGHTNESS = 1.0f;
  static const float SCOREBOARD_BRIGHTNESS = 0.5f;
  #endif

  static const size_t STALK_LIGHTS_COUNT_RED =
    STALK_HEIGHT_RED * STALK_WIDTH;
  static const size_t STALK_LIGHTS_COUNT_GREEN =
    STALK_HEIGHT_GREEN * STALK_WIDTH;
  static const size_t STALK_LIGHTS_COUNT_BLUE =
    STALK_HEIGHT_BLUE * STALK_WIDTH;
  static const size_t STALK_LIGHTS_COUNT_PINK =
    STALK_HEIGHT_PINK * STALK_WIDTH;
  static const size_t SCOREBOARD_LIGHTS_COUNT =
    SCOREBOARD_WIDTH * SCOREBOARD_HEIGHT;

  namespace hw {
    extern Serial input_ser;
    extern PortOut lights_ws2812_port;
    extern DigitalOut debug_amber_led;
    extern DigitalOut debug_green_led;
    extern DigitalOut debug_red_led;
    extern DigitalOut debug_blue_led;
    extern DigitalOut debug_frame_sync;
    extern DigitalOut debug_lights_sync;
  }

  class Debug {
  public:
    static void pause();
    static void abort();
    static void assert(bool expr, char const * failMessage);
    static void assertf(bool expr, char const * failFormat, ...);
    static void vassertf(bool expr, char const * failFormat, va_list va);
    static void trace(char const * message);
    static void tracef(char const * format, ...);
    static void vtracef(char const * format, va_list va);
    static void error(char const * message);
    static void errorf(char const * format, ...);
    static void verrorf(char const * format, va_list va);
#ifdef NDEBUG
    static void dev_assert(bool, char const *) {}
    static void dev_assertf(bool, char const *, ...) {}
    static void dev_trace(char const * message) {}
    static void dev_tracef(char const * format, ...) {}
    static void dev_error(char const * message) {}
    static void dev_errorf(char const * format, ...) {}
#else
    static void dev_assert(bool expr, char const * failMessage) {
      assert(expr, failMessage);
    }
    static void dev_assertf(bool expr, char const * failFormat, ...) {
      if(!expr) {
        va_list va;
        va_start(va, failFormat);
        vassertf(expr, failFormat, va);
        va_end(va);
      }
    }
    static void dev_trace(char const * message) { trace(message); }
    static void dev_tracef(char const * format, ...) {
      va_list va;
      va_start(va, format);
      vtracef(format, va);
      va_end(va);
    }
    static void dev_error(char const * message) { error(message); }
    static void dev_errorf(char const * format, ...) {
      va_list va;
      va_start(va, format);
      verrorf(format, va);
      va_end(va);
    }
#endif

    static void push_context(char const * name);
    static void pop_context();

  private:
    static int32_t _indent_depth;
  };

  class LogContext {
  public:
    explicit LogContext(char const * name) { Debug::push_context(name); }
    ~LogContext() { Debug::pop_context(); }
  };

}


#endif // AMANITA_ARCADE_H
