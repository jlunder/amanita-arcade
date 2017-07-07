#include "amanita_arcade.h"

#include "aa_lights.h"

#include "aa_input.h"


namespace aa {
  namespace {
    //uint8_t const font[64][5];
    char const * const splash_image =
      "      WWRRRRRRRRRWW           "
      "   RWWWWWRRRRRRRWWWWRR        "
      " RRRWWWWWRRRRRRRWWWWWRRR      "
      "RRRRRWWWRRRRRRRRWWWWWRRRR     "
      "WWWRRRRRRRRRRRRRRWWWRRRRRR    "
      "WWWRRRRRRWWWWRRRRRRRRRRRRR    "
      "RRRRRRRRWWWWWWRRRRRRRWWWWR    "
      " RRRRRRRRWWWWRRRRRRRWWWWW     "
      "  RWWWRRRRRRRRRRRRRRWWW       "
      "    WWWRRRRRRRRRRRRRR         "
      "        .ww.www.ww.           "
      "        .ww.www.ww.           "
      "       .www.www.ww.           "
      "       .www.wwww.ww.          "
      "         XwXw    wX           "
      "         XwXw    wX           "
      "         XwXw r  wX           "
      "         XwXw r  wX           "
      "         XwwXwrwwwwX          "
      "         XwwXwrwrwwX          "
      "         XwwwXwwwwXwX         "
      "         XwwwXwwXXwwX         "
      "         XwwwXXXwwwwX         "
      "         XwwwXwwwwwwX         "
      "         XwwwXwwwwwwX         "
      "          XwwXwwwwwwX         "
      "           XwXwwwwwXX         "
      "            XXwwwwXX          "
      "             XwwXX            "
      "              XX              ";
  }

  Lights::Layer Lights::_layers[LAYER_COUNT] = {
    { .width = STALK_HEIGHT_RED, .height = STALK_WIDTH },
    { .width = STALK_HEIGHT_RED, .height = STALK_WIDTH },
    { .width = STALK_HEIGHT_RED, .height = STALK_WIDTH },
    { .width = STALK_HEIGHT_GREEN, .height = STALK_WIDTH },
    { .width = STALK_HEIGHT_GREEN, .height = STALK_WIDTH },
    { .width = STALK_HEIGHT_GREEN, .height = STALK_WIDTH },
    { .width = STALK_HEIGHT_BLUE, .height = STALK_WIDTH },
    { .width = STALK_HEIGHT_BLUE, .height = STALK_WIDTH },
    { .width = STALK_HEIGHT_BLUE, .height = STALK_WIDTH },
    { .width = STALK_HEIGHT_PINK, .height = STALK_WIDTH },
    { .width = STALK_HEIGHT_PINK, .height = STALK_WIDTH },
    { .width = STALK_HEIGHT_PINK, .height = STALK_WIDTH },
    { .width = SCOREBOARD_WIDTH, .height = SCOREBOARD_HEIGHT },
    { .width = SCOREBOARD_WIDTH, .height = SCOREBOARD_HEIGHT },
  };
  uint32_t Lights::_output_buf[PAGE_COUNT][PAGE_SIZE];
  Color Lights::_texture_temp[2][TEXTURE_TEMP_MAX];


  Lights::AnimatorPool::AnimatorPool() : _next() {
    for(size_t i = 0; i < POOL_COUNT; ++i) {
      _animators[i] = nullptr;
    }
  }


  void Lights::AnimatorPool::add_animator(Animator * anim) {
    for(size_t i = 0; i < POOL_COUNT; ++i) {
      if(_animators[(_next + i) % POOL_COUNT] == nullptr) {
        _next = (_next + i) % POOL_COUNT;
        break;
      }
    }
    Debug::assert(_animators[_next] == nullptr,
      "No room left in AnimatorPool");
    _animators[_next] = anim;
    _next = (_next + 1) % POOL_COUNT;
  }


  Lights::Animator * Lights::AnimatorPool::get_animator() {
    // Try to find an idle animator
    for(size_t i = 0; i < POOL_COUNT; ++i) {
      Animator * anim = _animators[(_next + i) % POOL_COUNT];
      if(!anim->is_playing()) {
        _next = (_next + i + 1) % POOL_COUNT;
        return anim;
      }
    }
    // Because there are two animators in every pool, and Lights discards all
    // but one for any given layer before it tries to get a new animator, this
    // should only happen if you're sharing AnimatorPools between multiple
    // layers (which shouldn't be done!)
    Debug::error("No free animators in this pool!");
    return nullptr;
  }


  void Lights::Animator::on_play() {
    Debug::assert(!_transitioning, "Animator on_play() and _transitioning");
    Debug::assert(!_playing, "Animator on_play() and _playing");
    _total_time = ShortTimeSpan::from_micros(0);
    _playing = true;
  }


  void Lights::Animator::on_transition() {
    Debug::assert(_playing, "Animator on_stopping() and !_playing");
    Debug::assert(!_transitioning, "Animator on_play() and _transitioning");
    _transitioning = true;
  }


