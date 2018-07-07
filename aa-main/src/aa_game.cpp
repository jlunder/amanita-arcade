#include "amanita_arcade.h"

#include "aa_game.h"

#include "aa_input.h"
#include "aa_lights.h"
#include "aa_timer.h"

#include "aa_game_logo.inl"


namespace aa {
  namespace {
    bool textures_initialized = false;

    AutoTexture2D<5, 5> orange_5x5_grad_tex;
    AutoTexture2D<5, 5> bright_orange_5x5_grad_tex;
    AutoTexture2D<5, 5> aqua_5x5_grad_tex;

    AutoTexture2D<SCOREBOARD_WIDTH, SCOREBOARD_HEIGHT> logo_tex;
    AutoTexture2D<SCOREBOARD_WIDTH, SCOREBOARD_HEIGHT + 10> scroll_tex;

    char const amanita_arcade_text[3][SCOREBOARD_WIDTH / 5 + 1] = {
      " AMA- ",
      " NITA ",
      "ARCADE",
    };
    char const instructions_title_text[3][SCOREBOARD_WIDTH / 5 + 1] = {
      "REPEAT",
      "AFTER ",
      "  ME! ",
    };
    char const instructions_text[29][SCOREBOARD_WIDTH / 5 + 1] = {
      "I WILL",
      " SHOW ",
      "YOU A ",
      " PAT- ",
      " TERN ",
      "ON THE",
      " BIG  ",
      " MUSH-",
      "ROOMS,",
      " YOU  ",
      " PLAY ",
      "IT ON ",
      " THE  ",
      "SMALL ",
      "ONES. ",
      " HOW  ",
      " MUCH ",
      " CAN  ",
      " YOU  ",
      "REMEM-",
      " BER? ",
      "PRESS ",
      "ON ANY",
      "OF THE",
      "SMALL ",
      "MUSH- ",
      "ROOMS ",
      "  TO  ",
      "BEGIN!",
    };


    void init_textures() {
      if(textures_initialized) {
        return;
      }

      Color orange_tl(1.0f, 0.5f, 0.0f);
      Color orange_br(0.5f, 0.0f, 0.0f);
      Color bright_orange_tl(1.0f, 0.75f, 0.25f);
      Color bright_orange_br(0.5f, 0.25f, 0.12f);
      Color aqua_tl(0.0f, 0.5f, 1.0f);
      Color aqua_br(0.0f, 0.0f, 0.5f);

      orange_5x5_grad_tex.box_grad_c(0, 0, 5, 5, orange_tl,
        orange_tl.lerp(orange_br, 0.5f), orange_tl.lerp(orange_br, 0.5f),
        orange_br);
      bright_orange_5x5_grad_tex.box_grad_c(0, 0, 5, 5, bright_orange_tl,
        bright_orange_tl.lerp(bright_orange_br, 0.5f),
        bright_orange_tl.lerp(bright_orange_br, 0.5f), bright_orange_br);
      aqua_5x5_grad_tex.box_grad_c(0, 0, 5, 5, aqua_tl,
        aqua_tl.lerp(aqua_br, 0.5f), aqua_tl.lerp(aqua_br, 0.5f), aqua_br);

      for(size_t y = 0; y < SCOREBOARD_WIDTH; ++y) {
        for(size_t x = 0; x < SCOREBOARD_WIDTH; ++x) {
          switch(logo_image[y * SCOREBOARD_WIDTH + x]) {
            case 'W': logo_tex.set(x, y, Color(1.0f, 1.0f, 1.0f)); break;
            case 'w': logo_tex.set(x, y, Color(0.5f, 0.5f, 0.5f)); break;
            case ',': logo_tex.set(x, y, Color(0.2f, 0.2f, 0.2f)); break;
            case '.': logo_tex.set(x, y, Color(0.1f, 0.1f, 0.1f)); break;
            case 'R': logo_tex.set(x, y, Color(1.0f, 0.0f, 0.0f)); break;
            case 'r': logo_tex.set(x, y, Color(0.5f, 0.0f, 0.0f)); break;
            case ' ': logo_tex.set(x, y, Color(0.0f, 0.0f, 0.0f, 0.0f)); break;
            default:  logo_tex.set(x, y, Color(1.0f, 0.0f, 1.0f)); break;
          }
        }
      }
    }


