#include "amanita_arcade.h"

#include "aa_lights.h"


namespace aa {
  uint8_t Lights::lights_buf[LIGHTS_BUF_SIZE];

  void Lights::init() {
    hardware::stalk_lights_ser.baud(2400000);
    hardware::stalk_lights_ser.format(7, SerialBase::None, 1);
  }

  void Lights::update(ShortTimeSpan dt) {
    mbed::Timer tm;
    tm.start();
    for(size_t i = 0; (i + 7) < LIGHTS_BUF_SIZE; i += 8) {
      uint8_t r = 0xFF, g = 0x00, b = 0x00;
      lights_buf[i + 0] = 0x10010010 |
        ((((~r) & 0b10000000) >> 7) << 0) |
        ((((~r) & 0b01000000) >> 6) << 3) |
        ((((~r) & 0b00100000) >> 5) << 6);
      lights_buf[i + 1] = 0x10010010 |
        ((((~r) & 0b00010000) >> 4) << 0) |
        ((((~r) & 0b00001000) >> 3) << 3) |
        ((((~r) & 0b00000100) >> 2) << 6);
      lights_buf[i + 2] = 0x10010010 |
        ((((~r) & 0b00000010) >> 1) << 0) |
        ((((~r) & 0b00000001) >> 0) << 3) |
        ((((~g) & 0b10000000) >> 7) << 6);
      lights_buf[i + 3] = 0x10010010 |
        ((((~g) & 0b01000000) >> 6) << 0) |
        ((((~g) & 0b00100000) >> 5) << 3) |
        ((((~g) & 0b00010000) >> 4) << 6);
      lights_buf[i + 4] = 0x10010010 |
        ((((~g) & 0b00001000) >> 3) << 0) |
        ((((~g) & 0b00000100) >> 2) << 3) |
        ((((~g) & 0b00000010) >> 1) << 6);
      lights_buf[i + 5] = 0x10010010 |
        ((((~g) & 0b00000001) >> 0) << 0) |
        ((((~b) & 0b10000000) >> 7) << 3) |
        ((((~b) & 0b01000000) >> 6) << 6);
      lights_buf[i + 6] = 0x10010010 |
        ((((~b) & 0b00100000) >> 5) << 0) |
        ((((~b) & 0b00010000) >> 4) << 3) |
        ((((~b) & 0b00001000) >> 3) << 6);
      lights_buf[i + 7] = 0x10010010 |
        ((((~b) & 0b00000100) >> 2) << 0) |
        ((((~b) & 0b00000010) >> 1) << 3) |
        ((((~b) & 0b00000001) >> 0) << 6);
    }
    tm.stop();
    Debug::tracef("Lights pack %uus", tm.read_us());
  }
}
