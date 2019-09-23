#include "amanita_arcade.h"

#include <stm32f4xx.h>
#include <stm32f4xx_hal.h>

#include "aa_lights.h"

#include "aa_input.h"


namespace aa {
  namespace {
    // There's some inconsistency in the timing of the first DMA transfer to
    // the GPIOs, so we add an extra word which should just repeat the previous
    // state to hide the jitter. This has the nice side effect of guaranteeing
    // that the bus is low before the first low-to-high transition.
    static uint16_t lights_dma_buf[Lights::PAGE_SIZE * 24 * 3 + 1];
    static TIM_HandleTypeDef lights_dma_tim; // TIM1
    static DMA_HandleTypeDef lights_dma; // DMA2 stream 5

    static void lights_dma_init() {
      __HAL_RCC_TIM1_CLK_ENABLE();
      __HAL_RCC_DMA2_CLK_ENABLE();

      lights_dma_tim.Instance = TIM1;

      lights_dma_tim.Init.Prescaler = 0;
      lights_dma_tim.Init.CounterMode = TIM_COUNTERMODE_UP;
      lights_dma_tim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
      lights_dma_tim.Init.RepetitionCounter = 0;
      lights_dma_tim.Init.Period = SystemCoreClock / 4000000UL - 1; // 4MHz = 250ns

      TIM_ClockConfigTypeDef tim_clock;
      tim_clock.ClockSource = TIM_CLOCKSOURCE_INTERNAL;

      TIM_MasterConfigTypeDef tim_master;
      tim_master.MasterOutputTrigger = TIM_TRGO_RESET;
      tim_master.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;

      lights_dma.Instance = DMA2_Stream5;

      lights_dma.Init.Channel = DMA_CHANNEL_6;
      lights_dma.Init.Direction = DMA_MEMORY_TO_PERIPH;
      lights_dma.Init.PeriphInc = DMA_PINC_DISABLE;
      lights_dma.Init.MemInc = DMA_MINC_ENABLE;
      lights_dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
      lights_dma.Init.MemDataAlignment = DMA_PDATAALIGN_HALFWORD;
      lights_dma.Init.Mode = DMA_NORMAL;
      lights_dma.Init.Priority = DMA_PRIORITY_VERY_HIGH;
      lights_dma.Init.FIFOMode = DMA_FIFOMODE_ENABLE;
      lights_dma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
      lights_dma.Init.MemBurst = DMA_MBURST_SINGLE;
      lights_dma.Init.PeriphBurst = DMA_PBURST_SINGLE;

      Debug::assert(HAL_TIM_Base_Init(&lights_dma_tim) == HAL_OK,
        "Timer init failed");
      Debug::assert(
        HAL_TIM_ConfigClockSource(&lights_dma_tim, &tim_clock) == HAL_OK,
        "Timer clock source config failed");
      Debug::assert(
        HAL_TIMEx_MasterConfigSynchronization(&lights_dma_tim, &tim_master)
          == HAL_OK,
        "Timer master sync config failed");
      Debug::assert(HAL_DMA_Init(&lights_dma) == HAL_OK, "DMA init failed");
      __HAL_LINKDMA(&lights_dma_tim, hdma[TIM_DMA_ID_UPDATE], lights_dma);
      __HAL_TIM_ENABLE_DMA(&lights_dma_tim, TIM_DMA_UPDATE);
      __HAL_TIM_ENABLE(&lights_dma_tim);

      static uint32_t const set = 0b1111111111111111;
      static uint32_t const reset = 0b0000000000000000;
      size_t n = 0;
      lights_dma_buf[n++] = reset;
      while(n + 3 <= (sizeof lights_dma_buf / sizeof *lights_dma_buf)) {
        lights_dma_buf[n++] = set;
        lights_dma_buf[n++] = reset;
        lights_dma_buf[n++] = reset;
      }
    }
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
  Texture2D Lights::_composite_tex;
  Texture2D Lights::_transition_tex;
  Color Lights::_composite_tex_data[TEXTURE_TEMP_MAX];
  Color Lights::_transition_tex_data[TEXTURE_TEMP_MAX];