    void scroll_scoreboard_text(Texture2D * dest, int32_t y,
        char text[][SCOREBOARD_WIDTH / 5 + 1], size_t line_count,
        Texture2D const * tex) {
      static AutoTexture2D<SCOREBOARD_WIDTH, SCOREBOARD_HEIGHT + 5> temp;
      int32_t h = line_count * 5;

      if((y + h <= 0) || (y >= (int32_t)SCOREBOARD_HEIGHT)) {
        return;
      }

      size_t begin_line;
      size_t y_ofs;
      if(y > -5) {
        y_ofs = y + 5;
        begin_line = 0;
      }
      else {
        y_ofs = (-y) % 5;
        begin_line = -y / 5;
      }

      size_t end_line = (SCOREBOARD_HEIGHT + 4 - y) / 5;
      if(end_line > line_count) {
        end_line = line_count;
      }

      temp.fill_solid(Color::transparent);
      for(size_t line = begin_line; line < end_line; ++line) {
        temp.char_5x5_mask(0, (line - begin_line) * 5, text[line], tex);
      }
      dest->mix(&temp, 0, y_ofs);
    }


    class GlowBackgroundAnimator : public Lights::Animator {
    public:
      void init(ShortTimeSpan period, Color color) {
        Animator::init(period, true);
        _color = color;
      }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest)
          const {
        float aa = 0.10f * cosf((float)M_PI * 2.0f * a) + 0.90f;
        Color c = _color.cie_scale(aa);
        dest->fill_solid(c);
      }

    private:
      Color _color;
    };


    class BubbleAnimator : public Lights::Animator {
    public:
      void init(ShortTimeSpan duration, Color color) {
        Animator::init(duration, false);
        _color = color;
      }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest)
          const {
        dest->bubble_x(a * 45.0f, 10.0f, _color);
      }

