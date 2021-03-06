#include "amanita_arcade.h"

#include "aa_game.h"
#include "aa_input.h"
#include "aa_lights.h"
#include "aa_time_span.h"
#include "aa_timer.h"


#define AA_MAX_INDENT 16
// 50 FPS
// This is limited mostly by comms with the input board, could go to 100FPS
// comfortably if that code were properly async
#define AA_FRAME_MICROS 20000


namespace aa {
  namespace hw {
    static void debug_ser_init() {
      static bool initialized = false;
      if(!initialized) {
        initialized = true;
        new(&debug_ser) UARTSerial(PA_2, PA_3, 115200);
        debug_ser.set_blocking(true);
        char const * initial_msg = "!DEBUG OUTPUT BEGIN\r\n";
        debug_ser.write(initial_msg, strlen(initial_msg));
      }
    }
  }
}


namespace mbed
{
    FileHandle *mbed_target_override_console(int) {
      aa::hw::debug_ser_init();
      return &aa::hw::debug_ser;
    }
}


void Debug::service_watchdog() {
  aa::System::service_watchdog();
}


namespace aa {
  class Program {
  public:
    static void main();
  };


  namespace {
    static aa::Timer disconnected_min_pulse_timeout(TimeSpan::from_millis(0), false);
    static aa::Timer heartbeat_cycle(TimeSpan::from_millis(1000));
    static bool was_input_connected;

    /*
    static __attribute__((section(".text"),aligned(4096)))
      uint8_t nv_data[4096] = { 0xFF, 0xFF, 0xFF, 0xFF, };
    */
  }


  namespace hw {
    static const uint32_t eeprom_i2c_settle_us = 5;
    static const uint32_t eeprom_i2c_reset_power_off_us = 1200; // 1ms min
    static const uint32_t eeprom_i2c_reset_power_on_us = 200; // 100us min

    __attribute__((aligned(4))) uint8_t debug_ser_alloc[sizeof (UARTSerial)];
    UARTSerial & debug_ser = *(UARTSerial *)debug_ser_alloc; // PA_2, PA_3: USART2
    UARTSerial input_ser(PB_10, PB_11, 115200); // USART3
    PortOut lights_ws2812_port(PortE);
    DigitalOut debug_amber_led(LED3);
    DigitalOut debug_green_led(LED4);
    DigitalOut debug_red_led(LED5);
    DigitalOut debug_blue_led(LED6);
    DigitalOut debug_frame_sync(PD_0);
    DigitalOut debug_lights_sync(PD_1);
    DigitalOut debug_misc(PD_2);
    static const PinName eeprom_i2c_sda_pin = PC_9;
    static const PinName eeprom_i2c_scl_pin = PA_8;
    __attribute__((aligned(4))) uint8_t eeprom_i2c_alloc[sizeof (I2C)];
    I2C & eeprom_i2c(*(I2C *)eeprom_i2c_alloc);
    bool eeprom_i2c_is_live = false;
    DigitalIn eeprom_i2c_sda(eeprom_i2c_sda_pin, PullUp);
    DigitalOut eeprom_i2c_vcc(PC_6, 0);
    DigitalOut eeprom_i2c_wp(PC_7, 0);

    static bool eeprom_i2c_clear_bus();
    static void eeprom_i2c_shutdown();

    static void eeprom_i2c_create()
    {
      eeprom_i2c_is_live = true;
      new(&eeprom_i2c) I2C(eeprom_i2c_sda_pin, eeprom_i2c_scl_pin);
    }

    static void eeprom_i2c_destroy()
    {
      eeprom_i2c.~I2C();
      eeprom_i2c_is_live = false;
    }

    static bool eeprom_i2c_init()
    {
      // Ensure minimum power-off time is observed, in case we were powered
      // up and the entire system was reset
      eeprom_i2c_shutdown();

      eeprom_i2c_vcc.write(1);
      eeprom_i2c_wp.write(0);
      wait_us(eeprom_i2c_reset_power_on_us);
      if(eeprom_i2c_clear_bus()) {
        eeprom_i2c_create();
        return true;
      }
      else {
        return false;
      }
    }