  void Lights::Animator::play() {
    Debug::auto_assert(_state == AS_RESET);
    _total_time = ShortTimeSpan::from_micros(0);
    _state = AS_PLAYING;
  }


  void Lights::Animator::restart() {
    Debug::auto_assert((_state == AS_PLAYING) ||
      (_state == AS_TRANSITIONING));
    if(_end_behavior != EB_LOOP) {
      _total_time = ShortTimeSpan::from_micros(0);
    }
    _state = AS_PLAYING;
  }


  void Lights::Animator::transition() {
    Debug::auto_assert(_state == AS_PLAYING);
    _state = AS_TRANSITIONING;
  }


  void Lights::Animator::stop() {
    Debug::auto_assert((_state == AS_PLAYING) ||
      (_state == AS_TRANSITIONING));
    _state = AS_RESET;
  }


  bool Lights::Animator::animate(ShortTimeSpan dt) {
    Debug::auto_assert(_state != AS_RESET);
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

    lights_dma_init();
  }


  void Lights::start_animator(size_t layer, Animator * animator,
      ShortTimeSpan transition) {
    Layer * layer_struct = &_layers[layer];

    Debug::auto_assert(animator != nullptr);

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
      Debug::auto_assert(layer_struct->trans_animator != nullptr);
      layer_struct->trans_animator->transition();
      layer_struct->trans_time = TimeSpan::zero;
      layer_struct->trans_length = transition;
    }
    else {
      // No: just stop the current animator abruptly; don't disturb any
      // in-progress transition
      Debug::auto_assert(layer_struct->animator != nullptr);
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
      Debug::auto_assert(layer_struct->trans_animator != nullptr);
      layer_struct->trans_animator->transition();
      layer_struct->trans_time = TimeSpan::zero;
      layer_struct->trans_length = transition;
    }
    else {
      // No: just stop the current animator abruptly; don't disturb any
      // in-progress transition
      Debug::auto_assert(layer_struct->animator != nullptr);
      _layers[layer].animator->stop();
      _layers[layer].animator = nullptr;
    }
  }


