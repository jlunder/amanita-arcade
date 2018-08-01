#ifndef AA_TIMER_H
#define AA_TIMER_H


#include "amanita_arcade.h"

#include "aa_time_span.h"


namespace aa {
  class Timer {
  public:
    Timer(TimeSpan period, bool periodic = true);

    TimeSpan get_period() const { return _period; }
    int32_t peek_periods() const { return _periods; }
    int32_t read_periods();
    TimeSpan get_time() const { return _current; }
    TimeSpan get_time_remaining() const { return _period - _current; }
    bool is_done() const { return _periods > 0; }
    void update(TimeSpan elapsed);
    void cancel();
    void restart();

  private:
    TimeSpan _period;
    TimeSpan _current;
    int32_t _periods;
    bool _periodic;
  };

}


#endif // AA_TIMER_H
