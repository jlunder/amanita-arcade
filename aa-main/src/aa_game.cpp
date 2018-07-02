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



    class ScoreboardVis {
    public:
      ScoreboardVis(size_t layer_start, ShortTimeSpan period, Color bg_color);
      void init();

      void update();

    private:
      static size_t const COLUMNS = 5;
      static size_t const ROWS = 5;
      char _chars[ROWS][COLUMNS];
      aa::Color _colors[ROWS][COLUMNS];

      void renderText();
    };


    class PanelColorTestAnimator : public Lights::Animator {
    public:
      PanelColorTestAnimator()
        : Animator(ShortTimeSpan::from_millis(4000), true) { }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const {
        if(a < 0.5f) {
          dest->box_grad_o(0, 0, 30, 30, Color(0.0f, 0.0f, 0.0f),
            Color(0.25f, 0.0f, 0.0f), Color(0.0f, 0.25f, 0.0f),
            Color(0.25f, 0.25f, 0.0f));
        }
        else {
          dest->box_grad_o(0, 0, 30, 30, Color(0.0f, 0.25f, 0.0f),
            Color(0.0f, 0.25f, 0.25f), Color(0.0f, 0.0f, 0.0f),
            Color(0.0f, 0.0f, 0.25f));
        }
      }
    };


    class HighScoreEntryForegroundAnimator : public Lights::Animator {
    public:
      HighScoreEntryForegroundAnimator()
          : Animator(ShortTimeSpan::from_millis(500), true) {
        if(!_initialized) {
          init();
        }
      }

      void set_state(int32_t x, int32_t y, char const * name,
          int32_t cursor_pos) {
        _x = x;
        _y = y;
        strncpy(_name, name, MAX_NAME_CHARS);
        _cursor_pos = cursor_pos;
      }

      static void init() {
        Color text_tl(1.0f, 0.5f, 0.0f);
        Color text_br(0.5f, 0.0f, 0.0f);
        Color cursor_tl(0.0f, 0.5f, 1.0f);
        Color cursor_br(0.0f, 0.0f, 0.5f);

        _text_tex.init(5, 5, _text_tex_data);
        _text_tex.box_grad_c(0, 0, 5, 5, text_tl, text_tl.lerp(text_br, 0.5f),
          text_tl.lerp(text_br, 0.5f), text_br);
        _cursor_tex.init(5, 5, _cursor_tex_data);
        _cursor_tex.box_grad_c(0, 0, 5, 5, cursor_tl,
          cursor_tl.lerp(cursor_br, 0.5f), cursor_tl.lerp(cursor_br, 0.5f),
          cursor_br);
      }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const {
        for(size_t i = 0; i < MAX_NAME_CHARS; ++i) {
          dest->char_5x5_mask(_x + i * 5, _y, _name[i], &_text_tex);
        }
        if(a >= 0.5f) {
          dest->box_mask(_x + _cursor_pos * 5, _y, 5, 5, &_cursor_tex);
        }
      }

    private:
      static size_t const MAX_NAME_CHARS = 4;
      static bool _initialized;
      static Texture2D _text_tex;
      static Color _text_tex_data[5 * 5];
      static Texture2D _cursor_tex;
      static Color _cursor_tex_data[5 * 5];

      int32_t _x;
      int32_t _y;
      char _name[MAX_NAME_CHARS];
      int32_t _cursor_pos;
    };


    bool HighScoreEntryForegroundAnimator::_initialized = false;
    Texture2D HighScoreEntryForegroundAnimator::_text_tex;
    Color HighScoreEntryForegroundAnimator::_text_tex_data[5 * 5];
    Texture2D HighScoreEntryForegroundAnimator::_cursor_tex;
    Color HighScoreEntryForegroundAnimator::_cursor_tex_data[5 * 5];


/*
    class HighScoreEntryForegroundAnimator : public Lights::Animator {
    public:
      HighScoreEntryForegroundAnimator()
        : Animator(ShortTimeSpan::from_millis(500), true) { }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const;

    private:
      static const Color _;
    };


    class HighScoreListAnimator : public Lights::Animator {
    public:
      PanelAnimator()
        : Animator(ShortTimeSpan::from_millis(1000), true) { }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const;

    private:
      Color _color;
    };


    void PanelAnimator::render(ShortTimeSpan t, float a, Texture2D * dest)
        const {
      dest->fill_solid(Color(0.0f, 0.0f, 0.25f));
      dest->char_5x5(0, 0, 'H', Color::white);
      dest->char_5x5(6, 0, 'E', Color::white);
      dest->char_5x5(12, 0, 'L', Color::white);
      dest->char_5x5(18, 0, 'L', Color::white);
      dest->char_5x5(24, 0, 'O', Color::white);
      dest->char_5x5(0, 6, 'W', Color::white);
      dest->char_5x5(6, 6, 'O', Color::white);
      dest->char_5x5(12,6, 'R', Color::white);
      dest->char_5x5(18, 6, 'L', Color::white);
      dest->char_5x5(24, 6, 'D', Color::white);

      dest->char_10x15(0, 15, '2', Color::white);
      dest->char_10x15(10, 15, '5', Color::white);
      dest->char_10x15(20, 15, '6', Color::white);
    }
*/

    //PanelAnimator panel_animator;
    PanelColorTestAnimator panel_background_animator;
    HighScoreEntryForegroundAnimator panel_animator;


    enum GameState {
      ST_RESET,
      ST_ATTRACT,
      ST_PLAYING,
      ST_WAITING_RESPONSE,
      ST_LISTENING,
      ST_GAME_OVER,
      ST_ENTER_HIGH_SCORE,
    };


    enum AttractState {
      AT_DELAY_TOP,
      AT_SCROLL_DOWN,
      AT_DELAY_BOTTOM,
      AT_SCROLL_UP,
      AT_STATIC_IMAGE_0,
      AT_STATIC_IMAGE_1,
    };


    size_t const PATTERN_LENGTH_MAX = 999;
    char pattern[PATTERN_LENGTH_MAX];
    size_t pattern_length;
    size_t pattern_pos;
    GameState state = ST_RESET;
    aa::Timer state_timer(TimeSpan::zero, false);
  }


  void Game::init() {
    red_stalk_vis.init();
    green_stalk_vis.init();
    blue_stalk_vis.init();
    pink_stalk_vis.init();
    //scoreboard_vis.init();
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
      panel_animator.set_state(5, 12, "JOE", 3);
      Lights::start_animator(
        Lights::LAYER_SB_START + Lights::LAYER_SB_BACKGROUND,
        &panel_background_animator);
      Lights::start_animator(
        Lights::LAYER_SB_START + Lights::LAYER_SB_FOREGROUND, &panel_animator);
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