  void Lights::Animator::on_stop() {
    Debug::assert(_playing, "Animator on_stop() and !_playing");
    _playing = false;
    _transitioning = false;
  }


  bool Lights::Animator::animate(ShortTimeSpan dt) {
    _total_time += dt;
    if(_total_time > _anim_length) {
      if(_looping) {
        _total_time %= _anim_length;
        return true;
      } else {
        return false;
      }
    } else {
      return true;
    }
  }


  void Lights::Animator::render(Texture2D * dest) const {
    if(_total_time >= _anim_length) {
      render(_total_time, 1.0f, dest);
    } else {
      render(_total_time,
        static_cast<float>(_total_time.to_micros()) /
          static_cast<float>(_anim_length.to_micros()),
        dest);
    }
  }


  void Lights::init() {
    uint32_t c = Color(0.0f, 0.0f, 0.0f).to_ws2811_color32();
    for(size_t i = 0; i < 8; ++i) {
      for(size_t j = 0; j < 8; ++j) {
        // 0x00GGRRBB
        _output_buf[i][j] = c;
      }
    }
  }


  void Lights::start_animator(size_t layer, AnimatorPool * pool,
      ShortTimeSpan transition) {
    // Note that in this method we try not to disturb any existing
    // transitioning animators if it's not necessary... this is helpful in the
    // edge case where a very long transition is initiated to present a short
    // animation: the animation might naturally end before the transition. In
    // that scenario, it will be most visually seamless to start the new
    // animation in the foreground while continuing to run the same
    // transition.
    // This is kind of a pathological case because normally transitions should
    // be very short. Since the composited result could look really different
    // depending on the timing, this scenario should only be set up in the
    // first place if you know what you're getting into and the timing is
    // controlled well enough to get consistent results.

    // Is there an animator playing?
    if(_layers[layer].animator != nullptr) {
      // Is there a transition?
      if(transition > TimeSpan::zero) {
        // Yes: if anything was still transitioning, get rid of it
        if(_layers[layer].trans_animator != nullptr) {
          _layers[layer].trans_animator->on_stop();
          _layers[layer].trans_animator = nullptr;
        }

        // Next, move the current animator into transitioning
        _layers[layer].trans_animator = _layers[layer].animator;
        _layers[layer].trans_animator->on_transition();
        _layers[layer].animator = nullptr;
      } else {
        // No: stop the current animator
        _layers[layer].animator->on_stop();
        _layers[layer].animator = nullptr;
      }
    }

    // Start the new animator
    _layers[layer].animator = pool->get_animator();
    _layers[layer].animator->on_play();
  }


  void Lights::update(ShortTimeSpan dt) {
    update_animators(dt);

    Texture2D tex;

    update_composite_layers(&tex, LAYER_STALK_RED_START, LAYER_STALK_COUNT);
    update_encode_stalk_texture_to_output(STALK_PAGE_RED, &tex);

    update_composite_layers(&tex, LAYER_STALK_GREEN_START, LAYER_STALK_COUNT);
    update_encode_stalk_texture_to_output(STALK_PAGE_GREEN, &tex);

    update_composite_layers(&tex, LAYER_STALK_BLUE_START, LAYER_STALK_COUNT);
    update_encode_stalk_texture_to_output(STALK_PAGE_BLUE, &tex);

    update_composite_layers(&tex, LAYER_STALK_PINK_START, LAYER_STALK_COUNT);
    update_encode_stalk_texture_to_output(STALK_PAGE_PINK, &tex);

    update_composite_layers(&tex, LAYER_SB_START, LAYER_SB_COUNT);
    update_encode_scoreboard_texture_to_output(SCOREBOARD_PAGES_START, 4,
      &tex);
/*
    memset(_output_buf, 0, sizeof _output_buf);

    uint32_t red_color = Color(1.0f, 0.0f, 0.0f).to_ws2811_color32();
    uint32_t green_color = Color(0.0f, 1.0f, 0.0f).to_ws2811_color32();
    uint32_t blue_color = Color(0.0f, 0.0f, 1.0f).to_ws2811_color32();
    uint32_t pink_color = Color(1.0f, 0.3f, 0.3f).to_ws2811_color32();
    if(!Input::button_state(Input::B_RED)) {
      red_color = 0;
    }
    if(!Input::button_state(Input::B_GREEN)) {
      green_color = 0;
    }
    if(!Input::button_state(Input::B_BLUE)) {
      blue_color = 0;
    }
    if(!Input::button_state(Input::B_PINK)) {
      pink_color = 0;
    }
    uint32_t combined_color =
      red_color | green_color | blue_color | pink_color;
    for(size_t i = 0; i < PAGE_SIZE; ++i) {
      _output_buf[0][i] = combined_color;
      _output_buf[1][i] = red_color;
      _output_buf[2][i] = green_color;
      _output_buf[3][i] = blue_color;
      _output_buf[4][i] = pink_color;
    }
    */
  }


