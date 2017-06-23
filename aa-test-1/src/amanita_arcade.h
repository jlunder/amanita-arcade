#ifndef AMANITA_ARCADE_H
#define AMANITA_ARCADE_H


#include <stdarg.h>
#include <stdint.h>

#include <mbed.h>


#define AA_AUTO_ASSERT(expr) \
  expr, "%s: %d: expression not true: %s", __FILE__, __LINE__, #expr


namespace AA {
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

  class TimeSpan {
  public:
    explicit TimeSpan(int64_t micros): _micros(micros) { }

    float to_seconds() const { return _micros * 1e-6f; }
    int64_t to_millis() const { return _micros / 1000; }
    int64_t to_micros() const { return _micros; }

    static TimeSpan from_micros(int64_t micros) { return TimeSpan(micros); }

    static TimeSpan from_millis(int64_t millis) {
      return TimeSpan(millis * 1000);
    }

    static TimeSpan from_seconds(float seconds) {
      return TimeSpan(seconds * 1e6f);
    }

  private:
    int64_t _micros;
  };

  class ShortTimeSpan {
  public:
    explicit ShortTimeSpan(int32_t micros): _micros(micros) { }

    float to_seconds() const { return _micros * 1e-6f; }
    int32_t to_millis() const { return _micros / 1000; }
    int32_t to_micros() const { return _micros; }

    static ShortTimeSpan from_micros(int32_t micros) {
      return ShortTimeSpan(micros);
    }

    static ShortTimeSpan from_millis(int32_t millis) {
      return ShortTimeSpan(millis * 1000);
    }

    static ShortTimeSpan from_seconds(float seconds) {
      return ShortTimeSpan(seconds);
    }

    operator TimeSpan() const;

  private:
    int32_t _micros;
  };

  inline ShortTimeSpan::operator TimeSpan() const {
    return TimeSpan(_micros);
  }

}


#endif // AMANITA_ARCADE_H
