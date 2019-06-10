#include "amanita_arcade.h"

#include "aa_timer.h"


namespace aa {
  Timer::Timer(TimeSpan period, bool periodic)
      : _period(period), _current(), _periods(0), _periodic(periodic) {
    Debug::assertf(AA_AUTO_ASSERT(!periodic || (period > TimeSpan::zero)));
  }


  int32_t Timer::read_periods() {
    int32_t periods = _periods;
    _periods = 0;
    return periods;
  }


  void Timer::update(TimeSpan elapsed) {
    Debug::assertf(AA_AUTO_ASSERT(elapsed >= TimeSpan(0)));

    if(elapsed < _period - _current) {
      _current += elapsed;
    }
    else if(_periodic) {
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
    else {
      _current = _period;
      _periods = 1;
    }
  }


  void Timer::cancel() {
    _periods = 1;
    _current = _period;
  }


  void Timer::restart() {
    _periods = 0;
    _current = TimeSpan::zero;
  }
}
