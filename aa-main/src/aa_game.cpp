#include "amanita_arcade.h"

#include <algorithm>
#include <iterator>

#include "aa_game.h"

#include "aa_input.h"
#include "aa_game_hi_score.h"
#include "aa_game_vis.h"
#include "aa_lights.h"
#include "aa_timer.h"


#define CTRL(c) (c - 'A' + 1)


namespace aa {
  namespace {
    class Xoshiro256ss {
    public:
      Xoshiro256ss()
          : _s{ 0x10203040, 0x50607080, 0x90A0B0C0, 0xD0E0F000 } {
      }

      void init(uint64_t const iv[4]) {
        memcpy(_s, iv, sizeof _s);
      }

      void init_rand() {
        for(size_t n = 0; n < std::size(_s); ++n) {
          _s[n] = (static_cast<uint64_t>(rand()) << 48)
            ^ (static_cast<uint64_t>(rand()) << 32)
            ^ (static_cast<uint64_t>(rand()) << 16)
            ^ (static_cast<uint64_t>(rand()) << 0);
        }
      }

      uint64_t next() {
        uint64_t const result = rol64(_s[1] * 5, 7) * 9;
        uint64_t const t = _s[1] << 17;

        _s[2] ^= _s[0];
        _s[3] ^= _s[1];
        _s[1] ^= _s[2];
        _s[0] ^= _s[3];

        _s[2] ^= t;
        _s[3] = rol64(_s[3], 45);

        return result;
      }

    private:
      uint64_t _s[4];

      static constexpr uint64_t rol64(uint64_t x, int k)
      {
        return (x << k) | (x >> (64 - k));
      }
    };


    size_t const MAX_PATTERN_LENGTH = HiScore::MAX_SCORE;


    Xoshiro256ss rng;


