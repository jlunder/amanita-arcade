#include "amanita_arcade.h"

#include <algorithm>
#include <iterator>

#include "aa_game_hi_score.h"


namespace aa {
  namespace {
    static_assert(sizeof (HiScoreEntry) == 16);

    typedef HiScoreEntry NvHiScoreEntry;

    struct __packed NvHiScoreMetadata {
      HiScoreSettings settings;
      uint8_t pad0[2+8];
    };

    static_assert(sizeof (NvHiScoreEntry) == 16);
    static_assert(sizeof (NvHiScoreMetadata) == 16);


    static size_t const NUM_NV_HI_SCORE_ENTRIES = 16;
    static_assert((NUM_NV_HI_SCORE_ENTRIES * sizeof (NvHiScoreEntry)
      + sizeof (NvHiScoreMetadata)) <= NV_HI_SCORES_SIZE);
    static NvHiScoreEntry nv_hi_scores[NUM_NV_HI_SCORE_ENTRIES];
    static NvHiScoreMetadata nv_hi_score_metadata;
    static size_t hi_score_order[NUM_NV_HI_SCORE_ENTRIES];
    static HiScoreEntry const * combined_list[NUM_NV_HI_SCORE_ENTRIES + 1];
    static HiScoreEntry const * todays_list[HiScore::ENTRIES_PER_LIST + 1];
    static HiScoreEntry const * event_list[HiScore::ENTRIES_PER_LIST + 1];
    static HiScoreEntry const * all_time_list[HiScore::ENTRIES_PER_LIST + 1];
    static uint32_t seq;

    static bool nv_available = false;


    static bool nv_hi_score_entry_valid(NvHiScoreEntry const * entry);
    static uint8_t nv_hi_score_entry_new_check(NvHiScoreEntry const * entry);

    static void init_nv();
    static void nv_hi_scores_sort();
    static void nv_hi_scores_rebuild_lists();
    static void nv_hi_scores_clear_score(size_t i);
    static void nv_hi_scores_clear_all();
    static void nv_hi_scores_store_all();
    static size_t nv_hi_scores_store_new(HiScoreEntry const & entry);
    static void nv_hi_scores_store_metadata();
    static void nv_clear(void * p, size_t len);
    template<typename T>
    static inline void nv_clear(T * p) {
      nv_clear(p, sizeof (T));
    }


    bool nv_hi_score_entry_valid(NvHiScoreEntry const * entry) {
      if((entry->check0 == 0) || (entry->check0 == 0xFF)
          || (entry->check0 != entry->check1)) {
        return false;
      }
      if(entry->score > HiScore::MAX_SCORE) {
        return false;
      }
      if((entry->name[0] < 32) || (entry->name[0] >= 128)) {
        return false;
      }
      if((entry->event[0] < 32) || (entry->event[0] >= 128)) {
        return false;
      }
      return true;
    }


    uint8_t nv_hi_score_entry_new_check(
        NvHiScoreEntry const * entry) {
      uint8_t check = (entry->check0 | 1) * 241;
      if((check == entry->check1) || (check == 0xFF)) {
        check *= 241;
      }
      if((check == entry->check1) || (check == 0xFF)) {
        check *= 241;
      }
      Debug::dev_auto_check((check != entry->check0)
        && (check != entry->check1) && (check != 0) && (check != 0xFF));

      return check;
    }


    void init_nv() {
      nv_available = false;
      if(System::read_nv(aa::NV_HI_SCORES_ADDR,
          &nv_hi_scores, sizeof nv_hi_scores)) {
        for(size_t i = 0; i < NUM_NV_HI_SCORE_ENTRIES; ++i) {
          NvHiScoreEntry * entry = &nv_hi_scores[i];
          if(!nv_hi_score_entry_valid(entry)) {
            // Garbage entry? Clear it all the way...
            nv_clear(entry, sizeof *entry);
          }
        }
        nv_hi_scores_sort();
        if(System::read_nv(aa::NV_HI_SCORES_ADDR + aa::NV_HI_SCORES_SIZE
              - sizeof (NvHiScoreMetadata), &nv_hi_score_metadata,
            sizeof (NvHiScoreMetadata))) {
          nv_available = true;
        }
      } else {
        nv_clear(&nv_hi_scores, sizeof nv_hi_scores);
        nv_clear(&nv_hi_score_metadata, sizeof nv_hi_score_metadata);
        for(size_t i = 0; i < NUM_NV_HI_SCORE_ENTRIES; ++i) {
          hi_score_order[i] = i;
        }
      }
      if(nv_available) {
        Debug::trace("Successfully read hi scores from EEPROM");
      } else {
        Debug::trace("Unable to read hi scores from EEPROM, won't write");
      }
      nv_hi_scores_sort();
      for(size_t i = 0; i < sizeof nv_hi_score_metadata.settings.event; ++i) {
        if((nv_hi_score_metadata.settings.event[i] < 32)
            || (nv_hi_score_metadata.settings.event[0] >= 127)) {
          strncpy(nv_hi_score_metadata.settings.event, HiScore::DEFAULT_EVENT,
            sizeof nv_hi_score_metadata.settings.event);
          break;
        }
      }
    }


