#ifndef AA_TIMER_H
#define AA_TIMER_H


#include "amanita_arcade.h"

#include "aa_time_span.h"


namespace aa {
  class Timer {
  public:
    Timer(TimeSpan period, bool periodic = true);

    int32_t peek_periods() const { return _periods; }
    int32_t read_periods();
    TimeSpan get_time() const;
    TimeSpan get_time_remaining() const;

    void update(TimeSpan elapsed);

  private:
    TimeSpan _period;
    TimeSpan _current;
    int32_t _periods;
    bool _periodic;
  };

}


#endif // AA_TIMER_H
