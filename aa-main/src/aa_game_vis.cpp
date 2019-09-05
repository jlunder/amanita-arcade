#include "amanita_arcade.h"

#include "aa_game_vis.h"

#include "aa_game_hi_score.h"
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
    char const instructions_short_text[3][SCOREBOARD_WIDTH / 5 + 1] = {
      "REPEAT",
      "AFTER",
      "ME!",
    };
    char const instructions_long_text[29][SCOREBOARD_WIDTH / 5 + 1] = {
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
      // Round up so the text tends to round rightward, because the font is
      // weighted leftward
      return (SCOREBOARD_WIDTH - strlen(str) * char_w + 1) / 2;
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
      void render(ShortTimeSpan t, float a, Texture2D * dest) const override {
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
      void render(ShortTimeSpan t, float a, Texture2D * dest) const override {
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
      void render(ShortTimeSpan t, float a, Texture2D * dest) const override {
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
      void render(ShortTimeSpan t, float a, Texture2D * dest) const override {
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

    protected:
      void render(ShortTimeSpan t, float a, Texture2D * dest) const override {
        HiScoreEntry const * todays_hi =
          HiScore::get_scores(HiScore::LIST_TODAYS)[0];
        uint16_t hi_score = 0;
        if(todays_hi != nullptr) {
          hi_score = todays_hi->score;
        }
        dest->fill_set(&logo_tex);
        dest->box_lerp(0, 22, 30, 7, Color::make(0.0f, 0.0f, 0.25f), 0.5f);
        char buf[7];
        snprintf(buf, sizeof buf, "HI:%d", (int)hi_score);
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

      void render(ShortTimeSpan t, float a, Texture2D * dest) const override {
        scroll_scoreboard_text(dest, SCOREBOARD_HEIGHT - a * _pixel_height,
          _text, _line_count, _tex);
      }
    };


    class ScoreboardStaticTextAnimator : public Lights::Animator {
    public:
      ScoreboardStaticTextAnimator(char const (* text)[7],
          size_t line_count, Texture2D const * tex)
          : _text(text), _line_count(line_count), _tex(tex) { }

      void init() {
        _pixel_height = _line_count * 7;
        Animator::init(TimeSpan::zero, EB_PAUSE);
      }

    protected:
      char const (* _text)[7];
      size_t _pixel_height;
      size_t _line_count;
      Texture2D const * _tex;

      void render(ShortTimeSpan t, float a, Texture2D * dest) const override {
        scroll_scoreboard_text(dest,
          (SCOREBOARD_HEIGHT - _pixel_height + 1) / 2,
          _text, _line_count, _tex);
      }
    };


    class ScoreboardScoreAnimator : public Lights::Animator {
    public:
      void init() {
        Animator::init(TimeSpan::zero, EB_PAUSE);
        _score = 0;
      }

      void set_score(int32_t score) { _score = score; }

    protected:
      int32_t _score;

      void render(ShortTimeSpan t, float a, Texture2D * dest) const override {
        HiScoreEntry const * todays_hi =
          HiScore::get_scores(HiScore::LIST_TODAYS)[0];
        uint16_t hi_score = 0;
        if(todays_hi != nullptr) {
          hi_score = todays_hi->score;
        }
        char buf[7];
        snprintf(buf, sizeof buf, "%d", (int)_score);
        dest->char_10x15_set(scoreboard_center(buf, 10), 2, buf,
          Color::white);
        dest->box_lerp(0, 22, 30, 7, Color::black, 0.5f);
        snprintf(buf, sizeof buf, "%d", (int)hi_score);
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
      void render(ShortTimeSpan t, float a, Texture2D * dest) const override {
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

      void set_new_hi_score(int32_t new_hi_score) {
        _new_hi_score = new_hi_score;
      }

    protected:
      int32_t _new_hi_score;

      void render(ShortTimeSpan t, float a, Texture2D * dest) const override {
        dest->fill_set(Color::black);
        if(_new_hi_score > 0) {
          dest->char_5x5_set(2, 2, "WRONG", &bright_orange_5x5_grad_tex);
          dest->char_5x5_set(0, 9, "NEW HI", &orange_5x5_grad_tex);
          dest->char_5x5_set(0, 16, "SCORE:", &orange_5x5_grad_tex);

          char buf[7];
          snprintf(buf, sizeof buf, "%d", (int)_new_hi_score);
          dest->char_5x5_set(scoreboard_center(buf, 5), 23, buf,
            &bright_orange_5x5_grad_tex);
        }
        else {
          dest->char_5x5_set( 2, 12, "WRONG", &bright_orange_5x5_grad_tex);
        }
      }
    };


    ShortTimeSpan const SHORT_TRANSITION = ShortTimeSpan::from_millis(100);


    class MultiVis : public Vis {
    public:
      MultiVis() : _count(0) { }

      void attract_start() override {
        Debug::tracef("vis: attract_start");
        foreach_vis([=] (Vis * vis) { vis->attract_start(); });
      }
      void update(ShortTimeSpan dt) override {
        //Debug::tracef("vis: update dt=%ldus", (int32_t)dt.to_micros());
        foreach_vis([=] (Vis * vis) { vis->update(dt); });
      }
      void game_start() override {
        Debug::tracef("vis: game_start");
        foreach_vis([=] (Vis * vis) { vis->game_start(); });
      }
      void play_pattern() override {
        Debug::tracef("vis: play_pattern");
        foreach_vis([=] (Vis * vis) { vis->play_pattern(); });
      }
      void play_color(char id) override {
        Debug::tracef("vis: play_color id=%c", id);
        foreach_vis([=] (Vis * vis) { vis->play_color(id); });
      }
      void await_press() override {
        Debug::tracef("vis: await_press");
        foreach_vis([=] (Vis * vis) { vis->await_press(); });
      }
      void press_color(char id, bool correct) override {
        Debug::tracef("vis: press_color id=%c correct=%c", id,
          (correct ? 't' : 'f'));
        foreach_vis([=] (Vis * vis) { vis->press_color(id, correct); });
      }
      void score_change(int32_t score) override {
        Debug::tracef("vis: score_change score=%ld", score);
        foreach_vis([=] (Vis * vis) { vis->score_change(score); });
      }
      void game_over(int32_t final_score, uint16_t rating, bool win)
          override {
        Debug::tracef("vis: game_over");
        foreach_vis([=] (Vis * vis) {
          vis->game_over(final_score, rating, win); });
      }
      void enter_name(char const * name, size_t cursor_pos) override {
        Debug::tracef("vis: enter_name name=%s pos=%z", name, cursor_pos);
        foreach_vis([=] (Vis * vis) { vis->enter_name(name, cursor_pos); });
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

      void attract_start() override {
        Lights::start_animator(_layer_start + Lights::LAYER_STALK_BASE_COLOR,
          &_active_color, ShortTimeSpan::from_millis(500));
        Lights::transition_out(_layer_start + Lights::LAYER_STALK_BUBBLE,
          SHORT_TRANSITION);
        Lights::transition_out(_layer_start + Lights::LAYER_STALK_OVERLAY,
          ShortTimeSpan::from_millis(2000));
      }

      void play_pattern() override {
        Lights::start_animator(_layer_start + Lights::LAYER_STALK_BUBBLE,
            &_mute, SHORT_TRANSITION);
      }

      void play_color(char id) override {
        if(id == _id) {
          Lights::start_animator(_layer_start + Lights::LAYER_STALK_BUBBLE,
            _bubble_pool.acquire());
        }
        else {
          Lights::start_animator(_layer_start + Lights::LAYER_STALK_BUBBLE,
            &_mute, SHORT_TRANSITION);
        }
      }

      void await_press() override {
        Lights::transition_out(_layer_start + Lights::LAYER_STALK_BUBBLE,
          SHORT_TRANSITION);
      }

      void press_color(char id, bool correct) override {
        play_color(id);
      }

      void game_over(int32_t final_score, uint16_t rating, bool win)
          override {
        if(win) {
          Lights::start_animator(
            _layer_start + Lights::LAYER_STALK_OVERLAY, &_game_over_win,
            ShortTimeSpan::from_millis(2000));
        } else {
          Lights::start_animator(
            _layer_start + Lights::LAYER_STALK_OVERLAY, &_game_over_lose,
            ShortTimeSpan::from_millis(2000));
        }
      }

      void enter_name(char const * name, size_t cursor_pos) override {
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
          _instructions_long_scroll(instructions_long_text,
            sizeof instructions_long_text / sizeof *instructions_long_text,
            &orange_5x5_grad_tex),
          _instructions_short(instructions_short_text,
            sizeof instructions_short_text / sizeof *instructions_short_text,
            &bright_orange_5x5_grad_tex),
          _red_pulse(PulseFunction(Color::red)),
          _green_pulse(PulseFunction(Color::green)),
          _blue_pulse(PulseFunction(Color::blue)),
          _pink_pulse(PulseFunction(Color::pink)) {
        _splash.init(ShortTimeSpan::from_millis(10000));
        _splash_title.init(ShortTimeSpan::from_millis(3000));
        _instructions_long_scroll.init(ShortTimeSpan::from_millis(3000));
        _instructions_short.init();
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

      void attract_start() override {
        _state = AS_SPLASH;

        Lights::start_animator(
          Lights::LAYER_SB_START + Lights::LAYER_SB_BACKGROUND,
          &_neutral_bg, SHORT_TRANSITION);
        Lights::start_animator(Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
          &_splash, SHORT_TRANSITION);
        Lights::transition_out(
          Lights::LAYER_SB_START + Lights::LAYER_SB_OVERLAY,
          SHORT_TRANSITION);
      }

      void update(ShortTimeSpan dt) override {
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
              SHORT_TRANSITION);
          }
          break;
        case AS_SPLASH_TITLE:
          if(_splash_title.is_at_end()) {
            _state = AS_INSTRUCTIONS;
            Lights::start_animator(
              Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
              &_instructions_long_scroll,
              SHORT_TRANSITION);
            Lights::transition_out(
              Lights::LAYER_SB_START + Lights::LAYER_SB_OVERLAY,
              SHORT_TRANSITION);
          }
          break;
        case AS_INSTRUCTIONS:
          if(_instructions_long_scroll.is_at_end()) {
            _state = AS_SPLASH;
            Lights::start_animator(
              Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
              &_splash, SHORT_TRANSITION);
          }
          break;
        default:
          break;
        }
      }

      void game_start() override {
        _state = AS_NONE;

        _game_over_lose.set_new_hi_score(0);
        _score.set_score(0);
        Lights::start_animator(
          Lights::LAYER_SB_START + Lights::LAYER_SB_BACKGROUND, &_neutral_bg,
          SHORT_TRANSITION);
        Lights::start_animator(
          Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN, &_instructions_short,
          SHORT_TRANSITION);
        Lights::transition_out(
          Lights::LAYER_SB_START + Lights::LAYER_SB_OVERLAY, SHORT_TRANSITION);
      }

      void play_pattern() override {
        _pattern_count = 0;
        _score.set_score(_pattern_count);

        Lights::start_animator(
          Lights::LAYER_SB_START + Lights::LAYER_SB_BACKGROUND, &_neutral_bg,
          SHORT_TRANSITION);
        Lights::transition_out(
          Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
          SHORT_TRANSITION);
        Lights::start_animator(
          Lights::LAYER_SB_START + Lights::LAYER_SB_OVERLAY, &_score,
          SHORT_TRANSITION);
      }

      void play_color(char id) override {
        ++_pattern_count;
        _score.set_score(_pattern_count);

        switch(id) {
        case 'R':
          Lights::start_animator(
            Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
            &_red_pulse, SHORT_TRANSITION);
          break;
        case 'G':
          Lights::start_animator(
            Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
            &_green_pulse, SHORT_TRANSITION);
          break;
        case 'B':
          Lights::start_animator(
            Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
            &_blue_pulse, SHORT_TRANSITION);
          break;
        case 'P':
          Lights::start_animator(
            Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN,
            &_pink_pulse, SHORT_TRANSITION);
          break;
        }
      }

      void await_press() override {
        int32_t last_pattern_count = _pattern_count;
        play_pattern();
        _score.set_score(last_pattern_count);
      }

      void press_color(char id, bool correct) override {
        play_color(id);
      }

      void game_over(int32_t final_score, uint16_t rating, bool win)
          override {
        Lights::start_animator(
          Lights::LAYER_SB_START + Lights::LAYER_SB_BACKGROUND,
          &_neutral_bg, SHORT_TRANSITION);
        if(win) {
          Lights::start_animator(
            Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN, &_game_over_win,
            SHORT_TRANSITION);
        } else {
          Lights::start_animator(
            Lights::LAYER_SB_START + Lights::LAYER_SB_MAIN, &_game_over_lose,
            SHORT_TRANSITION);
        }
        Lights::transition_out(
          Lights::LAYER_SB_START + Lights::LAYER_SB_OVERLAY,
          SHORT_TRANSITION);
      }

      void enter_name(char const * name, size_t cursor_pos) override { }

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
        AS_INSTRUCTIONS,
      };

      AttractState _state;
      int32_t _pattern_count;

      ScoreboardSplashAnimator _splash;
      ScoreboardScrollingTextAnimator _splash_title;
      ScoreboardScrollingTextAnimator _instructions_long_scroll;
      ScoreboardStaticTextAnimator _instructions_short;
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
    MultiVis multi_vis;
  }

  void Vis::init() {
    init_textures();
    multi_vis.add_vis(&red_stalk_vis);
    multi_vis.add_vis(&green_stalk_vis);
    multi_vis.add_vis(&blue_stalk_vis);
    multi_vis.add_vis(&pink_stalk_vis);
    multi_vis.add_vis(&scoreboard_vis);
  }

  Vis & Vis::vis() {
    return multi_vis;
  }
}