    private:
      Color _color;
    };


    class GameOverWinAnimator : public Lights::Animator {
    public:
      void init() { Animator::init(ShortTimeSpan::from_millis(4000), false); }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest)
          const {
        dest->lerp_solid(Color::white, 1.0f - fabsf(2.0f * a - 1.0f));
      }
    };


    class GameOverLoseAnimator : public Lights::Animator {
    public:
      void init() { Animator::init(ShortTimeSpan::from_millis(4000), false); }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest)
          const {
        dest->lerp_solid(Color::red, 1.0f - fabsf(2.0f * a - 1.0f));
      }
    };


    class StalkVis {
    public:
      StalkVis(size_t layer_start, ShortTimeSpan period, Color bg_color);
      void init();
      void trigger_bubble();
      void trigger_game_over_win();
      void trigger_game_over_lose();

    private:
      size_t _layer_start;
      Lights::StaticAnimatorPool<GlowBackgroundAnimator, 3> _base_color_pool;
      Lights::StaticAnimatorPool<BubbleAnimator, 3> _bubble_pool;
      Lights::StaticAnimatorPool<GameOverWinAnimator, 3> _game_over_win_pool;
      Lights::StaticAnimatorPool<GameOverLoseAnimator, 3> _game_over_lose_pool;
    };


    StalkVis::StalkVis(size_t layer_start, ShortTimeSpan period,
        Color bg_color)
        : _layer_start(layer_start)
    {
      _base_color_pool.init([=] (GlowBackgroundAnimator * anim) {
        anim->init(period, bg_color);
      });
      _bubble_pool.init([=] (BubbleAnimator * anim) {
        anim->init(ShortTimeSpan::from_millis(500), Color::white);
      });
      _game_over_win_pool.default_init();
      _game_over_lose_pool.default_init();
    }


    void StalkVis::init() {
      Lights::start_animator(_layer_start + Lights::LAYER_STALK_BASE_COLOR,
        _base_color_pool.acquire());
    }


    void StalkVis::trigger_bubble() {
      Lights::start_animator(_layer_start + Lights::LAYER_STALK_BUBBLE,
        _bubble_pool.acquire());
    }


    void StalkVis::trigger_game_over_win() {
      Lights::start_animator(_layer_start + Lights::LAYER_STALK_GAME_OVER_FADE,
        _game_over_win_pool.acquire());
    }


    void StalkVis::trigger_game_over_lose() {
      Lights::start_animator(_layer_start + Lights::LAYER_STALK_GAME_OVER_FADE,
        _game_over_lose_pool.acquire());
    }


    StalkVis red_stalk_vis(Lights::LAYER_STALK_RED_START,
      ShortTimeSpan::from_millis(1023), Color::red);
    StalkVis green_stalk_vis(Lights::LAYER_STALK_GREEN_START,
      ShortTimeSpan::from_millis(1490), Color::green);
    StalkVis blue_stalk_vis(Lights::LAYER_STALK_BLUE_START,
      ShortTimeSpan::from_millis(1717), Color::blue);
    StalkVis pink_stalk_vis(Lights::LAYER_STALK_PINK_START,
      ShortTimeSpan::from_millis(1201), Color::pink);


    class ScoreboardSplashAnimator : public Lights::Animator {
    public:
      void init() {
        Animator::init(ShortTimeSpan::from_millis(4000), true);
      }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const {
        //dest->copy(logo_tex);
        dest->char_5x5_mask(0, 3, " AMA- ", &orange_5x5_grad_tex);
        dest->char_5x5_mask(0, 3, " NITA ", &orange_5x5_grad_tex);
        dest->char_5x5_mask(0, 3, "ARCADE", &orange_5x5_grad_tex);
      }
    };


    class ScoreboardInstructionsAnimator : public Lights::Animator {
    public:
      void init() {
        Animator::init(ShortTimeSpan::from_millis(4000), true);
      }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const {
        dest->fill_solid(Color::red);
      }
    };


    class ScoreboardInGameAnimator : public Lights::Animator {
    public:
      void init() {
        Animator::init(ShortTimeSpan::from_millis(4000), true);
      }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const {
        size_t w = dest->get_width();
        size_t h = dest->get_height();
        for(size_t y = 0; y < h; ++y) {
          for(size_t x = 0; x < w; ++x) {
            dest->set(x, y, Color::black);
          }
        }
      }
    };


    class ScoreboardCorrectAnimator : public Lights::Animator {
    public:
      void init() {
        Animator::init(ShortTimeSpan::from_millis(500), true);
      }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const {
        dest->fill_solid(Color(1.0f, 1.0f, 1.0f, 1.0f - a));
      }
    };


    class ScoreboardGameOverWinAnimator : public Lights::Animator {
    public:
      void init() {
        Animator::init(ShortTimeSpan::from_millis(4000), true);
      }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const {
        dest->fill_solid(Color::red);
      }
    };


    class ScoreboardGameOverLoseAnimator : public Lights::Animator {
    public:
      void init() {
        Animator::init(ShortTimeSpan::from_millis(4000), true);
      }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const {
        dest->fill_solid(Color::black);
        dest->char_5x5_mask( 2, 10, "WRONG", &orange_5x5_grad_tex);
      }
    };


    class ScoreboardVis {
    public:
      ScoreboardVis() : _current_score(0), _best_score(0) { };
      void init();

      void set_mode_attract();
      void set_mode_in_game();

      void trigger_correct();
      void trigger_game_over_win();
      void trigger_game_over_lose();

      void set_score(size_t score);

    private:
      ScoreboardSplashAnimator _splash;
      ScoreboardInstructionsAnimator _instructions;
      ScoreboardInGameAnimator _in_game;
      Lights::StaticAnimatorPool<ScoreboardCorrectAnimator, 3> _correct_pool;
      ScoreboardGameOverWinAnimator _game_over_win;
      ScoreboardGameOverLoseAnimator _game_over_lose;

      size_t _current_score;
      size_t _best_score;
    };


    void ScoreboardVis::init() {
      _splash.init();
      _instructions.init();
      _in_game.init();
      _correct_pool.default_init();
      _game_over_win.init();
      _game_over_lose.init();
    }


    void ScoreboardVis::set_mode_attract() {
      Lights::start_animator(Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
        &_splash);
      _current_score = 0;
    }


    void ScoreboardVis::set_mode_in_game() {
      //Lights::start_animator(LAYER_SB_START + LAYER_SB_MAIN, &_in_game);
    }


    void ScoreboardVis::trigger_correct() {
      //Lights::start_animator(LAYER_SB_START + LAYER_SB_OVERLAY,
      //  &_correct_pool);
    }


    void ScoreboardVis::trigger_game_over_win() {
      //Lights::start_animator(LAYER_SB_START + LAYER_SB_MAIN, &_game_over_win,
      //  ShortTimeSpan::from_millis(250));
    }


    void ScoreboardVis::trigger_game_over_lose() {
      //Lights::start_animator(LAYER_SB_START + LAYER_SB_MAIN, &_game_over_lose,
      //  ShortTimeSpan::from_millis(250));
    }


    void ScoreboardVis::set_score(size_t score) {
      _current_score = score;
      if(score > _best_score) {
        _best_score = score;
      }
    }


    ScoreboardVis scoreboard_vis;