    static void eeprom_i2c_shutdown()
    {
      if(eeprom_i2c_is_live) {
        eeprom_i2c_destroy();
      }
      eeprom_i2c_wp.write(0);
      eeprom_i2c_vcc.write(0);
      wait_us(eeprom_i2c_reset_power_off_us);
    }

    static bool eeprom_i2c_clear_bus()
    {
      if(eeprom_i2c_sda.read() != 0) {
        // Bus released!
        return true;
      }

      bool was_live = eeprom_i2c_is_live;
      // Bus locked, we gotta try to unstick it
      if(was_live) {
        eeprom_i2c_destroy();
      }

      bool result = false;
      { // Scope to trigger destruction of scl
        DigitalInOut scl(eeprom_i2c_scl_pin);
        scl.mode(PullUp);

        wait_us(eeprom_i2c_settle_us);
        if(scl.read() == 0) {
          Debug::trace("EEPROM SCL stuck low, hardware fault!");
        }
        else {
          scl.write(0);
          for(int i = 0; i < 20; ++i) {
            if(eeprom_i2c_sda.read() != 0) {
              // Bus released!
              result = true;
              break;
            }
            scl.output();
            wait_us(eeprom_i2c_settle_us);
            scl.input();
            wait_us(eeprom_i2c_settle_us);
          }

          if(!result) {
            Debug::trace("EEPROM SDA stuck low, bus clear failed!");
          }
        }
      }

      // fall through
      if(was_live) {
        eeprom_i2c_create();
      }
      return result;
    }

    static void eeprom_i2c_start() {
      eeprom_i2c.start();
    }

    static void eeprom_i2c_stop() {
      eeprom_i2c.stop();
    }

    static bool eeprom_i2c_read(uint8_t * val) {
      *val = eeprom_i2c.read(I2C::ACK);
      return true;
    }

    static bool eeprom_i2c_write(uint8_t val, bool * ack) {
      int res = eeprom_i2c.write(val);
      *ack = (res == I2C::ACK);
      return (res == I2C::ACK) || (res == I2C::NoACK);
    }
  }


  TimeSpan System::uptime() {
    static mbed::Timer timer;
    static bool timer_started = false;
    static uint32_t last_reading;
    static int64_t total_micros;

    if(!timer_started) {
      Debug::trace("Starting timer for the first time");
      timer_started = true;
      timer.start();
      last_reading = timer.read_us();
    }

    uint32_t this_reading = timer.read_us();
    uint32_t delta = this_reading - last_reading;

    last_reading = this_reading;
    total_micros += delta;

    return TimeSpan::from_micros(total_micros);
  }

  void System::init_nv()
  {
    Debug::push_context("EEPROM Init");

    Debug::trace("Hardware reset");
    if(!hw::eeprom_i2c_init()) {
      Debug::trace("Init failed");
      return;
    }
    Debug::trace("Reset complete, bus clear");
  }


  bool System::write_nv(uint16_t mem_addr, void const * data, size_t size) {
    static size_t const page_size = 16;
    bool ack;
    size_t page_end;

    for(size_t page_addr = mem_addr; page_addr < (mem_addr + size);
        page_addr = page_end) {
      uint8_t const * page_data = ((uint8_t const *)data) + (page_addr - mem_addr);
      page_end = (page_addr & ~(page_size - 1)) + page_size;
      //Debug::tracef("Write to EEPROM at %04X, %d bytes", (int)page_addr,
      //  (int)(page_end - page_addr));

      uint8_t addr0 = 0b10100000 | (((page_addr >> 8) & 0b11) << 1) | 0;
      uint8_t addr1 = (page_addr >> 0) & 0xFF;
      hw::eeprom_i2c_start();

      mbed::Timer write_timer;
      write_timer.start();
      do {
        if(!hw::eeprom_i2c_write(addr0, &ack)) {
          hw::eeprom_i2c_clear_bus();
          hw::eeprom_i2c_start();
          if(!hw::eeprom_i2c_write(addr0, &ack)) {
            Debug::trace("EEPROM I2C write device address write bus fault");
            return false;
          }
        }
      } while(!ack && (write_timer.read_ms() < 10));
      if(!ack) {
        Debug::trace("EEPROM I2C write device address write NAK");
        return false;
      }

      if(!hw::eeprom_i2c_write(addr1, &ack)) {
        Debug::trace("EEPROM I2C write mem address write bus fault");
        return false;
      }
      if(!ack) {
        Debug::trace("EEPROM I2C write mem address NAK");
        return false;
      }

      for(size_t i = 0; i < (page_end - page_addr); ++i) {
        //Debug::tracef("byte %02X", page_data[i]);
        if(!hw::eeprom_i2c_write(page_data[i], &ack)) {
          Debug::trace("EEPROM I2C write data bus fault");
          return false;
        }
        if(!ack) {
          Debug::trace("EEPROM I2C write data NAK");
          return false;
        }
      }

      hw::eeprom_i2c_stop();
    }

    return true;
  }


