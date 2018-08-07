#ifndef AA_TIME_SPAN_H
#define AA_TIME_SPAN_H


#include "amanita_arcade.h"


namespace aa {
  class ShortTimeSpan;
  class TimeSpan;

  class ZeroTimeSpan {
  public:
    float to_seconds() const { return 0.0f; }
    int32_t to_millis() const { return 0; }
    int32_t to_micros() const { return 0; }

    operator ShortTimeSpan() const;
    operator TimeSpan() const;
  };

  class InfinityTimeSpan {
  public:
    float to_seconds() const { return INFINITY; }
    int32_t to_millis() const { return INT32_MAX; }
    int32_t to_micros() const { return INT32_MAX; }

    operator ShortTimeSpan() const;
    operator TimeSpan() const;
  };

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

    ShortTimeSpan & operator +=(ShortTimeSpan y) {
      _micros += y._micros;
      return *this;
    }

    ShortTimeSpan & operator -=(ShortTimeSpan y) {
      _micros -= y._micros;
      return *this;
    }

    ShortTimeSpan & operator *=(int32_t y) {
      _micros *= y;
      return *this;
    }

    ShortTimeSpan & operator /=(int32_t y) {
      _micros /= y;
      return *this;
    }

    ShortTimeSpan & operator %=(ShortTimeSpan y) {
      _micros %= y._micros;
      return *this;
    }

    operator TimeSpan() const;

  private:
    int32_t _micros;
  };

  class TimeSpan {
  public:
    static ZeroTimeSpan const zero;
    static InfinityTimeSpan const infinity;

    TimeSpan(): _micros() { }
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

    TimeSpan & operator +=(TimeSpan other) {
      _micros += other._micros;
      return *this;
    }

    TimeSpan & operator -=(TimeSpan other) {
      _micros -= other._micros;
      return *this;
    }

    TimeSpan & operator *=(int64_t y) {
      _micros *= y;
      return *this;
    }

    TimeSpan & operator /=(int64_t y) {
      _micros /= y;
      return *this;
    }

    TimeSpan & operator %=(TimeSpan y) {
      _micros %= y._micros;
      return *this;
    }

  private:
    int64_t _micros;
  };

  inline ZeroTimeSpan::operator ShortTimeSpan() const {
    return ShortTimeSpan();
  }

  inline ZeroTimeSpan::operator TimeSpan() const {
    return TimeSpan();
  }

  inline InfinityTimeSpan::operator ShortTimeSpan() const {
    return ShortTimeSpan(INT32_MAX);
  }

  inline InfinityTimeSpan::operator TimeSpan() const {
    return TimeSpan(INT64_MAX);
  }

  inline ShortTimeSpan::operator TimeSpan() const {
    return TimeSpan(_micros);
  }

  inline ShortTimeSpan operator +(ShortTimeSpan x, ShortTimeSpan y) {
    return ShortTimeSpan(x.to_micros() + y.to_micros());
  }

  inline ShortTimeSpan operator -(ShortTimeSpan x) {
    return ShortTimeSpan(-x.to_micros());
  }

  inline ShortTimeSpan operator -(ShortTimeSpan x, ShortTimeSpan y) {
    return ShortTimeSpan(x.to_micros() - y.to_micros());
  }

  inline ShortTimeSpan operator *(ShortTimeSpan x, int32_t y) {
    return ShortTimeSpan(x.to_micros() * y);
  }

  inline ShortTimeSpan operator *(int32_t x, ShortTimeSpan y) {
    return ShortTimeSpan(x * y.to_micros());
  }

  inline int64_t operator /(ShortTimeSpan x, ShortTimeSpan y) {
    return (int64_t)x.to_micros() * 1000000 / y.to_micros();
  }

  inline ShortTimeSpan operator /(ShortTimeSpan x, int32_t y) {
    return ShortTimeSpan::from_micros(x.to_micros() / y);
  }

  inline ShortTimeSpan operator %(ShortTimeSpan x, ShortTimeSpan y) {
    return ShortTimeSpan(x.to_micros() % y.to_micros());
  }

  inline bool operator <(ShortTimeSpan x, ShortTimeSpan y) {
    return x.to_micros() < y.to_micros();
  }

  inline bool operator <=(ShortTimeSpan x, ShortTimeSpan y) {
    return x.to_micros() <= y.to_micros();
  }

  inline bool operator >(ShortTimeSpan x, ShortTimeSpan y) {
    return x.to_micros() > y.to_micros();
  }

  inline bool operator >=(ShortTimeSpan x, ShortTimeSpan y) {
    return x.to_micros() >= y.to_micros();
  }

  inline bool operator ==(ShortTimeSpan x, ShortTimeSpan y) {
    return x.to_micros() == y.to_micros();
  }

  inline bool operator !=(ShortTimeSpan x, ShortTimeSpan y) {
    return x.to_micros() != y.to_micros();
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
    return x.to_micros() * 1000000 / y.to_micros();
  }

  inline TimeSpan operator /(TimeSpan x, int64_t y) {
    return TimeSpan::from_micros(x.to_micros() / y);
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
