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

    char const splash_text[3][SCOREBOARD_WIDTH / 5 + 1] = {
      "AMA-",
      "NITA",
      "ARCADE",
    };
    char const instructions_title_text[3][SCOREBOARD_WIDTH / 5 + 1] = {
      "REPEAT",
      "AFTER",
      "ME!",
    };
    char const instructions_text[29][SCOREBOARD_WIDTH / 5 + 1] = {
      "I WILL",
      "SHOW",
      "YOU A",
      "PAT-",
      "TERN",
      "ON THE",
      "BIG",
      "MUSH-",
      "ROOMS,",
      "YOU",
      "PLAY",
      "IT ON",
      "THE",
      "SMALL",
      "ONES.",
      "HOW",
      "MUCH",
      "CAN",
      "YOU",
      "REMEM-",
      "BER?",
      "PRESS",
      "ON ANY",
      "OF THE",
      "SMALL",
      "MUSH-",
      "ROOMS",
      "TO",
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


    int32_t scoreboard_center(char const * str, int32_t char_w) {
      return (SCOREBOARD_WIDTH - strlen(str) * char_w) / 2;
    }


    void scroll_scoreboard_text(Texture2D * dest, int32_t y,
        char const (* text)[SCOREBOARD_WIDTH / 5 + 1], size_t line_count,
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
        temp.char_5x5_mask(scoreboard_center(text[line], 5),
          (line - begin_line) * 5, text[line], tex);
      }
      dest->mix(&temp, 0, y_ofs);
    }


    class SolidBackgroundAnimator : public Lights::Animator {
    public:
      void init(Color const & color) {
        Animator::init();
        _color = color;
      }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest)
          const {
        dest->fill_solid(_color);
      }

    private:
      Color _color;
    };


    class GlowBackgroundAnimator : public Lights::Animator {
    public:
      void init(ShortTimeSpan period, Color const & color) {
        Animator::init(period, EB_LOOP);
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


    class StalkBubbleAnimator : public Lights::Animator {
    public:
      void init(ShortTimeSpan duration, Color const & color) {
        Animator::init(duration);
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


    class ScoreboardSplashAnimator : public Lights::Animator {
    public:
      void init() { Animator::init(); }

      void set_best_score(uint32_t best_score) { _best_score = best_score; }

    protected:
      int32_t _best_score;

      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const {
        dest->copy(&logo_tex);
        dest->char_5x5_mask(5, 0, "BEST", &orange_5x5_grad_tex);
        char buf[7];
        snprintf(buf, sizeof buf, "%d", (int)_best_score);
        dest->char_5x5_mask((SCOREBOARD_WIDTH - strlen(buf) * 5) / 2, 23,
          buf, &orange_5x5_grad_tex);
      }
    };


    class ScoreboardScrollingTextAnimator : public Lights::Animator {
    public:
      ScoreboardScrollingTextAnimator(char const (* text)[7],
          size_t line_count, Texture2D const * tex)
          : _text(text), _line_count(line_count), _tex(tex) { }

      void init() {
        _pixel_height = _line_count * 5 + SCOREBOARD_HEIGHT * 2;
        Animator::init(ShortTimeSpan::from_millis(
          (_pixel_height * 1000) / SCOREBOARD_HEIGHT), EB_PAUSE);
      }

    protected:
      char const (* _text)[7];
      size_t _pixel_height;
      size_t _line_count;
      Texture2D const * _tex;

      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const {
        dest->fill_solid(Color(0.25f, 0.0f, 0.0f));
        scroll_scoreboard_text(dest, SCOREBOARD_HEIGHT - a * _pixel_height,
          _text, _line_count, _tex);
      }
    };


    class ScoreboardScoreAnimator : public Lights::Animator {
    public:
      void init() {
        Animator::init(TimeSpan::zero, EB_PAUSE);
        _score = 0;
        _best_score = 0;
      }

      void set_score(int32_t score) { _score = score; }
      void set_best_score(int32_t best_score) { _best_score = best_score; }

    protected:
      int32_t _score;
      int32_t _best_score;

      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const {
        char buf[7];
        snprintf(buf, sizeof buf, "%d", (int)_score);
        dest->char_10x15_solid(scoreboard_center(buf, 10), 2, buf,
          Color::white);
        snprintf(buf, sizeof buf, "%d", (int)_best_score);
        dest->char_5x5_mask(5, 18, "BEST", &orange_5x5_grad_tex);
        dest->char_5x5_mask(scoreboard_center(buf, 5), 23, buf,
          &orange_5x5_grad_tex);
      }
    };


    class ScoreboardGameOverWinAnimator : public Lights::Animator {
    public:
      void init() {
        Animator::init();
      }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const {
        dest->fill_solid(Color(0.2f, 0.2f, 0.0f));
        dest->char_5x5_mask( 7, 5, "YOU", &orange_5x5_grad_tex);
        dest->char_5x5_mask( 7, 10, "ARE", &orange_5x5_grad_tex);
        dest->char_5x5_mask( 5, 15, "AMA-", &bright_orange_5x5_grad_tex);
        dest->char_5x5_mask( 2, 20, "ZING!", &bright_orange_5x5_grad_tex);
      }
    };


    class ScoreboardGameOverLoseAnimator : public Lights::Animator {
    public:
      void init() { Animator::init(); }

      void set_new_best_score(int32_t new_best_score)
        { _new_best_score = new_best_score; }

    protected:
      int32_t _new_best_score;

      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const {
        dest->fill_solid(Color::black);
        if(_new_best_score > 0) {
          dest->char_5x5_mask( 2, 5, "WRONG", &orange_5x5_grad_tex);
          dest->char_5x5_mask( 7, 10, "NEW", &orange_5x5_grad_tex);
          dest->char_5x5_mask( 2, 15, "BEST!", &orange_5x5_grad_tex);

          char buf[7];
          snprintf(buf, sizeof buf, "%d", (int)_new_best_score);
          dest->char_5x5_mask((SCOREBOARD_WIDTH - strlen(buf) * 5) / 2, 20,
            buf, &orange_5x5_grad_tex);
        }
        else {
          dest->char_5x5_mask( 2, 10, "WRONG", &orange_5x5_grad_tex);
        }
      }
    };


    class Vis {
    public:
      virtual ~Vis() { }

      virtual void reset() { }
      virtual void update(ShortTimeSpan dt) { }
      virtual void game_start() { }
      virtual void play_color(char id) { }
      virtual void press_color(char id, bool correct) { }
      virtual void await_press() { }
      virtual void score_change(int32_t score) { }
      virtual void game_over_win() { }
      virtual void game_over_lose() { }
      virtual void game_stop() { }
      virtual void best_score_change(int32_t best_score) { }
    };


    class MultiVis : public Vis {
    public:
      MultiVis() : _count(0) { }

      virtual void reset() { foreach_vis([=] (Vis * vis) { vis->reset(); }); }
      virtual void update(ShortTimeSpan dt)
        { foreach_vis([=] (Vis * vis) { vis->update(dt); }); }
      virtual void game_start()
        { foreach_vis([=] (Vis * vis) { vis->game_start(); }); }
      virtual void play_color(char id)
        { foreach_vis([=] (Vis * vis) { vis->play_color(id); }); }
      virtual void press_color(char id, bool correct)
        { foreach_vis([=] (Vis * vis) { vis->press_color(id, correct); }); }
      virtual void await_press()
        { foreach_vis([=] (Vis * vis) { vis->await_press(); }); }
      virtual void score_change(int32_t score)
        { foreach_vis([=] (Vis * vis) { vis->score_change(score); }); }
      virtual void game_over_win()
        { foreach_vis([=] (Vis * vis) { vis->game_over_win(); }); }
      virtual void game_over_lose()
        { foreach_vis([=] (Vis * vis) { vis->game_over_lose(); }); }
      virtual void game_stop()
        { foreach_vis([=] (Vis * vis) { vis->game_stop(); }); }
      virtual void best_score_change(int32_t best_score) {
        foreach_vis([=] (Vis * vis) { vis->best_score_change(best_score); });
      }

      void add_vis(Vis * vis) {
        _vis_list[_count++] = vis;
      }

    private:
      static size_t const VIS_MAX = 10;
      Vis * _vis_list[VIS_MAX];
      size_t _count;

      template<typename F>
      void foreach_vis(F const & f) {
        for(size_t i = 0; i < VIS_MAX; ++i) {
          if(_vis_list[i] != nullptr) {
            f(_vis_list[i]);
          }
        }
      }
    };


    class StalkVis : public Vis {
    public:
      StalkVis(char id, size_t layer_start, ShortTimeSpan period,
          Color const & bg_color)
          : _id(id), _layer_start(layer_start), _in_game_over(false) {
        _active_color.init(period, bg_color);
        _muted_color.init(period, bg_color.lerp(Color::black, 0.5f));
        _bubble_pool.init([=] (StalkBubbleAnimator * anim) {
          anim->init(ShortTimeSpan::from_millis(500), Color::white);
        });
        _game_over_win.init(Color::white);
        _game_over_lose.init(Color::red);
      }

      virtual void reset() {
        Lights::start_animator(_layer_start + Lights::LAYER_STALK_BASE_COLOR,
          &_active_color);
        if(_in_game_over) {
          _in_game_over = false;
          Lights::transition_out(_layer_start + Lights::LAYER_STALK_OVERLAY,
            ShortTimeSpan::from_millis(2000));
        } else {
          Lights::transition_out(_layer_start + Lights::LAYER_STALK_OVERLAY,
            TimeSpan::zero);
        }
      }

      virtual void play_color(char id) {
        if(id == _id) {
          Lights::start_animator(_layer_start + Lights::LAYER_STALK_BUBBLE,
            _bubble_pool.acquire());
          Lights::start_animator(_layer_start + Lights::LAYER_STALK_BASE_COLOR,
            &_active_color, ShortTimeSpan::from_micros(100));
        }
        else {
          Lights::start_animator(_layer_start + Lights::LAYER_STALK_BASE_COLOR,
            &_muted_color, ShortTimeSpan::from_micros(100));
        }
      }

      virtual void await_press() {
        Lights::start_animator(_layer_start + Lights::LAYER_STALK_BASE_COLOR,
          &_active_color, ShortTimeSpan::from_micros(100));
      }

      virtual void press_color(char id, bool correct) {
        play_color(id);
      }

      virtual void game_over_win() {
        Lights::start_animator(
          _layer_start + Lights::LAYER_STALK_OVERLAY, &_game_over_win,
          ShortTimeSpan::from_millis(2000));
      }

      virtual void game_over_lose() {
        Lights::start_animator(
          _layer_start + Lights::LAYER_STALK_OVERLAY, &_game_over_lose,
          ShortTimeSpan::from_millis(2000));
      }

    private:
      char _id;
      size_t _layer_start;
      bool _in_game_over;

      GlowBackgroundAnimator _active_color;
      GlowBackgroundAnimator _muted_color;
      Lights::StaticAnimatorPool<StalkBubbleAnimator, 3> _bubble_pool;
      SolidBackgroundAnimator _game_over_win;
      SolidBackgroundAnimator _game_over_lose;
    };


    class ScoreboardVis : public Vis {
    public:
      ScoreboardVis()
          : _splash_scroll(splash_text,
            sizeof splash_text / sizeof *splash_text, &aqua_5x5_grad_tex),
          _instructions_title_scroll(instructions_title_text,
            sizeof instructions_title_text / sizeof *instructions_title_text,
            &bright_orange_5x5_grad_tex),
          _instructions_scroll(instructions_text,
            sizeof instructions_text / sizeof *instructions_text,
            &orange_5x5_grad_tex) {
      }

      virtual void reset() {
        Lights::start_animator(
          Lights::LAYER_STALK_RED_START + Lights::LAYER_STALK_BASE_COLOR,
          &_red_bg);
        Lights::start_animator(
          Lights::LAYER_STALK_GREEN_START + Lights::LAYER_STALK_BASE_COLOR,
          &_green_bg);
        Lights::start_animator(
          Lights::LAYER_STALK_BLUE_START + Lights::LAYER_STALK_BASE_COLOR,
          &_blue_bg);
        Lights::start_animator(
          Lights::LAYER_STALK_PINK_START + Lights::LAYER_STALK_BASE_COLOR,
          &_pink_bg);
        Lights::start_animator(Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
          &_splash);
      }

      virtual void game_start() {
        _game_over_lose.set_new_best_score(0);
        _score.set_score(0);
        Lights::start_animator(Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
          &_score, ShortTimeSpan::from_millis(250));
      }

      virtual void play_color(char id) {
        switch(id) {
        case 'R':
          Lights::start_animator(
            Lights::LAYER_SB_START + Lights::LAYER_SB_BACKGROUND,
            &_red_bg, ShortTimeSpan::from_millis(100));
          break;
        case 'G':
          Lights::start_animator(
            Lights::LAYER_SB_START + Lights::LAYER_SB_BACKGROUND,
            &_green_bg, ShortTimeSpan::from_millis(100));
          break;
        case 'B':
          Lights::start_animator(
            Lights::LAYER_SB_START + Lights::LAYER_SB_BACKGROUND,
            &_blue_bg, ShortTimeSpan::from_millis(100));
          break;
        case 'P':
          Lights::start_animator(
            Lights::LAYER_SB_START + Lights::LAYER_SB_BACKGROUND,
            &_pink_bg, ShortTimeSpan::from_millis(100));
          break;
        }
      }

      virtual void press_color(char id, bool correct) {
        play_color(id);
      }

      virtual void score_change(int32_t score) {
        _score.set_score(score);
      }

      virtual void game_over_win() {
        Lights::start_animator(
          Lights::LAYER_SB_START + Lights::LAYER_SB_BACKGROUND,
          &_neutral_bg, ShortTimeSpan::from_millis(100));
        Lights::start_animator(
          Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
          &_game_over_win, ShortTimeSpan::from_millis(100));
      }

      virtual void game_over_lose() {
        Lights::start_animator(
          Lights::LAYER_SB_START + Lights::LAYER_SB_BACKGROUND,
          &_neutral_bg, ShortTimeSpan::from_millis(100));
        Lights::start_animator(
          Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
          &_game_over_lose, ShortTimeSpan::from_millis(100));
      }

      virtual void game_stop() {
        Lights::transition_out(
          Lights::LAYER_SB_START + Lights::LAYER_SB_OVERLAY,
          ShortTimeSpan::from_millis(100));
      }

      virtual void best_score_change(int32_t best_score) {
        _score.set_best_score(best_score);
        _splash.set_best_score(best_score);
        _game_over_lose.set_new_best_score(best_score);
      }

    private:
      ScoreboardSplashAnimator _splash;
      ScoreboardScrollingTextAnimator _splash_scroll;
      ScoreboardScrollingTextAnimator _instructions_title_scroll;
      ScoreboardScrollingTextAnimator _instructions_scroll;
      SolidBackgroundAnimator _neutral_bg;
      GlowBackgroundAnimator _red_bg;
      GlowBackgroundAnimator _green_bg;
      GlowBackgroundAnimator _blue_bg;
      GlowBackgroundAnimator _pink_bg;
      ScoreboardScoreAnimator _score;
      ScoreboardGameOverWinAnimator _game_over_win;
      ScoreboardGameOverLoseAnimator _game_over_lose;
    };


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

    StalkVis red_stalk_vis('R', Lights::LAYER_STALK_RED_START,
      ShortTimeSpan::from_millis(1023), Color::red);
    StalkVis green_stalk_vis('G', Lights::LAYER_STALK_GREEN_START,
      ShortTimeSpan::from_millis(1490), Color::green);
    StalkVis blue_stalk_vis('B', Lights::LAYER_STALK_BLUE_START,
      ShortTimeSpan::from_millis(1717), Color::blue);
    StalkVis pink_stalk_vis('P', Lights::LAYER_STALK_PINK_START,
      ShortTimeSpan::from_millis(1201), Color::pink);
    ScoreboardVis scoreboard_vis;
    MultiVis vis;
  }


  void Game::init() {
    init_textures();

    vis.add_vis(&red_stalk_vis);
    vis.add_vis(&green_stalk_vis);
    vis.add_vis(&blue_stalk_vis);
    vis.add_vis(&pink_stalk_vis);
    vis.add_vis(&scoreboard_vis);
  }


  void Game::update(ShortTimeSpan dt) {
    state_timer.update(dt);

    switch(state) {
    case ST_RESET:
      Debug::tracef("Reset");
      pattern_length = 0;
      pattern_pos = 0;
      vis.reset();
      state = ST_LISTENING;
      state_timer.cancel();
      update(TimeSpan::zero);
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
          vis.game_start();
          vis.press_color(input, true);
          vis.score_change(1);
        } else if(input == pattern[pattern_pos]) {
          Debug::tracef("Correct input %c", input);
          vis.press_color(input, true);
          ++pattern_pos;
          if(pattern_pos >= pattern_length) {
            vis.score_change(pattern_length);
            if(pattern_length == PATTERN_LENGTH_MAX) {
              Debug::tracef("Win!!");
              state = ST_GAME_OVER;
              state_timer = aa::Timer(TimeSpan::from_millis(3000), false);
              vis.game_over_win();
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
          vis.press_color(input, false);
          vis.game_over_lose();
          state = ST_GAME_OVER;
          state_timer = aa::Timer(TimeSpan::from_millis(3000), false);
        }
      }
      break;
    }
    case ST_PLAYING:
      if(state_timer.get_time_remaining() <= TimeSpan::zero) {
        if(pattern_pos < pattern_length) {
          Debug::tracef("Play pos %d = %c", pattern_pos, pattern[pattern_pos]);
          vis.play_color(pattern[pattern_pos]);
          ++pattern_pos;
          state_timer = aa::Timer(TimeSpan::from_millis(500), false);
        } else {
          Debug::tracef("Awaiting response");
          state = ST_WAITING_RESPONSE;
          state_timer = aa::Timer(TimeSpan::from_millis(15000), false);
          vis.await_press();
        }
      }
      break;
    case ST_WAITING_RESPONSE:
      if(is_button_pressed()) {
        Debug::tracef("Button press, listening");
        state = ST_LISTENING;
        pattern_pos = 0;
        state_timer.cancel();
        vis.game_over_lose();
        update(TimeSpan::zero);
      } else if(state_timer.get_time_remaining() <= TimeSpan::zero) {
        Debug::tracef("Timeout, game over");
        state = ST_GAME_OVER;
        state_timer = aa::Timer(TimeSpan::from_millis(3000), false);
        vis.game_over_lose();
      }
      break;
    case ST_GAME_OVER:
      if(state_timer.get_time_remaining() <= TimeSpan::zero) {
        vis.game_stop();
        state = ST_RESET;
      }
      /*
      else if(state_timer.get_time_remaining() <= ShortTimeSpan::from_millis(2000)) {
        vis.game_stop();
        state = ST_RESET;
      }
      */
      break;
    default:
      state_timer.cancel();
      vis.game_stop();
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
      return 'R';
    }
    if(Input::button_pressed(Input::B_GREEN)) {
      return 'G';
    }
    if(Input::button_pressed(Input::B_BLUE)) {
      return 'B';
    }
    if(Input::button_pressed(Input::B_PINK)) {
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
}
