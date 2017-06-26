#ifndef AA_LIGHTS_H
#define AA_LIGHTS_H


#include "amanita_arcade.h"

#include "aa_color.h"
#include "aa_time_span.h"


namespace aa {
  class Lights {
  public:
    class Animator {
    public:
      virtual ~Animator() { }
      virtual void animate(MushroomID mushroom, float t) = 0;
    };

    static const size_t PAGE_COUNT = 16;
    static const size_t PAGE_SIZE = 128;

    enum LayerID {
      L_BASE_COLOR,
      L_BUBBLE,
      L_GAME_OVER_FADE,
      L_COUNT,
    };

    static void init();

    static void start_mushroom_animator(MushroomID mushroom, size_t layer,
      Animator * a);

    static void mix_solid(MushroomID mushroom, LayerID layer, Color color);
    static void mix_noise(MushroomID mushroom, LayerID layer, Color color,
      float min_l, float max_l);
    static void mix_bubble(MushroomID mushroom, LayerID layer,
      float height, float size);

    static void update(ShortTimeSpan dt);
    // BEWARE, output() will disable interrupts for approximately 5ms!
    static void output();

  private:
    static const size_t STALK_PAGE_RED = 0;
    static const size_t STALK_PAGE_GREEN = 1;
    static const size_t STALK_PAGE_BLUE = 2;
    static const size_t STALK_PAGE_PINK = 3;
    static const size_t SCOREBOARD_PAGES_START = 8;
    static const size_t SCOREBOARD_LIGHTS_PER_PAGE = 120;
    static const size_t SCOREBOARD_PAGES_COUNT =
      (SCOREBOARD_LIGHTS_COUNT + SCOREBOARD_LIGHTS_PER_PAGE - 1) /
        SCOREBOARD_LIGHTS_PER_PAGE; // Really 7.5

    static uint32_t color_buf[PAGE_COUNT][PAGE_SIZE];
  };
}


#endif // AA_LIGHTS_H
