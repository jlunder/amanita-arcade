#include "amanita_arcade.h"

#include "aa_lights.h"

#include "aa_input.h"


namespace aa {
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
  Texture2D Lights::_composite_tex;
  Texture2D Lights::_transition_tex;
  Color Lights::_composite_tex_data[TEXTURE_TEMP_MAX];
  Color Lights::_transition_tex_data[TEXTURE_TEMP_MAX];


  void Lights::Animator::play() {
    Debug::assertf(AA_AUTO_ASSERT(_state == AS_RESET));
    _total_time = ShortTimeSpan::from_micros(0);
    _state = AS_PLAYING;
  }


  void Lights::Animator::restart() {
    Debug::assertf(AA_AUTO_ASSERT((_state == AS_PLAYING) ||
      (_state == AS_TRANSITIONING)));
    if(_end_behavior != EB_LOOP) {
      _total_time = ShortTimeSpan::from_micros(0);
    }
    _state = AS_PLAYING;
  }


  void Lights::Animator::transition() {
    Debug::assertf(AA_AUTO_ASSERT(_state == AS_PLAYING));
    _state = AS_TRANSITIONING;
  }


  void Lights::Animator::stop() {
    Debug::assertf(AA_AUTO_ASSERT((_state == AS_PLAYING) ||
      (_state == AS_TRANSITIONING)));
    _state = AS_RESET;
  }


  bool Lights::Animator::animate(ShortTimeSpan dt) {
    Debug::assertf(AA_AUTO_ASSERT(_state != AS_RESET));
    _total_time += dt;
    if(_total_time < _anim_length) {
      return true;
    }
    else if(_end_behavior == EB_LOOP) {
      _total_time %= _anim_length;
      return true;
    }
    else if(_end_behavior == EB_PAUSE) {
      _total_time = _anim_length;
      return true;
    }
    else {
      return false;
    }
  }


  void Lights::Animator::render(Texture2D * dest) const {
    if(_total_time >= _anim_length) {
      render(_total_time, 1.0f, dest);
    }
    else {
      render(_total_time,
        static_cast<float>(_total_time.to_micros()) /
          static_cast<float>(_anim_length.to_micros()),
        dest);
    }
  }


  void Lights::init() {
    uint32_t c = Color::black.to_grb_color32();
    for(size_t i = 0; i < 8; ++i) {
      for(size_t j = 0; j < 8; ++j) {
        // 0x00GGRRBB
        _output_buf[i][j] = c;
      }
    }
  }


  void Lights::start_animator(size_t layer, Animator * animator,
      ShortTimeSpan transition) {
    Layer * layer_struct = &_layers[layer];

    Debug::assertf(AA_AUTO_ASSERT(animator != nullptr));

    if(layer_struct->trans_animator == animator) {
      // Swapping back from previous transition: run the transition backwards
      // from wherever it got to
      ShortTimeSpan inferred_trans_time = ShortTimeSpan::from_micros(
        ((int64_t)layer_struct->trans_time.to_micros() *
          transition.to_micros())
        / layer_struct->trans_length.to_micros());
      if(inferred_trans_time < transition) {
        layer_struct->trans_time = transition - inferred_trans_time;
      }
      else {
        layer_struct->trans_time = TimeSpan::zero;
      }
      layer_struct->trans_length = transition;

      // Swap animator with trans_animator
      layer_struct->trans_animator = layer_struct->animator;
      if(layer_struct->trans_animator != nullptr) {
        layer_struct->trans_animator->transition();
      }
      // restart() instead of play()!
      layer_struct->animator = animator;
      layer_struct->animator->restart();
      return;
    }
    if(layer_struct->animator == nullptr) {
      // No current animator: just start the one given immediately
      layer_struct->animator = animator;
      layer_struct->animator->play();
      return;
    }
    if(layer_struct->animator == animator) {
      // Restarting same animator: don't disturb any in-progress transition
      layer_struct->animator->restart();
      return;
    }
    if(transition <= TimeSpan::zero) {
      // No transition: just kill things and replace
      if(layer_struct->animator != nullptr) {
        layer_struct->animator->stop();
      }
      layer_struct->animator = animator;
      layer_struct->animator->play();
      return;
    }

    // And finally, the regular case: there is an animator already running,
    // and we must transition from it to the new animator

    // Is there a transition request?
    if(transition > TimeSpan::zero) {
      // Yes: if anything was still transitioning, get rid of it
      if(layer_struct->trans_animator != nullptr) {
        layer_struct->trans_animator->stop();
      }

      // Next, move the current animator into transitioning
      layer_struct->trans_animator = layer_struct->animator;
      layer_struct->animator = nullptr;
      Debug::assertf(AA_AUTO_ASSERT(layer_struct->trans_animator != nullptr));
      layer_struct->trans_animator->transition();
      layer_struct->trans_time = TimeSpan::zero;
      layer_struct->trans_length = transition;
    }
    else {
      // No: just stop the current animator abruptly; don't disturb any
      // in-progress transition
      Debug::assertf(AA_AUTO_ASSERT(layer_struct->animator != nullptr));
      _layers[layer].animator->stop();
      _layers[layer].animator = nullptr;
    }

    // Start the new animator
    _layers[layer].animator = animator;
    _layers[layer].animator->play();
  }


