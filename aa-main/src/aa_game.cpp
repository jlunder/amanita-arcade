#include "amanita_arcade.h"

#include "aa_game.h"

#include "aa_input.h"
#include "aa_lights.h"


namespace aa {
  namespace {
    class GlowBackgroundAnimator : public Lights::Animator {
    public:
      GlowBackgroundAnimator(ShortTimeSpan period, Color color)
        : Animator(period, true), _color(color) { }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest)
        const;

    private:
      Color _color;
    };

    void GlowBackgroundAnimator::render(ShortTimeSpan t, float a,
        Texture2D * dest) const {
      float aa = 0.25f * cosf((float)M_PI * 2.0f * a) + 0.75f;
      Color c = _color.cie_scale(aa);
      dest->fill_solid(c);
    }

    class BubbleAnimator : public Lights::Animator {
    public:
      BubbleAnimator(ShortTimeSpan duration, Color color)
        : Animator(duration, false), _color(color) { }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest)
        const;

    private:
      Color _color;
    };

    void BubbleAnimator::render(ShortTimeSpan t, float a,
        Texture2D * dest) const {
      dest->bubble_x(a * 45.0f, 5.0f, _color);
    }

    ShortTimeSpan const BUBBLE_DURATION = ShortTimeSpan::from_millis(500);
    Color const BUBBLE_COLOR(1.0f, 1.0f, 1.0f);

    class StalkVis {
    public:
      StalkVis(size_t layer_start, ShortTimeSpan period, Color bg_color);
      void init();
      void trigger_bubble();

    private:
      size_t _layer_start;
      GlowBackgroundAnimator _base_color_0;
      GlowBackgroundAnimator _base_color_1;
      Lights::AnimatorPool _base_color_pool;
      BubbleAnimator _bubble_0;
      BubbleAnimator _bubble_1;
      Lights::AnimatorPool _bubble_pool;
    };

    StalkVis::StalkVis(size_t layer_start, ShortTimeSpan period,
        Color bg_color)
        : _layer_start(layer_start),
        _base_color_0(period, bg_color),
        _base_color_1(period, bg_color),
        _bubble_0(BUBBLE_DURATION, BUBBLE_COLOR),
        _bubble_1(BUBBLE_DURATION, BUBBLE_COLOR)
    {
      _base_color_pool.add_animator(&_base_color_1);
      _base_color_pool.add_animator(&_base_color_1);
      _bubble_pool.add_animator(&_bubble_1);
      _bubble_pool.add_animator(&_bubble_1);
    }

    void StalkVis::init() {
      Lights::start_animator(_layer_start + Lights::LAYER_STALK_BASE_COLOR,
        &_base_color_pool);
    }

    void StalkVis::trigger_bubble() {
      Lights::start_animator(_layer_start + Lights::LAYER_STALK_BUBBLE,
        &_bubble_pool);
    }

    StalkVis red_stalk_vis(Lights::LAYER_STALK_RED_START,
      ShortTimeSpan::from_millis(1023), Color::red);
    StalkVis green_stalk_vis(Lights::LAYER_STALK_GREEN_START,
      ShortTimeSpan::from_millis(1490), Color::green);
    StalkVis blue_stalk_vis(Lights::LAYER_STALK_BLUE_START,
      ShortTimeSpan::from_millis(1717), Color::blue);
    StalkVis pink_stalk_vis(Lights::LAYER_STALK_PINK_START,
      ShortTimeSpan::from_millis(1201), Color::pink);
  }

  void Game::init() {
    red_stalk_vis.init();
    green_stalk_vis.init();
    blue_stalk_vis.init();
    pink_stalk_vis.init();
  }

  void Game::update(ShortTimeSpan dt) {
    if(Input::button_pressed(Input::B_RED)) {
      Debug::tracef("Red pressed");
      red_stalk_vis.trigger_bubble();
    }
    if(Input::button_pressed(Input::B_GREEN)) {
      Debug::tracef("Green pressed");
      green_stalk_vis.trigger_bubble();
    }
    if(Input::button_pressed(Input::B_BLUE)) {
      Debug::tracef("Blue pressed");
      blue_stalk_vis.trigger_bubble();
    }
    if(Input::button_pressed(Input::B_PINK)) {
      Debug::tracef("Pink pressed");
      pink_stalk_vis.trigger_bubble();
    }
  }
}