  void Lights::update(ShortTimeSpan dt) {
    update_animators(dt);

    for(size_t j = 0; j < PAGE_COUNT; ++j) {
      for(size_t i = 0; i < PAGE_SIZE; ++i) {
        _output_buf[j][i] = 0x101010;
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
          _layers[i].trans_length = TimeSpan::zero;
          _layers[i].trans_time = TimeSpan::zero;
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

//#define DEBUG_COMPOSITOR
  void Lights::update_composite_layers_to_composite_tex(size_t layer_start,
      size_t layer_count) {
    Debug::assertf(_layers[layer_start].width * _layers[layer_start].height
        <= TEXTURE_TEMP_MAX, "Layer %u too big for _texture_temp",
      layer_start);
    _composite_tex.init(_layers[layer_start].width,
      _layers[layer_start].height, _composite_tex_data);
    _composite_tex.fill_set(Color::white);
    _transition_tex.init(_composite_tex.get_width(),
      _composite_tex.get_height(), _transition_tex_data);

#ifdef DEBUG_COMPOSITOR
    static Color original_data[900];
    static Color temp_composite_data[900];
    Texture2D original_tex, temp_composite_tex;
    original_tex.init(
      _composite_tex.get_width(), _composite_tex.get_height(),
      original_data);
    temp_composite_tex.init(
      _composite_tex.get_width(), _composite_tex.get_height(),
      temp_composite_data);
    temp_composite_tex.fill_set(&_composite_tex);
#endif

    for(size_t i = layer_start; i < layer_start + layer_count; ++i) {
      bool do_transition = (_layers[i].trans_time < _layers[i].trans_length);
      if(do_transition) {
#ifdef DEBUG_COMPOSITOR
        _transition_tex.fill_set(&temp_composite_tex);
#else
        _transition_tex.fill_set(&_composite_tex);
#endif
      }
#ifdef DEBUG_COMPOSITOR
      original_tex.fill_set(Color::white);
#endif
      if(_layers[i].animator != nullptr) {
#ifdef DEBUG_COMPOSITOR
        _layers[i].animator->render(&temp_composite_tex);
        _layers[i].animator->render(&original_tex);
#else
        _layers[i].animator->render(&_composite_tex);
#endif
      }
#ifdef DEBUG_COMPOSITOR
      size_t x0 =
        (i - layer_start) * _composite_tex.get_width() / layer_count;
      size_t x1 =
        (i - layer_start + 1) * _composite_tex.get_width() / layer_count;
      size_t y0 = 0;
      size_t y2 = _composite_tex.get_height() / 2;
      size_t y3 = _composite_tex.get_height();
#endif
      if(do_transition) {
        // _transition_tex was init'ed earlier with a copy of whatever was in
        // _composite_tex before, so if there's no trans_animator here to
        // mutate that, we'll just use it straight
        if(_layers[i].trans_animator != nullptr) {
          _layers[i].trans_animator->render(&_transition_tex);
        }
        float trans_a =
          1.0f - static_cast<float>(_layers[i].trans_time.to_micros()) /
          static_cast<float>(_layers[i].trans_length.to_micros());
#ifdef DEBUG_COMPOSITOR
        size_t y1 = (1.0f - trans_a) * y2;
        _composite_tex.box_set(x0, y0, x1 - x0, y1 - y0,
          &temp_composite_tex, x0, y0);
        _composite_tex.box_set(x0, y1, x1 - x0, y2 - y1,
          &_transition_tex, x0, y1);
        temp_composite_tex.fill_lerp(&_transition_tex, trans_a);
      } else {
        _composite_tex.box_set(x0, y0, x1 - x0, y2 - y0,
          &temp_composite_tex, x0, y0);
#else
        _composite_tex.fill_lerp(&_transition_tex, trans_a);
#endif
      }
#ifdef DEBUG_COMPOSITOR
      _composite_tex.box_set(x0, y2, x1 - x0, y3 - y2,
        &original_tex, x0, y2);
#endif
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


  void AA_OPTIMIZE Lights::output() {
    mbed::Timer tm;

    uint32_t colors[PAGE_COUNT];

    tm.start();

    size_t n = 1;

    for(size_t i = 0; i < PAGE_SIZE; ++i) {
      for(size_t j = 0; j < PAGE_COUNT; ++j) {
        colors[j] = _output_buf[j][i];
      }

      for(size_t j = 0; j < 24; ++j) {
        uint_fast16_t data = 0;
        for(size_t k = 0; k < PAGE_COUNT; ++k) {
          // shift out MSB first
          uint32_t c = colors[k];
          colors[k] = c << 1;
          data |= (c & (1 << 23)) >> (23 - k);
        }
        Debug::dev_auto_assert(
          n + 3 <= sizeof lights_dma_buf / sizeof *lights_dma_buf);
        n++;
        lights_dma_buf[n++] = data;
        n++;
      }
    }

    //for(size_t i = 0; i < (sizeof dma_buf / sizeof *dma_buf); ++i) {
    //  aa::hw::lights_ws2812_port.write(dma_buf[i]);
    //}

    HAL_DMA_PollForTransfer(&lights_dma, HAL_DMA_FULL_TRANSFER, 10);
    __HAL_TIM_DISABLE(&lights_dma_tim);
    __HAL_TIM_SET_COUNTER(&lights_dma_tim, 0);
    Debug::assert(
      HAL_DMA_Start(&lights_dma, (uint32_t)lights_dma_buf,
          (uint32_t)&GPIOE->ODR,
          sizeof lights_dma_buf / sizeof *lights_dma_buf)
        == HAL_OK,
      "DMA transfer start failed");
    __HAL_TIM_ENABLE(&lights_dma_tim);
    uint32_t micros = tm.read_us();
    Debug::tracef("lights output %luus", (unsigned long)micros);
  }
}