/*
    class HighScoreEntryForegroundAnimator : public Lights::Animator {
    public:
      void init() {
        if(!_textures_initialized) {
          init_textures();
        }
        Animator::init(ShortTimeSpan::from_millis(500), true);
      }

      void set_state(int32_t x, int32_t y, char const * name,
          int32_t cursor_pos) {
        _x = x;
        _y = y;
        strncpy(_name, name, MAX_NAME_CHARS);
        _cursor_pos = cursor_pos;
      }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const {
        for(size_t i = 0; i < MAX_NAME_CHARS; ++i) {
          dest->char_5x5_mask(_x + i * 5, _y, _name[i], false, &_text_tex);
        }
        if(a >= 0.5f) {
          dest->box_mask(_x + _cursor_pos * 5, _y, 5, 5, &_cursor_tex);
        }
      }

    private:
      static size_t const MAX_NAME_CHARS = 4;

      int32_t _x;
      int32_t _y;
      char _name[MAX_NAME_CHARS];
      int32_t _cursor_pos;
    };
    */


    enum GameState {
      ST_RESET,
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
    init_textures();

    red_stalk_vis.init();
    green_stalk_vis.init();
    blue_stalk_vis.init();
    pink_stalk_vis.init();
    scoreboard_vis.init();
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
      scoreboard_vis.set_mode_attract();
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
          scoreboard_vis.set_mode_in_game();
          scoreboard_vis.set_score(1);
        } else if(input == pattern[pattern_pos]) {
          Debug::tracef("Correct input %c", input);
          scoreboard_vis.trigger_correct();
          ++pattern_pos;
          if(pattern_pos >= pattern_length) {
            scoreboard_vis.set_score(pattern_length);
            if(pattern_length == PATTERN_LENGTH_MAX) {
              Debug::tracef("Win!!");
              red_stalk_vis.trigger_game_over_win();
              green_stalk_vis.trigger_game_over_win();
              blue_stalk_vis.trigger_game_over_win();
              pink_stalk_vis.trigger_game_over_win();
              scoreboard_vis.trigger_game_over_win();
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
          scoreboard_vis.trigger_game_over_lose();
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