  void Lights::update_animators(ShortTimeSpan dt) {
    for(size_t i = 0; i < LAYER_COUNT; ++i) {
      if(_layers[i].trans_animator != nullptr) {
        _layers[i].trans_time += dt;
        if((_layers[i].trans_time >= _layers[i].trans_length) ||
            !_layers[i].trans_animator->animate(dt)) {
          _layers[i].trans_animator->on_stop();
          _layers[i].trans_animator = nullptr;
        }
      }

      if(_layers[i].animator != nullptr) {
        if(!_layers[i].animator->animate(dt)) {
          _layers[i].animator->on_stop();
          _layers[i].animator = nullptr;
        }
      }
    }
  }


  void Lights::update_composite_layers(Texture2D * tex, size_t layer_start,
      size_t layer_count) {
    Debug::assertf(_layers[layer_start].width * _layers[layer_start].height
        <= TEXTURE_TEMP_MAX, "Layer %u too big for _texture_temp",
      layer_start);
    tex->init(_layers[layer_start].width, _layers[layer_start].height,
      _texture_temp[0]);
    tex->fill_solid(Color::black);

    Texture2D temp;
    temp.init(tex->get_width(), tex->get_height(), _texture_temp[1]);

    for(size_t i = layer_start; i < layer_start + layer_count; ++i) {
      bool do_transition = (_layers[i].trans_time < _layers[i].trans_length);
      if(do_transition) {
        temp.copy(tex);
      }
      if(_layers[i].animator != nullptr) {
        _layers[i].animator->render(tex);
      }
      if(do_transition && _layers[i].trans_animator != nullptr) {
        _layers[i].trans_animator->render(&temp);
        tex->lerp(&temp,
          1.0f - static_cast<float>(_layers[i].trans_time.to_micros()) /
            static_cast<float>(_layers[i].trans_length.to_micros()));
      }
    }
  }


  void Lights::update_encode_stalk_texture_to_output(size_t page,
      Texture2D const * tex) {
    size_t w = tex->get_width();
    size_t h = tex->get_height();
    size_t i = 0;
    for(size_t y = 0; y < h && page < PAGE_COUNT; ++y) {
      for(size_t x = 0; x < w; ++x) {
        if(y % 2 == 0) {
          _output_buf[page][i++] = tex->sample(x, y).to_ws2811_color32();
        } else {
          _output_buf[page][i++] = tex->sample(w - 1 - x, y).to_ws2811_color32();
        }
      }
    }
  }


  void Lights::update_encode_scoreboard_texture_to_output(size_t page_start,
      size_t lines_per_page, Texture2D const * tex) {
    size_t w = tex->get_width();
    size_t h = tex->get_height();
    size_t i = 0;
    size_t page_lines = 0;
    size_t page = page_start;
    for(size_t y = 0; y < h && page < PAGE_COUNT; ++y) {
      for(size_t x = 0; x < w; ++x) {
        if(y % 2 == 0) {
          _output_buf[page][i++] = tex->sample(x, y).to_ws2811_color32();
        } else {
          _output_buf[page][i++] = tex->sample(w - 1 - x, y).to_ws2811_color32();
        }
      }
      ++page_lines;
      if(page_lines >= lines_per_page) {
        ++page;
        page_lines = 0;
      }
    }
  }


  void Lights::output() {
    mbed::Timer tm;
    tm.start();
    // This method is tuned for the STM32F407 DISCOVERY board, running at
    // 168MHz. If it's ported to any other board, it should be retuned.
    __disable_irq();
    hw::debug_lights_sync = 1;
    for(size_t i = 0; i < PAGE_SIZE; ++i) {
      uint32_t const set = 0b1111111111111111;
      uint32_t const reset = 0b0000000000000000;

      uint32_t colors[PAGE_COUNT];

      for(size_t j = 0; j < PAGE_COUNT; ++j) {
        colors[j] = _output_buf[j][i];
      }

      for(size_t j = 0; j < 24; ++j) {
        uint32_t data = 0b0000000000000000;
        for(size_t k = 0; k < PAGE_COUNT; ++k) {
          // shift out MSB first
          uint32_t c = colors[k];
          colors[k] = c << 1;
          data |= (c & (1 << 23)) >> (23 - k);
        }
        hw::lights_ws2812_port = set;
        // Delay -- to ~200ns
        for(int w = 0; w < 5; ++w) {
          __NOP();
        }
        hw::lights_ws2812_port = data;
        // Delay -- to ~600 ns
        for(int w = 0; w < 20; ++w) {
          __NOP();
        }
        hw::lights_ws2812_port = reset;
        // Delay -- to ~600ns (doesn't have to be quite so long, but a little
        // extra increases stability)
        for(int w = 0; w < 20; ++w) {
          __NOP();
        }
      }
    }
    hw::debug_lights_sync = 0;
    __enable_irq();
    tm.stop();
    //Debug::tracef("Lights output %uus", tm.read_us());
  }
}
