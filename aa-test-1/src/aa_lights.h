#ifndef AA_LIGHTS_H
#define AA_LIGHTS_H


#include "amanita_arcade.h"

#include "aa_color.h"
#include "aa_texture_2d.h"
#include "aa_time_span.h"


namespace aa {
  class Lights {
  public:
    static const size_t POOL_COUNT = 2;

    class Animator;

    class AnimatorPool {
    public:
      AnimatorPool();
      void add_animator(Animator * anim);
      Animator * get_animator();

    private:
      Animator * _animators[POOL_COUNT];
      size_t _next;
    };

    class Animator {
    public:
      Animator(ShortTimeSpan anim_length, bool looping)
        : _looping(looping), _playing(false), _transitioning(false),
        _anim_length(anim_length), _total_time() { }
      virtual ~Animator() { }

      bool is_playing() { return _playing; }
      bool is_transitioning() { return _transitioning; }

      void on_play();
      void on_transition();
      void on_stop();

      bool animate(ShortTimeSpan dt);
      void render(Texture2D * dest) const;

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest)
        const = 0;

    private:
      bool _looping;
      bool _playing;
      bool _transitioning;
      ShortTimeSpan _anim_length;
      ShortTimeSpan _total_time;
    };

    static const size_t PAGE_COUNT = 16;
    static const size_t PAGE_SIZE = 128;

    // Stalk layers
    static size_t const LAYER_STALK_BASE_COLOR     = 0;
    static size_t const LAYER_STALK_BUBBLE         = 1;
    static size_t const LAYER_STALK_GAME_OVER_FADE = 2;
    static size_t const LAYER_STALK_COUNT          = 3;

    // Scoreboard layers
    static size_t const LAYER_SB_BACKGROUND = 0;
    static size_t const LAYER_SB_FOREGROUND = 1;
    static size_t const LAYER_SB_COUNT      = 2;

    static size_t const LAYER_STALK_RED_START   = 0;
    static size_t const LAYER_STALK_GREEN_START =
      LAYER_STALK_RED_START + LAYER_STALK_COUNT;
    static size_t const LAYER_STALK_BLUE_START  =
      LAYER_STALK_GREEN_START + LAYER_STALK_COUNT;
    static size_t const LAYER_STALK_PINK_START  =
      LAYER_STALK_BLUE_START + LAYER_STALK_COUNT;
    static size_t const LAYER_SB_START =
      LAYER_STALK_PINK_START + LAYER_STALK_COUNT;
    static size_t const LAYER_COUNT = LAYER_SB_START + LAYER_SB_COUNT;
    static size_t const LAYER_NONE  = LAYER_COUNT;

    static void init();

    static void start_animator(size_t layer, AnimatorPool * pool,
      ShortTimeSpan transition = ShortTimeSpan::from_micros(0));

    static void update(ShortTimeSpan dt);
    // BEWARE, output() will disable interrupts for approximately 5ms!
    static void output();

  private:
    struct Layer {
      size_t const width;
      size_t const height;
      Animator * animator;
      Animator * trans_animator;
      ShortTimeSpan trans_length;
      ShortTimeSpan trans_time;
    };

    // Pages correspond to the pin index on port E: page 0 is output on PE0,
    // page 3 on PE3, etc.
    static size_t const STALK_PAGE_RED   = 0;
    static size_t const STALK_PAGE_GREEN = 1;
    static size_t const STALK_PAGE_BLUE  = 2;
    static size_t const STALK_PAGE_PINK  = 3;
    static size_t const SCOREBOARD_PAGES_START = 8;
    static size_t const SCOREBOARD_LIGHTS_PER_PAGE = 120;
    static size_t const SCOREBOARD_PAGES_COUNT =
      (SCOREBOARD_LIGHTS_COUNT + SCOREBOARD_LIGHTS_PER_PAGE - 1) /
        SCOREBOARD_LIGHTS_PER_PAGE; // = 8 pages as of this writing

    static size_t const TEXTURE_TEMP_MAX = 30 * 30;

    static Layer _layers[LAYER_COUNT];
    static uint32_t _output_buf[PAGE_COUNT][PAGE_SIZE];
    static Color _texture_temp[2][TEXTURE_TEMP_MAX];

    static void update_animators(ShortTimeSpan dt);
    static void update_composite_layers(Texture2D * tex, size_t layer_start,
      size_t layer_count);
    static void update_encode_stalk_texture_to_output(size_t page,
      Texture2D const * tex);
    static void update_encode_scoreboard_texture_to_output(size_t page_start,
      size_t lines_per_page, Texture2D const * tex);
  };
}


#endif // AA_LIGHTS_H
