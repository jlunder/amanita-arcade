#include "Debug.h"

#include <mbed.h>

#include <stdio.h>
#include <stdlib.h>


#define DB_MAX_INDENT 16


static int32_t indent_depth;
static char const indent_chars[DB_MAX_INDENT] = {
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
};
static char trace_buf[256];


void __attribute__((weak)) Debug::service_watchdog() {
  // Do nothing
}


void __attribute__((weak)) Debug::pause() {
  // TODO debug breakpoint
  trace("paused, any input to resume");
  for(;;) {
    service_watchdog();
    if(in_available()) {
      in_read_nb();
      break;
    }
  }
}


void __attribute__((weak)) Debug::abort() {
  trace("aborting execution");
  NVIC_SystemReset();
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


void Debug::check(bool expr, char const * fail_message) {
  if(!expr) {
    trace(fail_message);
  }
}


void Debug::checkf(bool expr, char const * fail_format, ...) {
  if(!expr) {
    va_list va;
    va_start(va, fail_format);
    vtracef(fail_format, va);
    va_end(va);
  }
}


void Debug::vcheckf(bool expr, char const * fail_format, va_list va) {
  if(!expr) {
    vtracef(fail_format, va);
  }
}


void Debug::trace(char const * message) {
  char const * q = message;
  char const * p;
  for(;;) {
    fwrite(indent_chars,
      indent_depth < DB_MAX_INDENT ? indent_depth : DB_MAX_INDENT, 1,
      stdout);
    p = q;
    if(p == message) {
      putchar('>');
    }
    else {
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
    _indent_depth < DB_MAX_INDENT ? _indent_depth : DB_MAX_INDENT, 1,
    stdout);
  putchar('[');
  fputs(name, stdout);
  putchar("]\r\n");
  */
  auto_assert(indent_depth < DB_MAX_INDENT);
  ++indent_depth;
}


void Debug::pop_context() {
  auto_assert(indent_depth > 0);
  if(indent_depth > 0) {
    --indent_depth;
  }
}


bool Debug::in_available() {
  FileHandle * f = mbed::mbed_override_console(STDIN_FILENO);

  return (f != NULL) && f->readable();
}


int Debug::in_read_nb() {
  if(in_available()) {
    return getc(stdin);
  }
  else {
    return -1;
  }
}


char Debug::in_read() {
  return getc(stdin);
}


