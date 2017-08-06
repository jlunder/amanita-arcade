#include "amanita_arcade.h"

#include "aa_game.h"

#include "aa_input.h"
#include "aa_lights.h"
#include "aa_timer.h"


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
      float aa = 0.10f * cosf((float)M_PI * 2.0f * a) + 0.90f;
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
      dest->bubble_x(a * 45.0f, 10.0f, _color);
    }


    class GameOverWinAnimator : public Lights::Animator {
    public:
      GameOverWinAnimator()
        : Animator(ShortTimeSpan::from_millis(4000), false) { }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest)
        const;
    };


    void GameOverWinAnimator::render(ShortTimeSpan t, float a,
        Texture2D * dest) const {
      dest->lerp_solid(Color::white, 1.0f - fabsf(2.0f * a - 1.0f));
    }


    class GameOverLoseAnimator : public Lights::Animator {
    public:
      GameOverLoseAnimator()
        : Animator(ShortTimeSpan::from_millis(4000), false) { }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest)
        const;
    };


    void GameOverLoseAnimator::render(ShortTimeSpan t, float a,
        Texture2D * dest) const {
      dest->lerp_solid(Color::red, 1.0f - fabsf(2.0f * a - 1.0f));
    }


    ShortTimeSpan const BUBBLE_DURATION = ShortTimeSpan::from_millis(500);
    Color const BUBBLE_COLOR(1.0f, 1.0f, 1.0f);


    class StalkVis {
    public:
      StalkVis(size_t layer_start, ShortTimeSpan period, Color bg_color);
      void init();
      void trigger_bubble();
      void trigger_game_over_win();
      void trigger_game_over_lose();

    private:
      size_t _layer_start;
      GlowBackgroundAnimator _base_color_0;
      GlowBackgroundAnimator _base_color_1;
      Lights::AnimatorPool _base_color_pool;
      BubbleAnimator _bubble_0;
      BubbleAnimator _bubble_1;
      Lights::AnimatorPool _bubble_pool;
      GameOverWinAnimator _game_over_win_0;
      GameOverWinAnimator _game_over_win_1;
      Lights::AnimatorPool _game_over_win_pool;
      GameOverLoseAnimator _game_over_lose_0;
      GameOverLoseAnimator _game_over_lose_1;
      Lights::AnimatorPool _game_over_lose_pool;
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
      _game_over_win_pool.add_animator(&_game_over_win_0);
      _game_over_win_pool.add_animator(&_game_over_win_1);
      _game_over_lose_pool.add_animator(&_game_over_lose_0);
      _game_over_lose_pool.add_animator(&_game_over_lose_1);
    }


    void StalkVis::init() {
      Lights::start_animator(_layer_start + Lights::LAYER_STALK_BASE_COLOR,
        &_base_color_pool);
    }


    void StalkVis::trigger_bubble() {
      Lights::start_animator(_layer_start + Lights::LAYER_STALK_BUBBLE,
        &_bubble_pool);
    }


    void StalkVis::trigger_game_over_win() {
      Lights::start_animator(_layer_start + Lights::LAYER_STALK_GAME_OVER_FADE,
        &_game_over_win_pool);
    }


    void StalkVis::trigger_game_over_lose() {
      Lights::start_animator(_layer_start + Lights::LAYER_STALK_GAME_OVER_FADE,
        &_game_over_lose_pool);
    }


    StalkVis red_stalk_vis(Lights::LAYER_STALK_RED_START,
      ShortTimeSpan::from_millis(1023), Color::red);
    StalkVis green_stalk_vis(Lights::LAYER_STALK_GREEN_START,
      ShortTimeSpan::from_millis(1490), Color::green);
    StalkVis blue_stalk_vis(Lights::LAYER_STALK_BLUE_START,
      ShortTimeSpan::from_millis(1717), Color::blue);
    StalkVis pink_stalk_vis(Lights::LAYER_STALK_PINK_START,
      ShortTimeSpan::from_millis(1201), Color::pink);


    enum State {
      ST_RESET,
      ST_PLAYING,
      ST_WAITING_RESPONSE,
      ST_LISTENING,
      ST_GAME_OVER,
    };


    size_t const PATTERN_LENGTH_MAX = 99;
    char pattern[PATTERN_LENGTH_MAX];
    size_t pattern_length;
    size_t pattern_pos;
    State state = ST_RESET;
    aa::Timer state_timer(TimeSpan::zero, false);
  }


  void Game::init() {
    red_stalk_vis.init();
    green_stalk_vis.init();
    blue_stalk_vis.init();
    pink_stalk_vis.init();
  }


  void Game::update(ShortTimeSpan dt) {
    state_timer.update(dt);

    switch(state) {
    case ST_RESET:
      Debug::tracef("Reset");
      pattern_length = 0;
      pattern_pos = 0;
      state = ST_LISTENING;
      state_timer.cancel();
      update(TimeSpan::zero);
      break;
    case ST_PLAYING:
      if(state_timer.get_time_remaining() <= TimeSpan::zero) {
        if(pattern_pos < pattern_length) {
          Debug::tracef("Play pos %d = %c", pattern_pos, pattern[pattern_pos]);
          trigger_bubble(pattern[pattern_pos]);
          ++pattern_pos;
          state_timer = aa::Timer(TimeSpan::from_millis(500), false);
        } else {
          Debug::tracef("Awaiting response");
          state = ST_WAITING_RESPONSE;
          state_timer = aa::Timer(TimeSpan::from_millis(15000), false);
        }
      }
      break;
    case ST_WAITING_RESPONSE:
      if(is_button_pressed()) {
        Debug::tracef("Button press, listening");
        state = ST_LISTENING;
        pattern_pos = 0;
        state_timer.cancel();
        update(TimeSpan::zero);
      } else if(state_timer.get_time_remaining() <= TimeSpan::zero) {
        Debug::tracef("Timeout, game over");
        state = ST_GAME_OVER;
        state_timer = aa::Timer(TimeSpan::from_millis(5000), false);
        red_stalk_vis.trigger_game_over_lose();
        green_stalk_vis.trigger_game_over_lose();
        blue_stalk_vis.trigger_game_over_lose();
        pink_stalk_vis.trigger_game_over_lose();
      }
      break;
    case ST_LISTENING:
    {
      char input = get_pressed_button();
      if(input != 0) {
        if(pattern_length == 0) {
          pattern[0] = input;
          pattern[1] = get_random_button();
          pattern_length = 2;
          pattern_pos = 0;
          state = ST_PLAYING;
          state_timer = aa::Timer(TimeSpan::from_millis(1000), false);
        } else if(input == pattern[pattern_pos]) {
          Debug::tracef("Correct input %c", input);
          ++pattern_pos;
          if(pattern_pos >= pattern_length) {
            if(pattern_length == PATTERN_LENGTH_MAX) {
              Debug::tracef("Win!!");
              red_stalk_vis.trigger_game_over_win();
              green_stalk_vis.trigger_game_over_win();
              blue_stalk_vis.trigger_game_over_win();
              pink_stalk_vis.trigger_game_over_win();
              state = ST_GAME_OVER;
              state_timer = aa::Timer(TimeSpan::from_millis(5000), false);
            } else {
              pattern[pattern_length] = get_random_button();
              ++pattern_length;
              pattern_pos = 0;
              state = ST_PLAYING;
              state_timer = aa::Timer(TimeSpan::from_millis(1000), false);
            }
          }
        } else {
          Debug::tracef("Wrong input %c, loss", input);
          red_stalk_vis.trigger_game_over_lose();
          green_stalk_vis.trigger_game_over_lose();
          blue_stalk_vis.trigger_game_over_lose();
          pink_stalk_vis.trigger_game_over_lose();
          state = ST_GAME_OVER;
          state_timer = aa::Timer(TimeSpan::from_millis(5000), false);
        }
      }
      break;
    }
    case ST_GAME_OVER:
      if(state_timer.get_time_remaining() <= TimeSpan::zero) {
        state = ST_RESET;
      }
      break;
    default:
      state_timer.cancel();
      state = ST_RESET;
    }
  }

  bool Game::is_button_pressed() {
    return Input::button_pressed(Input::B_RED) ||
        Input::button_pressed(Input::B_GREEN) ||
        Input::button_pressed(Input::B_BLUE) ||
        Input::button_pressed(Input::B_PINK);
  }

  char Game::get_pressed_button() {
    if(Input::button_pressed(Input::B_RED)) {
      red_stalk_vis.trigger_bubble();
      return 'R';
    }
    if(Input::button_pressed(Input::B_GREEN)) {
      green_stalk_vis.trigger_bubble();
      return 'G';
    }
    if(Input::button_pressed(Input::B_BLUE)) {
      blue_stalk_vis.trigger_bubble();
      return 'B';
    }
    if(Input::button_pressed(Input::B_PINK)) {
      pink_stalk_vis.trigger_bubble();
      return 'P';
    }
    return 0;
  }

  char Game::get_random_button() {
    switch(rand() % 4) {
    case 0: return 'R';
    case 1: return 'G';
    case 2: return 'B';
    case 3: return 'P';
    default: return 0;
    }
  }

  void Game::trigger_bubble(char button) {
    switch(pattern[pattern_pos]) {
      case 'R':
        red_stalk_vis.trigger_bubble();
        break;
      case 'G':
        green_stalk_vis.trigger_bubble();
        break;
      case 'B':
        blue_stalk_vis.trigger_bubble();
        break;
      case 'P':
        pink_stalk_vis.trigger_bubble();
        break;
    }
  }
}
