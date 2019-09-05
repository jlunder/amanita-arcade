#ifndef DEBUG_DEBUG_H
#define DEBUG_DEBUG_H

#include <stdint.h>
#include <stdarg.h>

#define DEBUG_STRINGIFY(x) #x
#define DEBUG_TOSTRING(x) DEBUG_STRINGIFY(x)
#if defined __PRETTY_FUNCTION__
#define DEBUG_ORIGIN __FILE__ ":" DEBUG_TOSTRING(__LINE__) ": in " __PRETTY_FUNCTION__
#elif defined __FUNCTION__
#define DEBUG_ORIGIN __FILE__ ":" DEBUG_TOSTRING(__LINE__) ": in " __FUNCTION__
#else
#define DEBUG_ORIGIN __FILE__ ":" DEBUG_TOSTRING(__LINE__)
#endif

#define auto_assert(expr) \
  assert(expr, DEBUG_ORIGIN ": assertion not true: " #expr)
#define auto_check(expr) \
  check(expr, DEBUG_ORIGIN ": check not true: " #expr)
#define auto_error(msg) \
  error(DEBUG_ORIGIN ": error: " #msg)

#ifdef NDEBUG
#define dev_auto_assert(expr) auto_assert(true)
#define dev_auto_check(expr) auto_check(true)
#define dev_auto_error(msg) auto_error("")
#else
#define dev_auto_assert(expr) auto_assert(expr)
#define dev_auto_check(expr) auto_check(expr)
#define dev_auto_error(msg) auto_error(msg)
#endif


class Debug {
public:
  static void pause();
  static void abort();
  static void assert(bool expr, char const * failMessage);
  static void assertf(bool expr, char const * failFormat, ...);
  static void vassertf(bool expr, char const * failFormat, va_list va);
  static void check(bool expr, char const * failMessage);
  static void checkf(bool expr, char const * failFormat, ...);
  static void vcheckf(bool expr, char const * failFormat, va_list va);
  static void trace(char const * message);
  static void tracef(char const * format, ...);
  static void vtracef(char const * format, va_list va);
  static void error(char const * message);
  static void errorf(char const * format, ...);
  static void verrorf(char const * format, va_list va);
#ifdef NDEBUG
  static void dev_assert(bool, char const *) {}
  static void dev_assertf(bool, char const *, ...) {}
  static void dev_check(bool, char const *) {}
  static void dev_checkf(bool, char const *, ...) {}
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
  static void dev_check(bool expr, char const * failMessage) {
    check(expr, failMessage);
  }
  static void dev_checkf(bool expr, char const * failFormat, ...) {
    if(!expr) {
      va_list va;
      va_start(va, failFormat);
      vcheckf(expr, failFormat, va);
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

  static void service_watchdog();
  static bool in_available();
  static int in_read_nb(); // non-blocking
  static char in_read();
};

class LogContext {
public:
  explicit LogContext(char const * name) { Debug::push_context(name); }
  ~LogContext() { Debug::pop_context(); }
};

#endif
