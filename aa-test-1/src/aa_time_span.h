#ifndef AA_TIME_SPAN_H
#define AA_TIME_SPAN_H


#include "amanita_arcade.h"


namespace AA {
  class TimeSpan;

  class ShortTimeSpan {
  public:
    ShortTimeSpan(): _micros() { }
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

  class TimeSpan {
  public:
    TimeSpan(): _micros() { }
    explicit TimeSpan(int64_t micros): _micros(micros) { }
    TimeSpan(ShortTimeSpan ts): _micros(ts.to_micros()) { }

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

    TimeSpan & operator +=(TimeSpan other) {
      _micros += other._micros;
      return *this;
    }

    TimeSpan & operator -=(TimeSpan other) {
      _micros -= other._micros;
      return *this;
    }

  private:
    int64_t _micros;
  };

  inline ShortTimeSpan::operator TimeSpan() const {
    return TimeSpan(_micros);
  }

  inline TimeSpan operator +(TimeSpan x, TimeSpan y) {
    return TimeSpan(x.to_micros() + y.to_micros());
  }

  inline TimeSpan operator -(TimeSpan x) {
    return TimeSpan(-x.to_micros());
  }

  inline TimeSpan operator -(TimeSpan x, TimeSpan y) {
    return TimeSpan(x.to_micros() - y.to_micros());
  }

  inline TimeSpan operator *(TimeSpan x, int64_t y) {
    return TimeSpan(x.to_micros() * y);
  }

  inline TimeSpan operator *(int64_t x, TimeSpan y) {
    return TimeSpan(x * y.to_micros());
  }

  inline int64_t operator /(TimeSpan x, TimeSpan y) {
    return x.to_micros() / y.to_micros();
  }

  inline TimeSpan operator %(TimeSpan x, TimeSpan y) {
    return TimeSpan(x.to_micros() % y.to_micros());
  }

  inline bool operator <(TimeSpan x, TimeSpan y) {
    return x.to_micros() < y.to_micros();
  }

  inline bool operator <=(TimeSpan x, TimeSpan y) {
    return x.to_micros() <= y.to_micros();
  }

  inline bool operator >(TimeSpan x, TimeSpan y) {
    return x.to_micros() > y.to_micros();
  }

  inline bool operator >=(TimeSpan x, TimeSpan y) {
    return x.to_micros() >= y.to_micros();
  }

  inline bool operator ==(TimeSpan x, TimeSpan y) {
    return x.to_micros() == y.to_micros();
  }

  inline bool operator !=(TimeSpan x, TimeSpan y) {
    return x.to_micros() != y.to_micros();
  }

}


#endif // AA_TIME_SPAN_H
