#ifndef AA_GAME_HI_SCORE_H
#define AA_GAME_HI_SCORE_H


#include "amanita_arcade.h"

#include "aa_time_span.h"


namespace aa {
  static size_t const HI_SCORE_NAME_MAX = 5;
  static size_t const HI_SCORE_EVENT_MAX = 5;

  struct __packed HiScoreEntry {
    uint8_t check0;
    uint8_t score;
    uint16_t seq;
    uint8_t day;
    char name[HI_SCORE_NAME_MAX];
    char event[HI_SCORE_EVENT_MAX];
    uint8_t check1;
  };

  struct __packed HiScoreSettings {
    char event[HI_SCORE_EVENT_MAX];
    uint8_t today;
  };

  class HiScore {
  public:
    static uint8_t const MAX_SCORE = 99;

    static uint16_t const RATING_ON_ANY_LIST = 0x0001;
    static uint16_t const RATING_ON_TODAYS_LIST = 0x0010;
    static uint16_t const RATING_ON_EVENT_LIST = 0x0020;
    static uint16_t const RATING_ON_ALL_TIME_LIST = 0x0040;
    static uint16_t const RATING_TODAYS_BEST = 0x0100;
    static uint16_t const RATING_EVENT_BEST = 0x0200;
    static uint16_t const RATING_ALL_TIME_BEST = 0x0400;

    static size_t const LIST_COMBINED = 0;
    static size_t const LIST_TODAYS = 1;
    static size_t const LIST_EVENT = 2;
    static size_t const LIST_ALL_TIME = 3;
    static size_t const ENTRIES_PER_LIST = 5;

    static constexpr char const * DEFAULT_EVENT = "-";
    static constexpr char const * DEFAULT_NAME = "AA";

    static void init();
    static void update(ShortTimeSpan dt);
    static uint16_t rate_score(uint16_t score);
    static void add_score(uint16_t score, char const * name);
    static HiScoreEntry const * const * get_scores(size_t list_id);

    static bool get_nv_available();
    static void add_score(HiScoreEntry const & entry);
    static void clear_score(HiScoreEntry const * entry);
    static void clear_all_scores();
    static void clear_settings();
    static HiScoreSettings const * get_settings();
    static void set_settings(HiScoreSettings const & settings);
  };
}


#endif // AA_GAME_H