  bool System::read_nv(uint16_t mem_addr, void * data, size_t size) {
    uint8_t addr0 = 0b10100000 | (((mem_addr >> 8) & 0b11) << 1);
    uint8_t addr1 = (mem_addr >> 0) & 0xFF;
    bool ack;

    //Debug::tracef("Read from EEPROM at %04X, %d bytes", (int)mem_addr, (int)size);

    hw::eeprom_i2c_start();

    mbed::Timer write_timer;
    write_timer.start();
    do {
      if(!hw::eeprom_i2c_write(addr0, &ack)) {
        hw::eeprom_i2c_clear_bus();
        hw::eeprom_i2c_start();
        if(!hw::eeprom_i2c_write(addr0, &ack)) {
          Debug::trace("EEPROM I2C read device address write bus fault");
          return false;
        }
      }
    } while(!ack && (write_timer.read_ms() < 10));
    if(!ack) {
      Debug::trace("EEPROM I2C read device address write NAK");
      return false;
    }

    if(!hw::eeprom_i2c_write(addr1, &ack)) {
      Debug::trace("EEPROM I2C read mem address write bus fault");
      return false;
    }
    if(!ack) {
      Debug::trace("EEPROM I2C read mem address NAK");
      return false;
    }

    hw::eeprom_i2c_start();

    if(!hw::eeprom_i2c_write(addr0 | 1,
        &ack)) {
      Debug::trace("EEPROM I2C read device address read bus fault");
      return false;
    }
    if(!ack) {
      Debug::trace("EEPROM I2C read device address read NAK");
      return false;
    }

    for(size_t i = 0; i < size; ++i) {
      uint8_t val = 0;
      if(!hw::eeprom_i2c_read(&val)) {
        Debug::tracef("EEPROM I2C read data bus fault");
        return false;
      }
      ((uint8_t *)data)[i] = val;
    }

    hw::eeprom_i2c_stop();

    return true;
  }


  void System::init_watchdog(ShortTimeSpan timeout) {
    static uint64_t const lsi_freq = 32768;

    uint64_t timeout_micros = timeout.to_micros();
    uint16_t prescaler_code;
    uint16_t prescaler;
    uint16_t reload_value;

    if ((timeout_micros * (lsi_freq / 4)) < 0x7FF * 1000000LLU) {
      prescaler_code = IWDG_PRESCALER_4;
      prescaler = 4;
    }
    else if ((timeout_micros * (lsi_freq / 8)) < 0xFF0 * 1000000LLU) {
      prescaler_code = IWDG_PRESCALER_8;
      prescaler = 8;
    }
    else if ((timeout_micros * (lsi_freq / 16)) < 0xFF0 * 1000000LLU) {
      prescaler_code = IWDG_PRESCALER_16;
      prescaler = 16;
    }
    else if ((timeout_micros * (lsi_freq / 32)) < 0xFF0 * 1000000LLU) {
      prescaler_code = IWDG_PRESCALER_32;
      prescaler = 32;
    }
    else if ((timeout_micros * (lsi_freq / 64)) < 0xFF0 * 1000000LLU) {
      prescaler_code = IWDG_PRESCALER_64;
      prescaler = 64;
    }
    else if ((timeout_micros * (lsi_freq / 128)) < 0xFF0 * 1000000LLU) {
      prescaler_code = IWDG_PRESCALER_128;
      prescaler = 128;
    }
    else {
      prescaler_code = IWDG_PRESCALER_256;
      prescaler = 256;
    }

    // specifies the IWDG Reload value. This parameter must be a number between 0 and 0x0FFF.
    reload_value =
      (uint32_t)((timeout_micros * (lsi_freq / prescaler) + 500000)
        / 1000000);

    /*
    uint64_t calculated_timeout_micros =
      (uint32_t)(((float)(prescaler * reload_value) * 1e6f)
        / lsi_freq + 0.5f);
    Debug::tracef(
      "Set WDT to %dx%d from desired timeout %lluus; actual %lluus",
      prescaler, reload_value, timeout_micros, calculated_timeout_micros);
    */

    IWDG->KR = 0x5555; // Disable write protection of IWDG registers
    IWDG->PR = prescaler_code; // Set PR value
    IWDG->RLR = reload_value; // Set RLR value
    IWDG->KR = 0xAAAA; // Reload IWDG
    IWDG->KR = 0xCCCC; // Start IWDG

    service_watchdog();
  }


