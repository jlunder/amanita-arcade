#ifndef AA_LIGHTS_H
#define AA_LIGHTS_H


#include "amanita_arcade.h"

#include "aa_color.h"
#include "aa_texture_2d.h"
#include "aa_time_span.h"


namespace aa {
  class Lights {
  public:
    class Animator;

    class AnimatorPool {
    public:
      virtual Animator * acquire() = 0;

    protected:
      bool _initialized;
    };

    template<typename T, size_t POOL_COUNT>
    class StaticAnimatorPool : public AnimatorPool {
    public:
      StaticAnimatorPool() : _next(0) { }

      void default_init() {
        Debug::auto_assert(!_initialized);
        for(size_t i = 0; i < POOL_COUNT; ++i) {
          _animators[i].init();
        }
        _initialized = true;
      }

      template<typename F>
      void init(F init_func) {
        Debug::auto_assert(!_initialized);
        for(size_t i = 0; i < POOL_COUNT; ++i) {
          init_func(&_animators[i]);
        }
        _initialized = true;
      }

      virtual Animator * acquire() {
        // Try to find an idle animator
        for(size_t i = 0; i < POOL_COUNT; ++i) {
          if(!_animators[_next].is_in_use()) {
            return &_animators[_next];
          }
          _next = (_next + 1) % POOL_COUNT;
        }
        // Because there are two animators in every pool, and Lights discards
        // all but one for any given layer before it tries to get a new
        // animator, this should only happen if you're sharing AnimatorPools
        // between multiple layers (which shouldn't be done!)
        Debug::error("No free animators in this pool!");
        return nullptr;
      }

    private:
      size_t _next;
      T _animators[POOL_COUNT];
    };

    class Animator {
    public:
      enum EndBehavior : uint8_t {
        EB_STOP,
        EB_LOOP,
        EB_PAUSE,
      };

      Animator()
          : _end_behavior(), _state(AS_RESET), _anim_length(), _total_time() {
      }
      virtual ~Animator() { }

      bool is_in_use() const { return _state != AS_RESET; }
      bool is_at_end() const {
        if(!is_in_use()) {
          return true;
        }
        if(_end_behavior == EB_LOOP) {
          return false;
        }
        return _total_time >= _anim_length;
      }
      bool is_transitioning() const { return _state == AS_TRANSITIONING; }

      void play();
      void restart();
      void transition();
      void stop();

      bool animate(ShortTimeSpan dt);
      void render(Texture2D * dest) const;

    protected:
      enum AnimationState : uint8_t {
        AS_RESET,
        AS_PLAYING,
        AS_TRANSITIONING,
      };

      void init() { init(TimeSpan::zero, EB_PAUSE); }

      void init(ShortTimeSpan anim_length,
          EndBehavior end_behavior = EB_STOP) {
        Debug::auto_assert(_state == AS_RESET);
        _end_behavior = end_behavior;
        _anim_length = anim_length;
        _total_time = TimeSpan::zero;
      }

      virtual void render(ShortTimeSpan t, float a, Texture2D * dest)
        const = 0;

    private:
      EndBehavior _end_behavior;
      AnimationState _state;
      ShortTimeSpan _anim_length;
      ShortTimeSpan _total_time;
    };

    static const size_t PAGE_COUNT = 16;
    static const size_t PAGE_SIZE = 128;

    // Stalk layers
    static size_t const LAYER_STALK_BASE_COLOR = 0;
    static size_t const LAYER_STALK_BUBBLE     = 1;
    static size_t const LAYER_STALK_OVERLAY    = 2;
    static size_t const LAYER_STALK_COUNT      = 3;

    // Scoreboard layers
    static size_t const LAYER_SB_BACKGROUND = 0;
    static size_t const LAYER_SB_MAIN       = 1;
    static size_t const LAYER_SB_OVERLAY    = 2;
    static size_t const LAYER_SB_COUNT      = 3;

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

    static void start_animator(size_t layer, Animator * anim,
      ShortTimeSpan transition = ShortTimeSpan::from_micros(0));
    static void transition_out(size_t layer,
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
    static size_t const SCOREBOARD_PAGES_START = 4;
    static size_t const SCOREBOARD_LIGHTS_PER_PAGE = 120;
    static size_t const SCOREBOARD_LINES_PER_PAGE =
      SCOREBOARD_LIGHTS_PER_PAGE / SCOREBOARD_WIDTH;
    static size_t const SCOREBOARD_PAGES_COUNT =
      (SCOREBOARD_LIGHTS_COUNT + SCOREBOARD_LIGHTS_PER_PAGE - 1) /
        SCOREBOARD_LIGHTS_PER_PAGE; // = 8 pages as of this writing

    static size_t const TEXTURE_TEMP_MAX = 30 * 30;

    static Layer _layers[LAYER_COUNT];
    static uint32_t _output_buf[PAGE_COUNT][PAGE_SIZE];
    static Texture2D _composite_tex;
    static Texture2D _transition_tex;
    static Color _composite_tex_data[TEXTURE_TEMP_MAX];
    static Color _transition_tex_data[TEXTURE_TEMP_MAX];

    static void update_animators(ShortTimeSpan dt);
    static void update_composite_layers_to_composite_tex(size_t layer_start,
      size_t layer_count);
    static void update_encode_stalk_texture_to_output(size_t page,
      Texture2D const * tex);
    static void update_encode_scoreboard_texture_to_output(size_t page_start,
      size_t page_count, size_t lines_per_page, Texture2D const * tex);
  };
}


#endif // AA_LIGHTS_H
