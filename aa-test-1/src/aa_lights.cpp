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
      "    WWRRRRRRRRRRRRRRR         "
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
      "            XXwwwwX           "
      "             XwwXX            "
      "              XX              ";
  }

  uint32_t Lights::color_buf[LIGHTS_BUF_PAGE_COUNT][LIGHTS_BUF_PAGE_SIZE];

  void Lights::init() {
    for(size_t i = 0; i < 8; ++i) {
      for(size_t j = 0; j < 8; ++j) {
        // 0x00BBRRGG
        color_buf[i][j] = 0x0000FF00;
      }
    }
  }

  void Lights::update(ShortTimeSpan dt) {
    mbed::Timer tm;
    tm.start();
    __disable_irq();
    for(size_t i = 0; i < LIGHTS_BUF_PAGE_SIZE; ++i) {
      hw::debug_lights_sync = 1;
      uint32_t const set = 0b1111111111111111;
      uint32_t const reset = 0b0000000000000000;

      uint32_t colors[LIGHTS_BUF_PAGE_COUNT];

      for(size_t j = 0; j < LIGHTS_BUF_PAGE_COUNT; ++j) {
        colors[j] = color_buf[j][i];
      }

      hw::debug_lights_sync = 0;
      for(size_t j = 0; j < 24; ++j) {
        uint32_t data = 0b0000000000000000;
        for(size_t k = 0; k < LIGHTS_BUF_PAGE_COUNT; ++k) {
          // shift out MSB first
          colors[k] = colors[k] << 1;
          data |= (colors[k] & (1 << 25)) >> (25 - k);
        }
        hw::lights_ws2812_port = set;
        // wait 200 ns
        for(int w = 0; w < 5; ++w) {
          __NOP();
        }
        hw::lights_ws2812_port = set;//data;
        // wait 300 ns
        for(int w = 0; w < 20; ++w) {
          __NOP();
        }
        hw::lights_ws2812_port = reset;
        // wait min 300 ns
        for(int w = 0; w < 20; ++w) {
          __NOP();
        }
      }
    }
    __enable_irq();
    tm.stop();
    //Debug::tracef("Lights output %uus", tm.read_us());
  }
}
