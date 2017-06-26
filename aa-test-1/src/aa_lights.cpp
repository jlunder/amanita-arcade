#include "amanita_arcade.h"

#include "aa_lights.h"


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

  uint32_t Lights::color_buf[PAGE_COUNT][PAGE_SIZE];

  void Lights::init() {
    uint32_t c = Color(0.0f, 0.0f, 0.0f).to_ws2811_color32();
    for(size_t i = 0; i < 8; ++i) {
      for(size_t j = 0; j < 8; ++j) {
        // 0x00GGRRBB
        color_buf[i][j] = c;
      }
    }
  }

  void Lights::update(ShortTimeSpan dt) {
    for(size_t i = 0; i < PAGE_COUNT; ++i) {
      for(size_t j = 0; j < PAGE_SIZE; ++j) {
        color_buf[i][j] = Color(0.0f, 0.0f, 0.1f).to_ws2811_color32();
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
        colors[j] = color_buf[j][i];
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