    void nv_hi_scores_sort() {
      for(size_t i = 0; i < NUM_NV_HI_SCORE_ENTRIES; ++i) {
        hi_score_order[i] = i;
      }
      // Sort descending -- this is written as "less", but with a/b reversed
      // in the params list!
      std::sort(std::begin(hi_score_order), std::end(hi_score_order),
        [](size_t b, size_t a)
            -> bool {
          Debug::auto_assert(a < NUM_NV_HI_SCORE_ENTRIES);
          Debug::auto_assert(b < NUM_NV_HI_SCORE_ENTRIES);
          bool b_valid = nv_hi_score_entry_valid(&nv_hi_scores[b]);
          if(!b_valid) return false;
          bool a_valid = nv_hi_score_entry_valid(&nv_hi_scores[a]);
          if(!a_valid) return true;
          HiScoreEntry const * a_entry = &nv_hi_scores[a];
          HiScoreEntry const * b_entry = &nv_hi_scores[b];
          if(a_entry->score < b_entry->score) return true;
          if(a_entry->score > b_entry->score) return false;
          int event_cmp = strncmp(a_entry->event, b_entry->event,
            sizeof HiScoreEntry::event);
          if(event_cmp < 0) return true;
          if(event_cmp > 0) return false;
          if(a_entry->day < b_entry->day) return true;
          if(a_entry->day > b_entry->day) return false;
          // Rank the first person to achieve a score highest when there's a
          // score tie; i.e., lowest seq > highest seq, so we need to reverse
          // the test relative to the score test
          if(nv_hi_scores[a].seq > nv_hi_scores[b].seq) return true;
          if(nv_hi_scores[a].seq < nv_hi_scores[b].seq) return false;
          // In a properly functioning system seq should disambiguate
          // entries positively, but maybe somehow we end up with duplicates
          // due to garbage in the list, so let's still try for a strict
          // ordering and not reorder scores randomly in case the sort is
          // unstable
          Debug::dev_tracef("Duplicate seq %" PRId32 " for entries %z and %z",
            nv_hi_scores[a].seq, a, b);
          // We don't normally consider the name because the user enters it
          // and we don't want them to be able to influence their relative
          // ordering when there's a tie
          int name_cmp = strncmp(a_entry->name, b_entry->name,
            sizeof HiScoreEntry::name);
          if(name_cmp < 0) return true;
          if(name_cmp > 0) return false;

          return false;
        });
      nv_hi_scores_rebuild_lists();
    }


