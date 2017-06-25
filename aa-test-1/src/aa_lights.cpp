#include "amanita_arcade.h"

#include "aa_lights.h"


namespace aa {
  uint8_t Lights::lights_buf[LIGHTS_BUF_SIZE];

  void Lights::init() {
    hardware::lights_ws2812_ser.baud(2400000);
    hardware::lights_ws2812_ser.format(7, SerialBase::None, 1);
  }

  void Lights::update(ShortTimeSpan dt) {
    mbed::Timer tm;
    tm.start();
    for(size_t i = 0; (i + 7) < LIGHTS_BUF_SIZE; i += 8) {
      uint8_t r = 0xFF, g = 0x00, b = 0x00;
      uint8_t x = g, y = r, z = b;
      __disable_irq();
      hardware::lights_ws2812_ser.putc(0b10010010 |
        ((((~x) & 0b10000000) >> 7) << 0) |
        ((((~x) & 0b01000000) >> 6) << 3) |
        ((((~x) & 0b00100000) >> 5) << 6));
      hardware::lights_ws2812_ser.putc(0b10010010 |
        ((((~x) & 0b00010000) >> 4) << 0) |
        ((((~x) & 0b00001000) >> 3) << 3) |
        ((((~x) & 0b00000100) >> 2) << 6));
      hardware::lights_ws2812_ser.putc(0b10010010 |
        ((((~x) & 0b00000010) >> 1) << 0) |
        ((((~x) & 0b00000001) >> 0) << 3) |
        ((((~y) & 0b10000000) >> 7) << 6));
      hardware::lights_ws2812_ser.putc(0b10010010 |
        ((((~y) & 0b01000000) >> 6) << 0) |
        ((((~y) & 0b00100000) >> 5) << 3) |
        ((((~y) & 0b00010000) >> 4) << 6));
      hardware::lights_ws2812_ser.putc(0b10010010 |
        ((((~y) & 0b00001000) >> 3) << 0) |
        ((((~y) & 0b00000100) >> 2) << 3) |
        ((((~y) & 0b00000010) >> 1) << 6));
      hardware::lights_ws2812_ser.putc(0b10010010 |
        ((((~y) & 0b00000001) >> 0) << 0) |
        ((((~z) & 0b10000000) >> 7) << 3) |
        ((((~z) & 0b01000000) >> 6) << 6));
      hardware::lights_ws2812_ser.putc(0b10010010 |
        ((((~z) & 0b00100000) >> 5) << 0) |
        ((((~z) & 0b00010000) >> 4) << 3) |
        ((((~z) & 0b00001000) >> 3) << 6));
      hardware::lights_ws2812_ser.putc(0b10010010 |
        ((((~z) & 0b00000100) >> 2) << 0) |
        ((((~z) & 0b00000010) >> 1) << 3) |
        ((((~z) & 0b00000001) >> 0) << 6));
        __enable_irq();
    }
    tm.stop();
    Debug::tracef("Lights output %uus", tm.read_us());
    /*
    tm.start();
    for(size_t i = 0; (i + 7) < LIGHTS_BUF_SIZE; i += 8) {
      uint8_t r = 0x00, g = 0x00, b = 0xFF;
      uint8_t x = g, y = r, z = b;
      lights_buf[i + 0] = 0b10010010 |
        ((((~x) & 0b10000000) >> 7) << 0) |
        ((((~x) & 0b01000000) >> 6) << 3) |
        ((((~x) & 0b00100000) >> 5) << 6);
      lights_buf[i + 1] = 0b10010010 |
        ((((~x) & 0b00010000) >> 4) << 0) |
        ((((~x) & 0b00001000) >> 3) << 3) |
        ((((~x) & 0b00000100) >> 2) << 6);
      lights_buf[i + 2] = 0b10010010 |
        ((((~x) & 0b00000010) >> 1) << 0) |
        ((((~x) & 0b00000001) >> 0) << 3) |
        ((((~y) & 0b10000000) >> 7) << 6);
      lights_buf[i + 3] = 0b10010010 |
        ((((~y) & 0b01000000) >> 6) << 0) |
        ((((~y) & 0b00100000) >> 5) << 3) |
        ((((~y) & 0b00010000) >> 4) << 6);
      lights_buf[i + 4] = 0b10010010 |
        ((((~y) & 0b00001000) >> 3) << 0) |
        ((((~y) & 0b00000100) >> 2) << 3) |
        ((((~y) & 0b00000010) >> 1) << 6);
      lights_buf[i + 5] = 0b10010010 |
        ((((~y) & 0b00000001) >> 0) << 0) |
        ((((~z) & 0b10000000) >> 7) << 3) |
        ((((~z) & 0b01000000) >> 6) << 6);
      lights_buf[i + 6] = 0b10010010 |
        ((((~z) & 0b00100000) >> 5) << 0) |
        ((((~z) & 0b00010000) >> 4) << 3) |
        ((((~z) & 0b00001000) >> 3) << 6);
      lights_buf[i + 7] = 0b10010010 |
        ((((~z) & 0b00000100) >> 2) << 0) |
        ((((~z) & 0b00000010) >> 1) << 3) |
        ((((~z) & 0b00000001) >> 0) << 6);
    }
    tm.stop();
    Debug::tracef("Lights pack %uus", tm.read_us());
    tm.start();
    static_cast<FileLike *>(&hardware::lights_ws2812_ser)->write(
      lights_buf, sizeof lights_buf
    );
    tm.stop();
    Debug::tracef("Lights output %uus", tm.read_us());
    */
  }
}
