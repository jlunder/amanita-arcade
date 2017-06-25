#ifndef AA_LIGHTS_H
#define AA_LIGHTS_H


#include "amanita_arcade.h"

#include "aa_color.h"
#include "aa_time_span.h"


namespace aa {
  class Lights {
  public:
    #if 0
    static const size_t NUM_STALK_LIGHTS_RED   = 1 * 3; // 36 * 3;
    static const size_t NUM_STALK_LIGHTS_GREEN = 1 * 3; // 24 * 3;
    static const size_t NUM_STALK_LIGHTS_BLUE  = 1 * 3; // 41 * 3;
    static const size_t NUM_STALK_LIGHTS_PINK  = 1 * 3; // 32 * 3;

    static const size_t NUM_SCOREBOARD_LIGHTS  = 2 * 2; // 30 * 30;
    #else
    static const size_t NUM_STALK_LIGHTS_RED   = 36 * 3; // Stalk labeled "1"
    static const size_t NUM_STALK_LIGHTS_GREEN = 24 * 3; // Stalk labeled "B"
    static const size_t NUM_STALK_LIGHTS_BLUE  = 41 * 3; // Stalk labeled "A"
    static const size_t NUM_STALK_LIGHTS_PINK  = 32 * 3; // Stalk labeled "2"

    static const size_t NUM_SCOREBOARD_LIGHTS  = 30 * 30;

    static const size_t LIGHTS_BUF_PAGE_COUNT = 16;
    static const size_t LIGHTS_BUF_PAGE_SIZE = 128;
    #endif

    static void init();

    static void play_pattern();

    static void update(ShortTimeSpan dt);

  private:
    static const size_t NUM_LIGHTS =
      NUM_STALK_LIGHTS_RED +
      NUM_STALK_LIGHTS_GREEN +
      NUM_STALK_LIGHTS_BLUE +
      NUM_STALK_LIGHTS_PINK +
      NUM_SCOREBOARD_LIGHTS;

    static uint32_t color_buf[LIGHTS_BUF_PAGE_COUNT][LIGHTS_BUF_PAGE_SIZE];
  };
}


#endif // AA_LIGHTS_H