    void nv_hi_scores_rebuild_lists() {
      size_t j = 0;
      for(size_t i = 0;
          (i < NUM_NV_HI_SCORE_ENTRIES) && (j < NUM_NV_HI_SCORE_ENTRIES);
          ++i) {
        NvHiScoreEntry const * entry = &nv_hi_scores[hi_score_order[i]];
        if(nv_hi_score_entry_valid(entry)) {
          combined_list[j++] = entry;
        }
      }
      for(; j < NUM_NV_HI_SCORE_ENTRIES; ++j) {
        combined_list[j++] = nullptr;
      }
      // All-time best
      j = 0;
      for(size_t i = 0;
          (i < NUM_NV_HI_SCORE_ENTRIES) && (j < HiScore::ENTRIES_PER_LIST);
          ++i) {
        NvHiScoreEntry const * entry = &nv_hi_scores[hi_score_order[i]];
        if(nv_hi_score_entry_valid(entry)) {
          all_time_list[j++] = entry;
        }
      }
      for(; j < HiScore::ENTRIES_PER_LIST; ++j) {
        all_time_list[j++] = nullptr;
      }
      // This event best
      j = 0;
      for(size_t i = 0;
          (i < NUM_NV_HI_SCORE_ENTRIES) && (j < HiScore::ENTRIES_PER_LIST);
          ++i) {
        NvHiScoreEntry const * entry = &nv_hi_scores[hi_score_order[i]];
        if(nv_hi_score_entry_valid(entry)
            && (strncmp(entry->event,
              nv_hi_score_metadata.settings.event,
              HI_SCORE_EVENT_MAX) == 0)) {
          event_list[j++] = entry;
        }
      }
      for(; j < HiScore::ENTRIES_PER_LIST; ++j) {
        event_list[j++] = nullptr;
      }
      // Today's best
      j = 0;
      for(size_t i = 0;
          (i < NUM_NV_HI_SCORE_ENTRIES) && (j < HiScore::ENTRIES_PER_LIST);
          ++i) {
        NvHiScoreEntry const * entry = &nv_hi_scores[hi_score_order[i]];
        if(nv_hi_score_entry_valid(entry)
            && (strncmp(entry->event,
              nv_hi_score_metadata.settings.event,
              HI_SCORE_EVENT_MAX) == 0)
            && (entry->day == nv_hi_score_metadata.settings.today)) {
          todays_list[j++] = entry;
        }
      }
      for(; j < HiScore::ENTRIES_PER_LIST; ++j) {
        todays_list[j++] = nullptr;
      }
    }


    void nv_hi_scores_clear_score(size_t i) {
      Debug::auto_assert(i < NUM_NV_HI_SCORE_ENTRIES);
      nv_clear(&nv_hi_scores[i], sizeof (NvHiScoreEntry));
      if(nv_available) {
        System::write_nv(NV_HI_SCORES_ADDR + i * sizeof (NvHiScoreEntry),
          &nv_hi_scores[i], sizeof (NvHiScoreEntry));
      }
      nv_hi_scores_sort();
    }


    void nv_hi_scores_clear_all() {
      nv_clear(&nv_hi_scores, sizeof nv_hi_scores);
      for(size_t i = 0; i < NUM_NV_HI_SCORE_ENTRIES; ++i) {
        hi_score_order[i] = i;
      }
      // will sort implicitly
      nv_hi_scores_store_all();
    }


    void nv_hi_scores_store_all() {
      if(nv_available) {
        System::write_nv(NV_HI_SCORES_ADDR, &nv_hi_scores,
          sizeof nv_hi_scores);
      }
      // will sort implicitly
      nv_hi_scores_store_metadata();
    }


    size_t nv_hi_scores_store_new(HiScoreEntry const & entry) {
      size_t i = hi_score_order[NUM_NV_HI_SCORE_ENTRIES - 1];
      Debug::auto_assert(i < NUM_NV_HI_SCORE_ENTRIES);
      NvHiScoreEntry * least_score = &nv_hi_scores[i];
      if(nv_hi_score_entry_valid(least_score)
          && (least_score->score > entry.score)) {
        // The worst score in the list is still better than this one, sorry
        return SIZE_MAX;
      }

      uint8_t check = nv_hi_score_entry_new_check(least_score);
      *least_score = entry;
      least_score->check0 = check;
      least_score->seq = seq++;
      least_score->check1 = check;
      if(nv_available) {
        System::write_nv(NV_HI_SCORES_ADDR + i * sizeof (NvHiScoreEntry),
          least_score, sizeof (NvHiScoreEntry));
      }
      nv_hi_scores_sort();
      return i;
    }


    void nv_hi_scores_store_metadata() {
      if(nv_available) {
        System::write_nv(aa::NV_HI_SCORES_ADDR + aa::NV_HI_SCORES_SIZE
            - sizeof (NvHiScoreMetadata), &nv_hi_score_metadata,
          sizeof (NvHiScoreMetadata));
      }
      nv_hi_scores_sort();
    }


    void nv_clear(void * p, size_t len) {
      memset(p, 0xFF, len);
    }
  }


