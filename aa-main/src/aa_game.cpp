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

      static const Texture2D::ClosedGradient orange_grad = {
        .cx0y0 = { 1.00f, 0.50f, 0.00f, 1.0f },
        .cx1y0 = { 0.75f, 0.25f, 0.00f, 1.0f },
        .cx0y1 = { 0.75f, 0.25f, 0.00f, 1.0f },
        .cx1y1 = { 0.50f, 0.00f, 0.00f, 1.0f },
      };
      static const Texture2D::ClosedGradient bright_orange_grad = {
        .cx0y0 = { 1.00f, 0.75f, 0.25f, 1.0f },
        .cx1y0 = { 0.75f, 0.50f, 0.18f, 1.0f },
        .cx0y1 = { 0.75f, 0.50f, 0.18f, 1.0f },
        .cx1y1 = { 0.50f, 0.25f, 0.12f, 1.0f },
      };
      static const Texture2D::ClosedGradient aqua_grad = {
        .cx0y0 = { 0.00f, 0.50f, 1.00f, 1.0f },
        .cx1y0 = { 0.00f, 0.25f, 0.75f, 1.0f },
        .cx0y1 = { 0.00f, 0.25f, 0.75f, 1.0f },
        .cx1y1 = { 0.00f, 0.00f, 0.50f, 1.0f },
      };

      orange_5x5_grad_tex.box_set(0, 0, 5, 5, orange_grad);
      bright_orange_5x5_grad_tex.box_set(0, 0, 5, 5, bright_orange_grad);
      aqua_5x5_grad_tex.box_set(0, 0, 5, 5, aqua_grad);

      for(size_t y = 0; y < SCOREBOARD_WIDTH; ++y) {
        for(size_t x = 0; x < SCOREBOARD_WIDTH; ++x) {
          switch(logo_image[y * SCOREBOARD_WIDTH + x]) {
            case 'W': logo_tex.set(x, y, Color::make(1.0f, 1.0f, 1.0f)); break;
            case 'w': logo_tex.set(x, y, Color::make(0.5f, 0.5f, 0.5f)); break;
            case ',': logo_tex.set(x, y, Color::make(0.2f, 0.2f, 0.2f)); break;
            case '.': logo_tex.set(x, y, Color::make(0.1f, 0.1f, 0.1f)); break;
            case 'R': logo_tex.set(x, y, Color::make(1.0f, 0.0f, 0.0f)); break;
            case 'r': logo_tex.set(x, y, Color::make(0.5f, 0.0f, 0.0f)); break;
            case ' ': logo_tex.set(x, y, Color::make(0.0f, 0.0f, 0.0f, 0.0f)); break;
            default:  logo_tex.set(x, y, Color::make(1.0f, 0.0f, 1.0f)); break;
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
      static const size_t line_height = 7;
      static const size_t line_offset = (line_height - 5) / 2;
      static const size_t temp_height =
        ((SCOREBOARD_HEIGHT + line_height - 1) / line_height + 1) *
        line_height;
      static AutoTexture2D<SCOREBOARD_WIDTH, temp_height> temp;
      int32_t h = line_count * line_height;

      if((y + h <= 0) || (y >= (int32_t)SCOREBOARD_HEIGHT)) {
        return;
      }

      size_t y_ofs;
      size_t begin_line;
      size_t begin_y;
      if(y > 0) {
        y_ofs = (line_height - 1) - ((y - 1) % line_height);
        begin_line = 0;
        begin_y = y + y_ofs;
      }
      else {
        y_ofs = (-y) % line_height;
        begin_line = -y / line_height;
        begin_y = 0;
      }

      size_t end_line =
        (SCOREBOARD_HEIGHT + (line_height - 1) - y) / line_height;
      if(end_line > line_count) {
        end_line = line_count;
      }

      temp.fill_set(Color::transparent);
      for(size_t line = begin_line; line < end_line; ++line) {
        temp.char_5x5_set(scoreboard_center(text[line], 5),
          begin_y + (line - begin_line) * line_height + line_offset,
          text[line], tex);
      }
      dest->fill_mix(&temp, 0, y_ofs);
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
        dest->fill_set(_color);
      }

    private:
      Color _color;
    };


    template<class F>
    class FillSetAnimator : public Lights::Animator {
    public:
      FillSetAnimator(F const & color_fun) : _color_fun(color_fun) { }

      void init(ShortTimeSpan period, EndBehavior end_behavior) {
        Animator::init(period, end_behavior);
      }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest)
          const {
        dest->fill_set(_color_fun(a));
      }

    private:
      F _color_fun;
    };


    template<class F>
    class FillMixAnimator : public Lights::Animator {
    public:
      FillMixAnimator(F const & color_fun) : _color_fun(color_fun) { }

      void init(ShortTimeSpan period, EndBehavior end_behavior) {
        Animator::init(period, end_behavior);
      }

    protected:
      virtual void render(ShortTimeSpan t, float a, Texture2D * dest)
          const {
        dest->fill_mix(_color_fun(a));
      }

    private:
      F _color_fun;
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
        dest->fill_bubble_x(a * 45.0f, 10.0f, _color);
      }

    private:
      Color _color;
    };


    class ScoreboardSplashAnimator : public Lights::Animator {
    public:
      void init(ShortTimeSpan duration) {
        Animator::init(duration, EB_PAUSE);
      }

      void set_best_score(uint32_t best_score) { _best_score = best_score; }

    protected:
      int32_t _best_score;

      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const {
        dest->fill_set(&logo_tex);
        dest->box_lerp(0, 22, 30, 7, Color::make(0.0f, 0.0f, 0.25f), 0.5f);
        char buf[7];
        snprintf(buf, sizeof buf, "HI:%d", (int)_best_score);
        dest->char_5x5_set(scoreboard_center(buf, 5), 23, buf,
          &orange_5x5_grad_tex);
      }
    };


    class ScoreboardScrollingTextAnimator : public Lights::Animator {
    public:
      ScoreboardScrollingTextAnimator(char const (* text)[7],
          size_t line_count, Texture2D const * tex)
          : _text(text), _line_count(line_count), _tex(tex) { }

      void init(ShortTimeSpan duration_per_screen) {
        _pixel_height = _line_count * 7 + SCOREBOARD_HEIGHT * 2;
        Animator::init((_pixel_height * duration_per_screen)
            / SCOREBOARD_HEIGHT, EB_PAUSE);
      }

    protected:
      char const (* _text)[7];
      size_t _pixel_height;
      size_t _line_count;
      Texture2D const * _tex;

      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const {
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
        dest->char_10x15_set(scoreboard_center(buf, 10), 2, buf,
          Color::white);
        dest->box_lerp(0, 22, 30, 7, Color::black, 0.5f);
        snprintf(buf, sizeof buf, "%d", (int)_best_score);
        dest->char_5x5_set(scoreboard_center(buf, 5), 23, buf,
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
        dest->fill_set(Color::make(0.2f, 0.2f, 0.0f));
        dest->char_5x5_set( 7, 5, "YOU", &orange_5x5_grad_tex);
        dest->char_5x5_set( 7, 10, "ARE", &orange_5x5_grad_tex);
        dest->char_5x5_set( 5, 15, "AMA-", &bright_orange_5x5_grad_tex);
        dest->char_5x5_set( 2, 20, "ZING!", &bright_orange_5x5_grad_tex);
      }
    };


    class ScoreboardGameOverLoseAnimator : public Lights::Animator {
    public:
      void init() {
        Animator::init();
      }

      void set_new_best_score(int32_t new_best_score) {
        _new_best_score = new_best_score;
      }

    protected:
      int32_t _new_best_score;

      virtual void render(ShortTimeSpan t, float a, Texture2D * dest) const {
        dest->fill_set(Color::black);
        if(_new_best_score > 0) {
          dest->char_5x5_set(2, 2, "WRONG", &bright_orange_5x5_grad_tex);
          dest->char_5x5_set(0, 9, "NEW HI", &orange_5x5_grad_tex);
          dest->char_5x5_set(0, 16, "SCORE:", &orange_5x5_grad_tex);

          char buf[7];
          snprintf(buf, sizeof buf, "%d", (int)_new_best_score);
          dest->char_5x5_set(scoreboard_center(buf, 5), 23, buf,
            &bright_orange_5x5_grad_tex);
        }
        else {
          dest->char_5x5_set( 2, 12, "WRONG", &bright_orange_5x5_grad_tex);
        }
      }
    };


    class Vis {
    public:
      virtual ~Vis() { }

      virtual void attract_start() { }
      virtual void update(ShortTimeSpan dt) { }
      virtual void game_start() { }
      virtual void play_pattern() { }
      virtual void play_color(char id) { }
      virtual void await_press() { }
      virtual void press_color(char id, bool correct) { }
      virtual void score_change(int32_t score) { }
      virtual void game_over_win() { }
      virtual void game_over_lose() { }
      virtual void best_score_change(int32_t best_score) { }
    };


    class MultiVis : public Vis {
    public:
      MultiVis() : _count(0) { }

      virtual void attract_start() {
        Debug::tracef("vis: attract_start");
        foreach_vis([=] (Vis * vis) { vis->attract_start(); });
      }
      virtual void update(ShortTimeSpan dt) {
        //Debug::tracef("vis: update dt=%ldus", (int32_t)dt.to_micros());
        foreach_vis([=] (Vis * vis) { vis->update(dt); });
      }
      virtual void game_start() {
        Debug::tracef("vis: game_start");
        foreach_vis([=] (Vis * vis) { vis->game_start(); });
      }
      virtual void play_pattern() {
        Debug::tracef("vis: play_pattern");
        foreach_vis([=] (Vis * vis) { vis->play_pattern(); });
      }
      virtual void play_color(char id) {
        Debug::tracef("vis: play_color id=%c", id);
        foreach_vis([=] (Vis * vis) { vis->play_color(id); });
      }
      virtual void await_press() {
        Debug::tracef("vis: await_press");
        foreach_vis([=] (Vis * vis) { vis->await_press(); });
      }
      virtual void press_color(char id, bool correct) {
        Debug::tracef("vis: press_color id=%c correct=%c", id, correct ? 't' : 'f');
        foreach_vis([=] (Vis * vis) { vis->press_color(id, correct); });
      }
      virtual void score_change(int32_t score) {
        Debug::tracef("vis: score_change score=%ld", score);
        foreach_vis([=] (Vis * vis) { vis->score_change(score); });
      }
      virtual void game_over_win() {
        Debug::tracef("vis: game_over_win");
        foreach_vis([=] (Vis * vis) { vis->game_over_win(); });
      }
      virtual void game_over_lose() {
        Debug::tracef("vis: game_over_lose");
        foreach_vis([=] (Vis * vis) { vis->game_over_lose(); });
      }
      virtual void best_score_change(int32_t best_score) {
        Debug::tracef("vis: best_score_change best_score=%ld", best_score);
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
          : _id(id), _layer_start(layer_start), _in_game_over(false),
          _active_color(GlowBackgroundFadeFunction(bg_color)),
          _mute(MuteFunction()) {
        _active_color.init(period, Lights::Animator::EB_LOOP);
        _mute.init(TimeSpan::zero, Lights::Animator::EB_PAUSE);
        _bubble_pool.init([=] (StalkBubbleAnimator * anim) {
          anim->init(ShortTimeSpan::from_millis(500), Color::white);
        });
        _game_over_win.init(Color::white);
        _game_over_lose.init(Color::red);
      }

      virtual void attract_start() {
        Lights::start_animator(_layer_start + Lights::LAYER_STALK_BASE_COLOR,
          &_active_color, ShortTimeSpan::from_millis(500));
        Lights::transition_out(_layer_start + Lights::LAYER_STALK_BUBBLE,
          ShortTimeSpan::from_millis(100));
        Lights::transition_out(_layer_start + Lights::LAYER_STALK_OVERLAY,
          ShortTimeSpan::from_millis(2000));
      }

      virtual void play_pattern() {
        Lights::start_animator(_layer_start + Lights::LAYER_STALK_BUBBLE,
            &_mute, ShortTimeSpan::from_millis(100));
      }

      virtual void play_color(char id) {
        if(id == _id) {
          Lights::start_animator(_layer_start + Lights::LAYER_STALK_BUBBLE,
            _bubble_pool.acquire());
        }
        else {
          Lights::start_animator(_layer_start + Lights::LAYER_STALK_BUBBLE,
            &_mute, ShortTimeSpan::from_millis(100));
        }
      }

      virtual void await_press() {
        Lights::transition_out(_layer_start + Lights::LAYER_STALK_BUBBLE,
          ShortTimeSpan::from_millis(100));
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
      struct GlowBackgroundFadeFunction {
        Color _color;

        GlowBackgroundFadeFunction(Color color) : _color(color) { }

        Color operator ()(float a) const {
          float aa = 0.10f * cosf((float)M_PI * 2.0f * (a + 0.5f)) + 0.90f;
          return _color.cie_scale(aa);
        }
      };

      struct MuteFunction {
        Color operator ()(float a) const {
          float aa;

          if(a < 0.0f) aa = 0.0f;
          else if(a < 0.2f) aa = a / 0.2f;
          else if(a < 0.8f) aa = 1.0f;
          else if(a < 1.0f) aa = (1.0f - a) / 0.2f;
          else aa = 0.0f;

          return Color::make(0.0f, 0.0f, 0.0f, 0.5f * aa);
        }
      };

      char _id;
      size_t _layer_start;
      bool _in_game_over;

      FillSetAnimator<GlowBackgroundFadeFunction> _active_color;
      FillMixAnimator<MuteFunction> _mute;
      Lights::StaticAnimatorPool<StalkBubbleAnimator, 3> _bubble_pool;
      SolidBackgroundAnimator _game_over_win;
      SolidBackgroundAnimator _game_over_lose;
    };


    class ScoreboardVis : public Vis {
    public:
      ScoreboardVis()
          : _state(AS_NONE), _pattern_count(0),
          _splash_title(splash_text,
            sizeof splash_text / sizeof *splash_text, &aqua_5x5_grad_tex),
          _instructions_title_scroll(instructions_title_text,
            sizeof instructions_title_text / sizeof *instructions_title_text,
            &bright_orange_5x5_grad_tex),
          _instructions_scroll(instructions_text,
            sizeof instructions_text / sizeof *instructions_text,
            &orange_5x5_grad_tex),
          _red_pulse(PulseFunction(Color::red)),
          _green_pulse(PulseFunction(Color::green)),
          _blue_pulse(PulseFunction(Color::blue)),
          _pink_pulse(PulseFunction(Color::pink)) {
        _splash.init(ShortTimeSpan::from_millis(10000));
        _splash_title.init(ShortTimeSpan::from_millis(3000));
        _instructions_title_scroll.init(ShortTimeSpan::from_millis(3000));
        _instructions_scroll.init(ShortTimeSpan::from_millis(3000));
        _neutral_bg.init(Color::black);
        _red_pulse.init(ShortTimeSpan::from_millis(500),
          Lights::Animator::EB_STOP);
        _green_pulse.init(ShortTimeSpan::from_millis(500),
          Lights::Animator::EB_STOP);
        _blue_pulse.init(ShortTimeSpan::from_millis(500),
          Lights::Animator::EB_STOP);
        _pink_pulse.init(ShortTimeSpan::from_millis(500),
          Lights::Animator::EB_STOP);
        _score.init();
        _game_over_win.init();
        _game_over_lose.init();
      }

      virtual void attract_start() {
        _state = AS_SPLASH;

        Lights::start_animator(
          Lights::LAYER_SB_START + Lights::LAYER_SB_BACKGROUND,
          &_neutral_bg, ShortTimeSpan::from_millis(100));
        Lights::start_animator(Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
          &_splash, ShortTimeSpan::from_millis(100));
        Lights::transition_out(
          Lights::LAYER_SB_START + Lights::LAYER_SB_OVERLAY,
          ShortTimeSpan::from_millis(100));
      }

      virtual void update(ShortTimeSpan dt) {
        switch(_state) {
        case AS_SPLASH:
          if(_splash.is_at_end()) {
            _state = AS_SPLASH_TITLE;
            Lights::transition_out(
              Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
              ShortTimeSpan::from_millis(3000));
            Lights::start_animator(
              Lights::LAYER_SB_START + Lights::LAYER_SB_OVERLAY,
              &_splash_title,
              ShortTimeSpan::from_millis(100));
          }
          break;
        case AS_SPLASH_TITLE:
          if(_splash_title.is_at_end()) {
            _state = AS_INSTRUCTIONS_TITLE;
            Lights::start_animator(
              Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
              &_instructions_title_scroll,
              ShortTimeSpan::from_millis(100));
            Lights::transition_out(
              Lights::LAYER_SB_START + Lights::LAYER_SB_OVERLAY,
              ShortTimeSpan::from_millis(100));
          }
          break;
        case AS_INSTRUCTIONS_TITLE:
          if(_instructions_title_scroll.is_at_end()) {
            _state = AS_INSTRUCTIONS;
            Lights::start_animator(
              Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
              &_instructions_scroll);
          }
          break;
        case AS_INSTRUCTIONS:
          if(_instructions_scroll.is_at_end()) {
            _state = AS_SPLASH;
            Lights::start_animator(
              Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
              &_splash, ShortTimeSpan::from_millis(100));
          }
          break;
        default:
          break;
        }
      }

      virtual void game_start() {
        _state = AS_NONE;

        _game_over_lose.set_new_best_score(0);
        _score.set_score(0);
        Lights::start_animator(Lights::LAYER_SB_START + Lights::LAYER_SB_OVERLAY,
          &_score, ShortTimeSpan::from_millis(100));
        Lights::transition_out(
          Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
          ShortTimeSpan::from_millis(100));
      }

      virtual void play_pattern() {
        _pattern_count = 0;
        _score.set_score(_pattern_count);

        Lights::start_animator(
          Lights::LAYER_SB_START + Lights::LAYER_SB_BACKGROUND,
          &_neutral_bg, ShortTimeSpan::from_millis(100));
        Lights::transition_out(
          Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
          ShortTimeSpan::from_millis(100));
      }

      virtual void play_color(char id) {
        ++_pattern_count;
        _score.set_score(_pattern_count);

        switch(id) {
        case 'R':
          Lights::start_animator(
            Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
            &_red_pulse, ShortTimeSpan::from_millis(100));
          break;
        case 'G':
          Lights::start_animator(
            Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
            &_green_pulse, ShortTimeSpan::from_millis(100));
          break;
        case 'B':
          Lights::start_animator(
            Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
            &_blue_pulse, ShortTimeSpan::from_millis(100));
          break;
        case 'P':
          Lights::start_animator(
            Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
            &_pink_pulse, ShortTimeSpan::from_millis(100));
          break;
        }
      }

      virtual void await_press() {
        int32_t last_pattern_count = _pattern_count;
        play_pattern();
        _score.set_score(last_pattern_count);
      }

      virtual void press_color(char id, bool correct) {
        play_color(id);
      }

      virtual void score_change(int32_t score) {
        _score.set_best_score(score);
      }

      virtual void game_over_win() {
        Lights::start_animator(
          Lights::LAYER_SB_START + Lights::LAYER_SB_BACKGROUND,
          &_neutral_bg, ShortTimeSpan::from_millis(100));
        Lights::start_animator(
          Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
          &_game_over_win, ShortTimeSpan::from_millis(100));
        Lights::transition_out(
          Lights::LAYER_SB_START + Lights::LAYER_SB_OVERLAY,
          ShortTimeSpan::from_millis(100));
      }

      virtual void game_over_lose() {
        Lights::start_animator(
          Lights::LAYER_SB_START + Lights::LAYER_SB_BACKGROUND,
          &_neutral_bg, ShortTimeSpan::from_millis(100));
        Lights::start_animator(
          Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
          &_game_over_lose, ShortTimeSpan::from_millis(100));
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
      struct PulseFunction {
        Color _color;

        PulseFunction(Color color) : _color(color) { }

        Color operator ()(float a) const {
          float aa;
          if(a < 0.10f) {
            aa = (a - 0.00f) * (1.0f / 0.10f) * 0.4f + 0.6f;
          }
          else if(a < 0.30f) {
            aa = (0.30f - a) * (1.0f / 0.20f) * 0.4f + 0.6f;
          }
          else if(a < 0.80f) {
            aa = 0.6f;
          }
          else {
            aa = (1.00f - a) * (1.0f / 0.20f) * 0.6f + 0.0f;
          }
          return _color.cie_scale(aa);
        }
      };

      enum AttractState {
        AS_NONE,
        AS_SPLASH,
        AS_SPLASH_TITLE,
        AS_INSTRUCTIONS_TITLE,
        AS_INSTRUCTIONS,
      };

      AttractState _state;
      int32_t _pattern_count;

      ScoreboardSplashAnimator _splash;
      ScoreboardScrollingTextAnimator _splash_title;
      ScoreboardScrollingTextAnimator _instructions_title_scroll;
      ScoreboardScrollingTextAnimator _instructions_scroll;
      SolidBackgroundAnimator _neutral_bg;
      FillSetAnimator<PulseFunction> _red_pulse;
      FillSetAnimator<PulseFunction> _green_pulse;
      FillSetAnimator<PulseFunction> _blue_pulse;
      FillSetAnimator<PulseFunction> _pink_pulse;
      ScoreboardScoreAnimator _score;
      ScoreboardGameOverWinAnimator _game_over_win;
      ScoreboardGameOverLoseAnimator _game_over_lose;
    };


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


    char get_random_button() {
      switch(rand() % 4) {
      case 0: return 'R';
      case 1: return 'G';
      case 2: return 'B';
      case 3: return 'P';
      default: return 0;
      }
    }


    class GameState {
    public:
      GameState(char const * name, ShortTimeSpan timeout)
          : _name(name), _timer(timeout, true) { }

      char const * get_name() const { return _name; }
      Timer * get_timer() { return &_timer; }
      GameState * get_next_state() const { return _next_state; }

      void reset() {
        _next_state = nullptr;
        _timer.restart();
      }

      virtual void on_enter() { }
      virtual bool on_button(char id) { return true; }
      virtual void on_timeout() { }
      virtual void on_update(ShortTimeSpan dt) { }
      virtual void on_exit() { }

    protected:
      void request_transition(GameState * next_state) {
        _next_state = next_state;
      }

    private:
      char const * _name;
      Timer _timer;
      GameState * _next_state;
    };


    extern GameState * initial_state;
    extern GameState * attract_state;
    extern GameState * pause_before_play_state;
    extern GameState * play_pattern_state;
    extern GameState * await_response_state;
    extern GameState * pause_before_game_over_state;
    extern GameState * game_over_state;


    size_t const PATTERN_LENGTH_MAX = 999;
    char pattern[PATTERN_LENGTH_MAX];
    size_t pattern_length;
    size_t score;
    size_t best_score;
    aa::Timer state_timer(TimeSpan::infinity, false);
    GameState * current_state;


    class InitialGameState : public GameState {
    public:
      InitialGameState() : GameState("initial", TimeSpan::infinity) { }
      virtual void on_enter() {
        pattern_length = 0;
        score = 0;
        best_score = 0;
        vis.score_change(0);
        vis.best_score_change(0);
        vis.attract_start();
        request_transition(attract_state);
      }
    } initial_state_instance;


    class AttractGameState : public GameState {
    public:
      enum AttractSubstate {
        AT_DELAY_TOP,
        AT_SCROLL_DOWN,
        AT_DELAY_BOTTOM,
        AT_SCROLL_UP,
        AT_STATIC_IMAGE_0,
        AT_STATIC_IMAGE_1,
      };

      AttractGameState()
        : GameState("attract", ShortTimeSpan::from_millis(30000)) { }
      virtual void on_enter() {
        vis.attract_start();
      }
      virtual bool on_button(char id) {
        pattern[0] = id;
        pattern_length = 1;
        request_transition(await_response_state);
        // returning false will cause the button press to be processed again
        // by the next state, basically we're simulating jumping into the game
        // right after playing a sequence of 1 button that happens to be the
        // button the user is about to press
        return false;
      }
      virtual void on_exit() {
        vis.game_start();
      }
    } attract_state_instance;


    class PauseBeforePlayGameState : public GameState {
    public:
      PauseBeforePlayGameState()
        : GameState("prepare pattern", ShortTimeSpan::from_millis(1000)) { }
      virtual void on_timeout() {
        request_transition(play_pattern_state);
      }
    } pause_before_play_state_instance;


    class PlayPatternGameState : public GameState {
    public:
      PlayPatternGameState()
        : GameState("play pattern", ShortTimeSpan::from_millis(500)) { }
      virtual void on_enter() {
        vis.play_pattern();
        Debug::assertf(AA_AUTO_ASSERT(pattern_length > 0));
        vis.play_color(pattern[0]);
        _pattern_pos = 1;
      }
      virtual void on_timeout() {
        if(_pattern_pos < pattern_length) {
          vis.play_color(pattern[_pattern_pos]);
          ++_pattern_pos;
        }
        else {
          request_transition(await_response_state);
        }
      }
    private:
      size_t _pattern_pos;
    } play_pattern_state_instance;


    class AwaitResponseGameState : public GameState {
    public:
      AwaitResponseGameState()
        : GameState("awaiting response",
          ShortTimeSpan::from_millis(20000)) { }
      virtual void on_enter() {
        vis.await_press();
        Debug::assertf(AA_AUTO_ASSERT(pattern_length > 0));
        _pattern_pos = 0;
      }
      virtual bool on_button(char id) {
        Debug::assertf(AA_AUTO_ASSERT(_pattern_pos < pattern_length));
        if(id == pattern[_pattern_pos]) {
          vis.press_color(id, true);
          ++_pattern_pos;
          if(_pattern_pos >= pattern_length) {
            score = pattern_length;
            vis.score_change(score);
            if(pattern_length >= PATTERN_LENGTH_MAX) {
              request_transition(pause_before_game_over_state);
            }
            else {
              pattern[pattern_length] = get_random_button();
              ++pattern_length;
              request_transition(pause_before_play_state);
            }
          }
          // else, do nothing and wait for the next button press
          get_timer()->restart();
        }
        else {
          vis.press_color(id, false);
          request_transition(pause_before_game_over_state);
        }
        return true;
      }
      virtual void on_timeout() {
        request_transition(game_over_state);
      }
    private:
      size_t _pattern_pos;
    } await_response_state_instance;


    class PauseBeforeGameOverGameState : public GameState {
    public:
      PauseBeforeGameOverGameState()
        : GameState("pause before game over",
          ShortTimeSpan::from_millis(500)) { }
      virtual void on_timeout() {
        request_transition(game_over_state);
      }
    } pause_before_game_over_state_instance;


    class GameOverGameState : public GameState {
    public:
      GameOverGameState()
        : GameState("game over", ShortTimeSpan::from_millis(4000)) { }
      virtual void on_enter() {
        if(score > best_score) {
          best_score = score;
          vis.best_score_change(best_score);
        }
        if(score >= PATTERN_LENGTH_MAX) {
          vis.game_over_win();
        }
        else {
          vis.game_over_lose();
        }
      }
      virtual void on_timeout() { request_transition(attract_state); }
    } game_over_state_instance;


    GameState * initial_state = &initial_state_instance;
    GameState * attract_state = &attract_state_instance;
    GameState * pause_before_play_state = &pause_before_play_state_instance;
    GameState * play_pattern_state = &play_pattern_state_instance;
    GameState * await_response_state = &await_response_state_instance;
    GameState * pause_before_game_over_state =
      &pause_before_game_over_state_instance;
    GameState * game_over_state = &game_over_state_instance;
  }


  void Game::init() {
    init_textures();

    vis.add_vis(&red_stalk_vis);
    vis.add_vis(&green_stalk_vis);
    vis.add_vis(&blue_stalk_vis);
    vis.add_vis(&pink_stalk_vis);
    vis.add_vis(&scoreboard_vis);
    current_state = initial_state;
    current_state->reset();
    current_state->on_enter();
  }


  void Game::update(ShortTimeSpan dt) {
    current_state->get_timer()->update(dt);
    //Debug::tracef("Update: %ldus -> %lldus/%lldus, %d", dt.to_micros(),
    //  current_state->get_timer()->get_time().to_micros(),
    //  current_state->get_timer()->get_period().to_micros(),
    //  current_state->get_timer()->peek_periods());

    uint32_t buttons = Input::all_pressed();
    int32_t periods = current_state->get_timer()->read_periods();
    bool done = false;
    do {
      if(current_state->get_next_state() != nullptr) {
        //Debug::tracef("[%s]: -> [%s]",
        //  current_state->get_name(),
        //  current_state->get_next_state()->get_name());
        current_state->on_exit();
        current_state = current_state->get_next_state();
        current_state->reset();
        periods = 0;
        current_state->on_enter();
      }
      else if(dt > TimeSpan::zero) {
        current_state->on_update(dt);
        dt = TimeSpan::zero;
      }
      else if((buttons & Input::B_RED) != 0) {
        if(current_state->on_button('R')) {
          //Debug::tracef("[%s]: button R", current_state->get_name());
          buttons &= ~Input::B_RED;
        }
      }
      else if((buttons & Input::B_GREEN) != 0) {
        if(current_state->on_button('G')) {
          buttons &= ~Input::B_GREEN;
          //Debug::tracef("[%s]: button G", current_state->get_name());
        }
      }
      else if((buttons & Input::B_BLUE) != 0) {
        if(current_state->on_button('B')) {
          buttons &= ~Input::B_BLUE;
          //Debug::tracef("[%s]: button B", current_state->get_name());
        }
      }
      else if((buttons & Input::B_PINK) != 0) {
        if(current_state->on_button('P')) {
          buttons &= ~Input::B_PINK;
          //Debug::tracef("[%s]: button P", current_state->get_name());
        }
      }
      else if(periods > 0) {
        //Debug::tracef("[%s]: timeout", current_state->get_name());
        current_state->on_timeout();
        --periods;
      }
      else {
        done = true;
      }
    } while(!done);

    vis.update(dt);
  }
}