    char get_random_button() {
      switch(rng.next() % 4) {
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
    extern GameState * game_start_state;
    extern GameState * pause_before_play_state;
    extern GameState * play_pattern_state;
    extern GameState * await_response_state;
    extern GameState * pause_before_game_over_state;
    extern GameState * game_over_state;


    char pattern[MAX_PATTERN_LENGTH];
    size_t pattern_length;
    size_t score;
    aa::Timer state_timer(TimeSpan::infinity, false);
    GameState * current_state;


    class InitialGameState : public GameState {
    public:
      InitialGameState() : GameState("initial", TimeSpan::infinity) { }
      virtual void on_enter() {
        pattern_length = 0;
        score = 0;
        Vis::vis().score_change(0);
        Vis::vis().attract_start();
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
        Vis::vis().attract_start();
      }
      virtual bool on_button(char id) {
        request_transition(game_start_state);
        return true;
      }
    } attract_state_instance;


    class GameStartGameState : public GameState {
    public:
      GameStartGameState()
        : GameState("game start", ShortTimeSpan::from_millis(5000)) { }
      virtual void on_enter() {
        pattern[0] = get_random_button();
        pattern_length = 1;
        Vis::vis().game_start();
      }
      virtual bool on_button(char id) {
        on_timeout();
        return true;
      }
      virtual void on_timeout() {
        request_transition(play_pattern_state);
      }
    } game_start_state_instance;


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
        Vis::vis().play_pattern();
        Debug::auto_assert(pattern_length > 0);
        Vis::vis().play_color(pattern[0]);
        _pattern_pos = 1;
      }
      virtual void on_timeout() {
        if(_pattern_pos < pattern_length) {
          Vis::vis().play_color(pattern[_pattern_pos]);
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
        : GameState("await response",
          ShortTimeSpan::from_millis(20000)) { }
      virtual void on_enter() {
        Vis::vis().await_press();
        Debug::auto_assert(pattern_length > 0);
        _pattern_pos = 0;
      }
      virtual bool on_button(char id) {
        Debug::auto_assert(_pattern_pos < pattern_length);
        if(id == pattern[_pattern_pos]) {
          Vis::vis().press_color(id, true);
          ++_pattern_pos;
          if(_pattern_pos >= pattern_length) {
            score = pattern_length;
            Vis::vis().score_change(score);
            if(pattern_length >= MAX_PATTERN_LENGTH) {
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
          Vis::vis().press_color(id, false);
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
        uint16_t rating = HiScore::rate_score(score);
        if(rating & HiScore::RATING_ON_ANY_LIST) {
          HiScore::add_score(score, HiScore::DEFAULT_NAME);
        }
        Vis::vis().game_over(score, rating, (score >= MAX_PATTERN_LENGTH));
      }
      virtual void on_timeout() { request_transition(attract_state); }
    } game_over_state_instance;


    GameState * initial_state = &initial_state_instance;
    GameState * attract_state = &attract_state_instance;
    GameState * game_start_state = &game_start_state_instance;
    GameState * pause_before_play_state =
      &pause_before_play_state_instance;
    GameState * play_pattern_state = &play_pattern_state_instance;
    GameState * await_response_state = &await_response_state_instance;
    GameState * pause_before_game_over_state =
      &pause_before_game_over_state_instance;
    GameState * game_over_state = &game_over_state_instance;


    static void debug_update(TimeSpan dt);
    static void debug_help();
    static void debug_setup();
    static void debug_list_scores();
    static void debug_add_score();
    static void debug_clear_score();
    static void debug_dump_nv();
    static void debug_fill_nv();
    static void debug_values();

    static bool debug_read_bool(char const * prompt, bool * val);
    static bool debug_read_int(char const * prompt, int64_t * val,
      int64_t min, int64_t max);
    static bool debug_read_str(char const * prompt, char * val, size_t len);

    template<typename T>
    static bool debug_read_int(char const * prompt, T * val,
        T min, T max) {
      int64_t val64 = *val;
      if(debug_read_int(prompt, &val64, min, max)) {
        *val = (T)val64;
        return true;
      }
      else {
        return false;
      }
    }


    void debug_update(TimeSpan dt) {
      uint32_t debug_buttons = 0;
      while(hw::debug_ser.readable()) {
        char c;
        hw::debug_ser.read(&c, 1);
        switch(c) {
          case 'r': case 'R': debug_buttons |= Input::B_RED; break;
          case 'g': case 'G': debug_buttons |= Input::B_GREEN; break;
          case 'b': case 'B': debug_buttons |= Input::B_BLUE; break;
          case 'p': case 'P': debug_buttons |= Input::B_PINK; break;

          default:
          case 'h': case 'H': debug_help(); break;
          case 's': case 'S': debug_setup(); break;
          case 'l': case 'L': debug_list_scores(); break;
          case 'a': case 'A': debug_add_score(); break;
          case 'c': case 'C': debug_clear_score(); break;
          case 'd': case 'D': debug_dump_nv(); break;
          case 'f': case 'F': debug_fill_nv(); break;
          case 'v': case 'V': debug_values(); break;
          case CTRL('C'): fputs("^cancel\r\n", stdout); break;
          case CTRL('R'): Debug::abort(); break;
        }
      }
      Input::debug_sim_buttons(debug_buttons);
    }


    void debug_help() {
      fputs(
        "Debug help:\r\n"
        "rgbp - simulate red, green, blue, pink button\r\n"
        "h    - help (print this message)\r\n"
        "s    - setup (change day, event)\r\n"
        "l    - list hi scores\r\n"
        "a    - add a hi score\r\n"
        "c    - clear a hi score or scores\r\n"
        "d    - dump EEPROM to console (backup)\r\n"
        "f    - fill EEPROM from console (restore)\r\n"
        "v    - values (print internal state)\r\n"
        "^R   - reset\r\n",
        stdout);
    }


    void debug_setup() {
      fputs("Setup:\r\n", stdout);
      HiScoreSettings settings = *HiScore::get_settings();
      debug_read_int<uint8_t>("day", &settings.today, 0, UINT8_MAX);
      debug_read_str("event", settings.event, sizeof settings.event);
      HiScore::set_settings(settings);
    }


    void debug_list_scores() {
      fputs("List scores:\r\n", stdout);
      size_t i = 1;
      for(HiScoreEntry const * const * entry =
            HiScore::get_scores(HiScore::LIST_COMBINED);
          *entry != nullptr; ++entry) {
        fprintf(stdout, "%2d. %*.*s: %2d - %*.*s %d\r\n", (int)i++,
          HI_SCORE_NAME_MAX, HI_SCORE_NAME_MAX, (*entry)->name,
          (int)(*entry)->score, HI_SCORE_EVENT_MAX, HI_SCORE_EVENT_MAX,
          (*entry)->event, (int)(*entry)->day);
      }
    }


    void debug_add_score() {
      fputs("Add score:\r\n", stdout);
      HiScoreSettings settings = *HiScore::get_settings();
      HiScoreEntry entry;
      entry.score = 1;
      entry.day = settings.today;
      strncpy(entry.event, settings.event, sizeof entry.event);
      strncpy(entry.name, HiScore::DEFAULT_NAME, sizeof entry.name);
      if(debug_read_int<uint8_t>("score", &entry.score, 0, HiScore::MAX_SCORE)
          && debug_read_int<uint8_t>("day", &entry.day, 0, UINT8_MAX)
          && debug_read_str("name", entry.name, sizeof entry.name)
          && debug_read_str("event", entry.event, sizeof entry.event)) {
        HiScore::add_score(entry);
      }
    }


    void debug_clear_score() {
      fputs("Clear score:\r\n", stdout);
      size_t num_entries = 0;
      HiScoreEntry const * const * list =
        HiScore::get_scores(HiScore::LIST_COMBINED);
      for(HiScoreEntry const * const * entry = list;
          *entry != nullptr; ++entry) {
        fprintf(stdout, "%2d. %*.*s: %2d - %*.*s %d\r\n",
          (int)num_entries + 1, HI_SCORE_NAME_MAX, HI_SCORE_NAME_MAX,
          (*entry)->name, (int)(*entry)->score, HI_SCORE_EVENT_MAX,
          HI_SCORE_EVENT_MAX, (*entry)->event, (int)(*entry)->day);
        ++num_entries;
      }
      uint8_t score_num = 0;
      bool confirm = false;
      if(debug_read_int<uint8_t>("entry number (0 for all)", &score_num, 0,
          num_entries)
          && debug_read_bool("really clear this entry", &confirm)
          && confirm) {
        if(score_num > 0) {
          HiScore::clear_score(list[score_num - 1]);
        }
        else {
          HiScore::clear_all_scores();
        }
      }
    }


    void debug_dump_nv() {
      static size_t const PAGE_SIZE = 16;
      fputs("Dump EEPROM:\r\n", stdout);
      size_t dump_start = 0;
      size_t dump_size = NV_SIZE;
      if(!debug_read_int<size_t>("start address", &dump_start, 0, NV_SIZE)) {
        return;
      }
      if(!debug_read_int<size_t>("size", &dump_size, 0,
          NV_SIZE - dump_start)) {
        return;
      }
      size_t dump_end = dump_start + dump_size;
      for(size_t i = (dump_start & ~0xF); i < dump_end; i += PAGE_SIZE) {
        System::service_watchdog();
        uint8_t data[PAGE_SIZE];
        if(!System::read_nv(i, data, PAGE_SIZE)) {
          return;
        }
        fprintf(stdout, "%04X: ", i);
        size_t j = 0;
        if((i + j) < dump_start) {
          fprintf(stdout, "%*c", (dump_start - (i + j)) * 3, ' ');
          j = dump_start - i;
        }
        for(; (j < PAGE_SIZE) && ((i + j) < dump_end); ++j) {
          fprintf(stdout, "%02X ", data[j]);
        }
        if(j < PAGE_SIZE) {
          fprintf(stdout, "%*c", (PAGE_SIZE - j) * 3, ' ');
        }
        fputc('|', stdout);
        j = 0;
        if((i + j) < dump_start) {
          fprintf(stdout, "%*c", (dump_start - (i + j)), ' ');
          j = dump_start - i;
        }
        for(; (j < PAGE_SIZE) && ((i + j) < dump_end); ++j) {
          char c = (char)data[j];
          fputc((c >= 32) && (c < 127) ? c : '.', stdout);
        }
        if(j < PAGE_SIZE) {
          fprintf(stdout, "%*c", (PAGE_SIZE - j), ' ');
        }
        fputs("|\r\n", stdout);
        if(hw::debug_ser.readable()) {
          char c;
          hw::debug_ser.read(&c, 1);
          if(c == CTRL('C')) {
            fputs("^cancel\r\n", stdout);
            return;
          }
        }
      }
    }


    void debug_fill_nv() {
      fputs("Fill EEPROM:\r\n", stdout);
    }


    void debug_values() {
      fputs("Values:\r\n", stdout);
    }


    bool debug_read_bool(char const * prompt, bool * val) {
      for(;;) {
        System::service_watchdog();
        fprintf(stdout, "%s (y/n) [%c]: ", prompt, *val ? 'y' : 'n');
        while(!hw::debug_ser.readable()) {
          // busy-wait
          wait_us(10000);
        }
        char c;
        hw::debug_ser.read(&c, 1);
        switch(c) {
        case 'y': case 'Y':
          fputs("y\r\n", stdout);
          *val = true;
          return true;
        case 'n': case 'N':
          fputs("n\r\n", stdout);
          *val = false;
          return true;
        case '\r': case '\n':
          // Leave default
          fputs(*val ? "y\r\n" : "n\r\n", stdout);
          return true;
        case CTRL('C'): case '\x1B':
          fputs("^cancel\r\n", stdout);
          return false;
        default:
          fputs("Please answer either 'y' or 'n', or use ctrl+C "
            "or esc to cancel\r\n", stdout);
          break;
        }
      }
    }


    bool debug_read_int(char const * prompt, int64_t * val,
        int64_t min, int64_t max) {
      char str_val[20];
      Debug::auto_assert(min < max);
      for(;;) {
        System::service_watchdog();
        if(snprintf(str_val, sizeof str_val, "%d", (int)*val)
            >= (int)sizeof str_val) {
          Debug::auto_error("converted default is longer than max string?");
          return false;
        }
        if(!debug_read_str(prompt, str_val, sizeof str_val)) {
          return false;
        }
        char * endp;
        long long llval;
        llval = strtoll(str_val, &endp, 10);
        if((endp == nullptr) || (endp >= str_val + sizeof str_val)
            || (endp == str_val) || (*endp != 0)
            || (llval < min) || (llval > max)) {
          fprintf(stdout,
            "Please enter a number between %lld and %lld, or use ctrl+C "
            "or esc to cancel\r\n",
            (long long)min, (long long)max);
        }
        else {
          *val = llval;
          return true;
        }
      }
    }


    bool debug_read_str(char const * prompt, char * val, size_t len) {
      fprintf(stdout, "%s [%.*s]: ", prompt, len, val);
      fflush(stdout);
      size_t pos = 0;
      bool has_input = false;
      for(;;) {
        while(!hw::debug_ser.readable()) {
          System::service_watchdog();
          wait_us(10000);
        }
        System::service_watchdog();
        char c;
        hw::debug_ser.read(&c, 1);
        switch(c) {
        case CTRL('C'): case '\x1B':
          fputs("^cancel\r\n", stdout);
          return false;
        case '\b': case '\x7F':
          if(pos > 0) {
            fputs("\b \b", stdout);
            --pos;
          }
          else {
            // Maybe beeps when you backspace past the beginning are obnoxious
            // fputc('\a', stdout);
          }
          has_input = true;
          break;
        case '\r':
          break;
        case '\n':
          Debug::auto_assert(has_input || (pos == 0));
          if(has_input) {
            while(pos < len) {
              val[pos++] = 0;
            }
          }
          fputs("\r\n", stdout);
          return true;
        default:
          if((pos < len) && (c >= 32) && (c < 127)) {
            fputc(c, stdout);
            val[pos++] = c;
          } 
          else {
            fputc('\a', stdout);
          }
          has_input = true;
          break;
        }
      }
    }
  }


  void Game::init() {
    HiScore::init();
    Vis::vis().init();
    rng.init_rand();

    current_state = initial_state;
    current_state->reset();
    current_state->on_enter();
  }


  void Game::update(ShortTimeSpan dt) {
    debug_update(dt);

    current_state->get_timer()->update(dt);

    //Debug::tracef("Update: %ldus -> %lldus/%lldus, %d", dt.to_micros(),
    //  current_state->get_timer()->get_time().to_micros(),
    //  current_state->get_timer()->get_period().to_micros(),
    //  current_state->get_timer()->peek_periods());

    uint32_t buttons = Input::all_pressed();
    int32_t periods = current_state->get_timer()->read_periods();
    while(true) {
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
        break;
      }
    }

    Vis::vis().update(dt);
  }
}