  void HiScore::init() {
    init_nv();
    seq = 0;
    if(nv_available) {
      for(size_t i = NUM_NV_HI_SCORE_ENTRIES; i-- > 0; ) {
        Debug::auto_assert(hi_score_order[i] < NUM_NV_HI_SCORE_ENTRIES);
        NvHiScoreEntry const * entry = &nv_hi_scores[hi_score_order[i]];
        if(nv_hi_score_entry_valid(entry)) {
          if(entry->seq > seq) seq = entry->seq;
        }
      }
      // Is seq implausibly large? Probably this is due to garbage, let's
      // re-sequence all the data so we don't wrap around
      if(seq > (INT16_MAX / 2)) {
        seq = 0;
        for(size_t i = NUM_NV_HI_SCORE_ENTRIES; i-- > 0; ) {
          Debug::auto_assert(hi_score_order[i] < NUM_NV_HI_SCORE_ENTRIES);
          NvHiScoreEntry * entry = &nv_hi_scores[hi_score_order[i]];
          if(nv_hi_score_entry_valid(entry)) {
            entry->seq = seq++;
          }
        }
        nv_hi_scores_store_all();
      }
    }
  }


  void HiScore::update(ShortTimeSpan dt) {
  }


  uint16_t HiScore::rate_score(uint16_t score) {
    uint16_t rating = 0;
    HiScoreEntry const * todays_worst = todays_list[ENTRIES_PER_LIST - 1];
    HiScoreEntry const * todays_best = todays_list[0];
    HiScoreEntry const * event_worst = event_list[ENTRIES_PER_LIST - 1];
    HiScoreEntry const * event_best = event_list[0];
    HiScoreEntry const * all_time_worst = all_time_list[ENTRIES_PER_LIST - 1];
    HiScoreEntry const * all_time_best = all_time_list[0];

    // Ties are broken to prefer the most recent score ONLY for today's worst,
    // which is why <= here but < elsewhere
    if ((todays_worst == nullptr) || (score >= todays_worst->score)) {
      rating |= RATING_ON_ANY_LIST | RATING_ON_TODAYS_LIST;
    }
    if ((todays_best == nullptr) || (score > todays_best->score)) {
      rating |= RATING_TODAYS_BEST;
    }
    if ((event_worst == nullptr) || (score > event_worst->score)) {
      rating |= RATING_ON_ANY_LIST | RATING_ON_EVENT_LIST;
    }
    if ((event_best == nullptr) || (score > event_best->score)) {
      rating |= RATING_EVENT_BEST;
    }
    if ((all_time_worst == nullptr) || (score > all_time_worst->score)) {
      rating |= RATING_ON_ANY_LIST | RATING_ON_ALL_TIME_LIST;
    }
    if ((all_time_best == nullptr) || (score > all_time_best->score)) {
      rating |= RATING_ALL_TIME_BEST;
    }

    return rating;
  }


  void HiScore::add_score(uint16_t score, char const * name) {
    HiScoreEntry entry;
    entry.score = score;
    entry.day = nv_hi_score_metadata.settings.today;
    strncpy(entry.name, name, sizeof entry.name);
    strncpy(entry.event, nv_hi_score_metadata.settings.event,
      sizeof entry.event);
    nv_hi_scores_store_new(entry);
  }


  void HiScore::add_score(HiScoreEntry const & entry) {
    nv_hi_scores_store_new(entry);
  }


  HiScoreEntry const * const * HiScore::get_scores(size_t list_id) {
    switch(list_id) {
    case LIST_COMBINED: return combined_list;
    case LIST_TODAYS: return todays_list;
    case LIST_EVENT: return event_list;
    case LIST_ALL_TIME: return all_time_list;
    default: Debug::auto_error("invalid list ID");
    }
    return nullptr;
  }


  bool HiScore::get_nv_available() {
    return nv_available;
  }


  void HiScore::clear_score(HiScoreEntry const * entry) {
    size_t i;
    for(i = 0; i < NUM_NV_HI_SCORE_ENTRIES; ++i) {
      if(&nv_hi_scores[i] == entry) {
        break;
      }
    }
    if(i >= NUM_NV_HI_SCORE_ENTRIES) {
      Debug::auto_error("hi score entry not found");
      return;
    }
    nv_hi_scores_clear_score(i);
  }


  void HiScore::clear_all_scores() {
    nv_hi_scores_clear_all();
  }


  void HiScore::clear_settings() {
    nv_clear(&nv_hi_score_metadata);
    nv_hi_scores_store_metadata();
  }


  HiScoreSettings const * HiScore::get_settings() {
    return &nv_hi_score_metadata.settings;
  }


  void HiScore::set_settings(HiScoreSettings const & settings) {
    nv_hi_score_metadata.settings = settings;
    nv_hi_scores_store_metadata();
  }
}
