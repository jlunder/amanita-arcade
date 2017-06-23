#include "amanita_arcade.h"

#include "aa_timer.h"


namespace AA {
  Timer::Timer(TimeSpan period, bool periodic)
      : _period(period), _current(), _periodic(periodic) {
  }

  int32_t Timer::read_periods() {
    int32_t periods = _periods;
    _periods = 0;
    return periods;
  }

  TimeSpan Timer::get_time() const {
    return _current;
  }

  TimeSpan Timer::get_time_remaining() const {
    return _period - _current;
  }

  void Timer::update(TimeSpan elapsed) {
    int64_t micros_elapsed = elapsed.to_micros();

    Debug::assertf(AA_AUTO_ASSERT(elapsed >= TimeSpan(0)));

    if(_periodic) {
      if(elapsed > (_period - _current)) {
        // Add one period -- probably we haven't overflowed and this will avoid
        // division. Plus, it deals with an edge case where elapsed + _current
        // overflows arithmetically
        _current += elapsed;
        _current -= _period;
        ++_periods;
        if(_current > _period) {
          _periods += static_cast<int32_t>(_current / _period);
          _current = _current % _period;
        }
      }
    } else {
      _current += elapsed;
    }
  }
}