  void Lights::transition_out(size_t layer, ShortTimeSpan transition) {
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

    Layer * layer_struct = &_layers[layer];

    if(layer_struct->animator == nullptr) {
      // No animator playing in the first place, nothing to do
      return;
    }

    // Is there a transition request?
    if(transition > TimeSpan::zero) {
      // Yes: if anything was still transitioning, get rid of it
      if(layer_struct->trans_animator != nullptr) {
        layer_struct->trans_animator->stop();
      }

      // Next, move the current animator into transitioning
      layer_struct->trans_animator = layer_struct->animator;
      layer_struct->animator = nullptr;
      Debug::assertf(AA_AUTO_ASSERT(layer_struct->trans_animator != nullptr));
      layer_struct->trans_animator->transition();
      layer_struct->trans_time = TimeSpan::zero;
      layer_struct->trans_length = transition;
    }
    else {
      // No: just stop the current animator abruptly; don't disturb any
      // in-progress transition
      Debug::assertf(AA_AUTO_ASSERT(layer_struct->animator != nullptr));
      _layers[layer].animator->stop();
      _layers[layer].animator = nullptr;
    }
  }


  void Lights::update(ShortTimeSpan dt) {
    update_animators(dt);

    for(size_t j = 0; j < PAGE_COUNT; ++j) {
      for(size_t i = 0; i < PAGE_SIZE; ++i) {
        _output_buf[j][i] = 0x00FFFF;
      }
    }

    update_composite_layers_to_composite_tex(LAYER_STALK_RED_START,
      LAYER_STALK_COUNT);
    update_encode_stalk_texture_to_output(STALK_PAGE_RED, &_composite_tex);

    update_composite_layers_to_composite_tex(LAYER_STALK_GREEN_START,
      LAYER_STALK_COUNT);
    update_encode_stalk_texture_to_output(STALK_PAGE_GREEN, &_composite_tex);

    update_composite_layers_to_composite_tex(LAYER_STALK_BLUE_START,
      LAYER_STALK_COUNT);
    update_encode_stalk_texture_to_output(STALK_PAGE_BLUE, &_composite_tex);

    update_composite_layers_to_composite_tex(LAYER_STALK_PINK_START,
      LAYER_STALK_COUNT);
    update_encode_stalk_texture_to_output(STALK_PAGE_PINK, &_composite_tex);

    update_composite_layers_to_composite_tex(LAYER_SB_START, LAYER_SB_COUNT);
    // LEDs can't handle more than 25% brightness, oy
    _composite_tex.fill_lerp(Color::black, 0.875f);
    update_encode_scoreboard_texture_to_output(SCOREBOARD_PAGES_START, 4,
      &_composite_tex);
  }


  void Lights::update_animators(ShortTimeSpan dt) {
    for(size_t i = 0; i < LAYER_COUNT; ++i) {
      if(_layers[i].trans_animator != nullptr) {
        _layers[i].trans_time += dt;
        if((_layers[i].trans_time >= _layers[i].trans_length) ||
            !_layers[i].trans_animator->animate(dt)) {
          _layers[i].trans_animator->stop();
          _layers[i].trans_animator = nullptr;
        }
      }

      if(_layers[i].animator != nullptr) {
        if(!_layers[i].animator->animate(dt)) {
          _layers[i].animator->stop();
          _layers[i].animator = nullptr;
        }
      }
    }
  }