  void System::service_watchdog() {
    IWDG->KR = 0xAAAA;
  }


  void Program::main() {
    hw::debug_blue_led.write(1);

    System::init_watchdog(ShortTimeSpan::from_millis(15000));

    // This line is important -- it implicitly inits debug_ser
    Debug::trace("Amanita Arcade 2018 initializing");
    // implicitly init uptime()
    System::uptime();

    hw::debug_red_led.write(1);

    System::init_nv();

    // Give external hardware time to wake up... some of it is sloooow
    wait_us(500000UL);
    System::service_watchdog();

    hw::debug_amber_led.write(1);
    hw::debug_red_led.write(0);

    Input::init();

    hw::debug_green_led.write(1);
    hw::debug_amber_led.write(0);

    Lights::init();
    Game::init();

    Debug::trace("Beginning main loop");
    hw::debug_green_led.write(0);

    TimeSpan next_frame_time = System::uptime();
    TimeSpan last_frame_time = next_frame_time -
      TimeSpan::from_micros(AA_FRAME_MICROS);
    uint32_t delta = AA_FRAME_MICROS;

    for(;;) {
      System::service_watchdog();

      TimeSpan now;
      do {
        now = System::uptime();
      } while(now - next_frame_time < TimeSpan::zero);

      TimeSpan frame_start = now;

      LogContext c("frame");
      hw::debug_frame_sync.write(1);

      ShortTimeSpan dt = ShortTimeSpan::from_micros(delta);

      // Frame timing is somewhat subtle: Input::read_buttons() depends on the
      // longest wait between interrupt service being no more than one full
      // serial character at 115.2kbps, or ~87us. This means we can't disable
      // interrupts for very long at all.
      // Input::read_buttons() sends a request to poll the controller, and it
      // is guaranteed to respond within ~10ms. The request consists of a
      // single character, and the response varies but is not more than
      // 60. That means worst case we will need approximately 16ms to
      // complete the entire round-trip -- after that, serial communications
      // from the controller should be silent.
      Lights::output();
      Input::read_buttons(dt);
      Game::update(dt);
      Lights::update(dt);
      hw::debug_frame_sync.write(0);

      disconnected_min_pulse_timeout.update(dt);
      if(!Input::connected()) {
        hw::debug_amber_led.write(1);
        if(was_input_connected) {
          disconnected_min_pulse_timeout = Timer(TimeSpan::from_millis(200), false);
        }
        was_input_connected = false;
      }
      else {
        if(disconnected_min_pulse_timeout.is_done()) {
          hw::debug_amber_led.write(0);
        }
        was_input_connected = true;
      }

      heartbeat_cycle.update(dt);
      hw::debug_blue_led.write(
        heartbeat_cycle.get_time() > heartbeat_cycle.get_time_remaining());

      now = System::uptime();
      uint32_t frame_us = (now - frame_start).to_micros();
      if(frame_us > AA_FRAME_MICROS - 2000) {
        Debug::tracef("Frame time of %luus exceeds budget (%luus)", frame_us,
          AA_FRAME_MICROS - 2000);
      }
      now = System::uptime();
      last_frame_time = frame_start;
      next_frame_time += TimeSpan::from_micros(AA_FRAME_MICROS);
      if(next_frame_time - now < TimeSpan::zero) {
        next_frame_time = now;
      }
    }
  }
}


int main() {
  aa::Program::main();
  return 0;
}
