#ifndef AMANITA_ARCADE_H
#define AMANITA_ARCADE_H


#include <new>

#include <inttypes.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>

#include "mbed.h"
#include "rtos.h"

#include <Debug.h>


#define AA_OPTIMIZE __attribute__((optimize("O4")))


namespace aa {
  class TimeSpan;
  class ShortTimeSpan;

  enum MushroomID {
    M_RED,   // Stalk labeled "1"
    M_GREEN, // Stalk labeled "B"
    M_BLUE,  // Stalk labeled "A"
    M_PINK,  // Stalk labeled "2"
    M_COUNT
  };

#if 0
  static const size_t STALK_HEIGHT_RED   = 12;
  static const size_t STALK_HEIGHT_GREEN = 12;
  static const size_t STALK_HEIGHT_BLUE  = 12;
  static const size_t STALK_HEIGHT_PINK  = 12;
  static const size_t STALK_HEIGHT_MAX   = 12;
  static const size_t STALK_WIDTH = 1;
  static const size_t SCOREBOARD_WIDTH  = 8;
  static const size_t SCOREBOARD_HEIGHT = 8;
  static const float STALK_BRIGHTNESS = 0.25f;
  static const float SCOREBOARD_BRIGHTNESS = 0.25f;
#else
  // Heights are 24, 32, 36, 41
  static const size_t STALK_HEIGHT_RED   = 36;
  static const size_t STALK_HEIGHT_GREEN = 24;
  static const size_t STALK_HEIGHT_BLUE  = 41;
  static const size_t STALK_HEIGHT_PINK  = 32;
  static const size_t STALK_HEIGHT_MAX   = 41;
  static const size_t STALK_WIDTH = 3;
  static const size_t SCOREBOARD_WIDTH = 30;
  static const size_t SCOREBOARD_HEIGHT = 30;
  static const float STALK_BRIGHTNESS = 1.0f;
  static const float SCOREBOARD_BRIGHTNESS = 0.50f;
#endif

  static const size_t STALK_LIGHTS_COUNT_RED =
    STALK_HEIGHT_RED * STALK_WIDTH;
  static const size_t STALK_LIGHTS_COUNT_GREEN =
    STALK_HEIGHT_GREEN * STALK_WIDTH;
  static const size_t STALK_LIGHTS_COUNT_BLUE =
    STALK_HEIGHT_BLUE * STALK_WIDTH;
  static const size_t STALK_LIGHTS_COUNT_PINK =
    STALK_HEIGHT_PINK * STALK_WIDTH;
  static const size_t SCOREBOARD_LIGHTS_COUNT =
    SCOREBOARD_WIDTH * SCOREBOARD_HEIGHT;

  static const size_t NV_SIZE = 0x0400;
  static const size_t NV_HI_SCORES_ADDR = 0x0000;
  static const size_t NV_HI_SCORES_SIZE = 0x0200;
  static const size_t NV_DEBUG_ADDR = 0x0200;
  static const size_t NV_DEBUG_SIZE = NV_SIZE - NV_DEBUG_ADDR;

  namespace hw {
    extern UARTSerial & debug_ser;
    extern UARTSerial input_ser;
    extern PortOut lights_ws2812_port;
    extern DigitalOut debug_amber_led;
    extern DigitalOut debug_green_led;
    extern DigitalOut debug_red_led;
    extern DigitalOut debug_blue_led;
    extern DigitalOut debug_frame_sync;
    extern DigitalOut debug_lights_sync;
    //extern I2C eeprom_i2c;
  }

  class System {
  public:
    static TimeSpan uptime();
    static void init_nv();
    static bool write_nv(uint16_t mem_addr, void const * data, size_t size);
    static bool read_nv(uint16_t mem_addr, void * data, size_t size);
    static void init_watchdog(ShortTimeSpan timeout);
    static void service_watchdog();

  private:
    static bool eeprom_write_byte(uint16_t addr, uint8_t data);
    static bool eeprom_write_bytes(uint16_t addr, uint8_t const * data, uint16_t size);
    static bool eeprom_read_byte(uint16_t addr, uint8_t * val);
  };

}

#endif // AMANITA_ARCADE_H