  void Lights::update_composite_layers_to_composite_tex(size_t layer_start,
      size_t layer_count) {
    Debug::assertf(_layers[layer_start].width * _layers[layer_start].height
        <= TEXTURE_TEMP_MAX, "Layer %u too big for _texture_temp",
      layer_start);
    _composite_tex.init(_layers[layer_start].width,
      _layers[layer_start].height, _composite_tex_data);
    _composite_tex.fill_set(Color::white);

    for(size_t i = layer_start; i < layer_start + layer_count; ++i) {
      bool do_transition = (_layers[i].trans_time < _layers[i].trans_length);
      if(do_transition) {
        _transition_tex.init(_composite_tex.get_width(),
          _composite_tex.get_height(), _transition_tex_data);
        _transition_tex.fill_set(&_composite_tex);
      }
      if(_layers[i].animator != nullptr) {
        _layers[i].animator->render(&_composite_tex);
      }
      if(do_transition) {
        if(_layers[i].trans_animator != nullptr) {
          _layers[i].trans_animator->render(&_transition_tex);
        }
        // Otherwise, _transition_tex just gets whatever was in _composite_tex
        _composite_tex.fill_lerp(&_transition_tex,
          1.0f - static_cast<float>(_layers[i].trans_time.to_micros()) /
            static_cast<float>(_layers[i].trans_length.to_micros()));
      }
    }
  }


  void Lights::update_encode_stalk_texture_to_output(size_t page,
      Texture2D const * tex) {
    tex->write_brg_color32(_output_buf[page], tex->get_width(),
      tex->get_height(), false, 0, 0);
  }


  void Lights::update_encode_scoreboard_texture_to_output(size_t page_start,
      size_t lines_per_page, Texture2D const * tex) {
    size_t page = page_start;
    for(size_t y = 0; y < tex->get_height(); ++page, y += lines_per_page) {
      size_t h = lines_per_page;
      if((y + h) > tex->get_height()) {
        h = tex->get_height() - y;
      }
      tex->write_grb_color32(_output_buf[page],
        tex->get_width(), h, true, 0, y);
    }
  }


  void __attribute__((optimize("O4"))) Lights::output() {
    static uint32_t const set = 0b1111111111111111;
    static uint32_t const reset = 0b0000000000000000;

    mbed::Timer tm;

    uint32_t colors_buf[2][PAGE_COUNT];
    uint32_t * colors;
    uint32_t * colors_next = colors_buf[0];
    uint_fast16_t data[24];
    uint_fast16_t data_next;

    tm.start();

    // Warm up the pipeline
    for(size_t j = 0; j < PAGE_COUNT; ++j) {
      colors_next[j] = _output_buf[j][0];
    }

    // Second stage warmup
    colors = colors_next;
    colors_next = colors_buf[1];
    for(size_t j = 0; j < 24; ++j) {
      data_next = 0;
      for(size_t k = 0; k < PAGE_COUNT; ++k) {
        // shift out MSB first
        uint32_t c = colors[k];
        colors[k] = c << 1;
        data_next |= (c & (1 << 23)) >> (23 - k);
      }
      data[j] = data_next;
      if(j < PAGE_COUNT) {
        colors_next[j] = _output_buf[j][1];
      }
    }

    // This method is tuned for the STM32F407 DISCOVERY board, running at
    // 168MHz. If it's ported to any other board, it should be retuned.
    __disable_irq();
    hw::debug_lights_sync = 1;
    for(size_t i = 0; i < PAGE_SIZE - 1; ++i) {
      colors = colors_next;
      colors_next = colors_buf[i & 1];

      for(size_t j = 0; j < 24; ++j) {
        data_next = 0;

        __sync_synchronize();
        hw::lights_ws2812_port = set;
        __sync_synchronize();
        // Do work to make up ~350ns (200ns < t < 500ns) (no __NOP() needed)
        for(size_t k = 0; k < 6; ++k) {
          // shift out MSB first
          uint32_t c = colors[k];
          colors[k] = c << 1;
          data_next |= (c & (1 << 23)) >> (23 - k);
        }
        __sync_synchronize();
        hw::lights_ws2812_port = data[j];
        __sync_synchronize();
        // Do work to make up >300ns (max 5000ns)
        for(size_t k = 6; k < 12; ++k) {
          // shift out MSB first
          uint32_t c = colors[k];
          colors[k] = c << 1;
          data_next |= (c & (1 << 23)) >> (23 - k);
        }
        __sync_synchronize();
        hw::lights_ws2812_port = reset;
        __sync_synchronize();
        // Do work to make up >250ns (max 5000ns)
        for(size_t k = 12; k < PAGE_COUNT; ++k) {
          // shift out MSB first
          uint32_t c = colors[k];
          colors[k] = c << 1;
          data_next |= (c & (1 << 23)) >> (23 - k);
        }
        data[j] = data_next;
        if(j < PAGE_COUNT) {
          colors_next[j] = _output_buf[j][i + 2];
        }
      }
    }
    hw::debug_lights_sync = 0;
    __enable_irq();

    tm.stop();
    //Debug::tracef("Lights output %uus", tm.read_us());
  }
}
