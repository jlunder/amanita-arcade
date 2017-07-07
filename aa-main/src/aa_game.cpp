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
      float aa = fabsf(a - 0.5f) + 0.5f;
      Color c = _color.cie_scale(aa);
      dest->fill_solid(c);
    }

    GlowBackgroundAnimator bgR0(ShortTimeSpan::from_millis(1000), Color::red);
    GlowBackgroundAnimator bgR1(ShortTimeSpan::from_millis(1000), Color::red);
    Lights::AnimatorPool bgRPool;
    GlowBackgroundAnimator bgG0(ShortTimeSpan::from_millis(1500), Color::green);
    GlowBackgroundAnimator bgG1(ShortTimeSpan::from_millis(1500), Color::green);
    Lights::AnimatorPool bgGPool;
    GlowBackgroundAnimator bgB0(ShortTimeSpan::from_millis(1750), Color::blue);
    GlowBackgroundAnimator bgB1(ShortTimeSpan::from_millis(1750), Color::blue);
    Lights::AnimatorPool bgBPool;
    GlowBackgroundAnimator bgP0(ShortTimeSpan::from_millis(1250), Color::pink);
    GlowBackgroundAnimator bgP1(ShortTimeSpan::from_millis(1250), Color::pink);
    Lights::AnimatorPool bgPPool;
  }

  void Game::init() {
    bgRPool.add_animator(&bgR0);
    bgRPool.add_animator(&bgR1);
    bgGPool.add_animator(&bgG0);
    bgGPool.add_animator(&bgG1);
    bgBPool.add_animator(&bgB0);
    bgBPool.add_animator(&bgB1);
    bgPPool.add_animator(&bgP0);
    bgPPool.add_animator(&bgP1);

    Lights::start_animator(
      Lights::LAYER_STALK_RED_START + Lights::LAYER_STALK_BASE_COLOR,
      &bgRPool);
    Lights::start_animator(
      Lights::LAYER_STALK_GREEN_START + Lights::LAYER_STALK_BASE_COLOR,
      &bgGPool);
    Lights::start_animator(
      Lights::LAYER_STALK_BLUE_START + Lights::LAYER_STALK_BASE_COLOR,
      &bgBPool);
    Lights::start_animator(
      Lights::LAYER_STALK_PINK_START + Lights::LAYER_STALK_BASE_COLOR,
      &bgPPool);
  }

  void Game::update(ShortTimeSpan dt) {
    /*
    Debug::tracef("%6d %c %c %c %c", dt.to_micros(),
      Input::button_state(Input::B_RED) ? 'R' : 'r',
      Input::button_state(Input::B_GREEN) ? 'G' : 'g',
      Input::button_state(Input::B_BLUE) ? 'B' : 'b',
      Input::button_state(Input::B_PINK) ? 'P' : 'p');
      */
    if(Input::button_pressed(Input::B_RED)) {
      Debug::tracef("Red pressed");
    }
    if(Input::button_pressed(Input::B_GREEN)) {
      Debug::tracef("Green pressed");
    }
    if(Input::button_pressed(Input::B_BLUE)) {
      Debug::tracef("Blue pressed");
    }
    if(Input::button_pressed(Input::B_PINK)) {
      Debug::tracef("Pink pressed");
    }
  }
}
