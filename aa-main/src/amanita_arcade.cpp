#include "amanita_arcade.h"

#include "aa_game.h"
#include "aa_input.h"
#include "aa_lights.h"
#include "aa_time_span.h"
#include "aa_timer.h"


#define AA_MAX_INDENT 16
// AA_FRAME_MICROS must remain >= Lights::update() (currently 6000) + 16000.
// See comments regarding frame timings in the main() loop! 40fps gives us 3ms
// of headroom.
#define AA_FRAME_MICROS 25000


namespace aa {
  class Program {
  public:
    static void main();
  };


  namespace {
    static char const indent_chars[AA_MAX_INDENT] = {
      ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
      ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    };

    static char trace_buf[256];

    static aa::Timer disconnected_min_pulse_timeout(TimeSpan::from_millis(0), false);
    static aa::Timer heartbeat_cycle(TimeSpan::from_millis(1000));
    static bool was_input_connected;
 
    /*
    static __attribute__((section(".text"),aligned(4096)))
      uint8_t nv_data[4096] = { 0xFF, 0xFF, 0xFF, 0xFF, };
    */
  }


  namespace hw {
    __attribute__((aligned(4))) uint8_t debug_ser_alloc[sizeof (Serial)];
    Serial & debug_ser = *(Serial *)debug_ser_alloc; // PA_2, PA_3: USART2
    Serial input_ser(PB_10, PB_11); // USART3 -- USART1 doesn't work?
    PortOut lights_ws2812_port(PortE, 0xFFFF);
    DigitalOut debug_amber_led(LED3);
    DigitalOut debug_green_led(LED4);
    DigitalOut debug_red_led(LED5);
    DigitalOut debug_blue_led(LED6);
    DigitalOut debug_frame_sync(PD_0);
    DigitalOut debug_lights_sync(PD_1);
    /*
    I2C eeprom_i2c(PC_9, PA_8);
    DigitalOut eeprom_i2c_vcc(PC_9);
    DigitalOut eeprom_i2c_wp(PC_7);
    */
    DigitalOut eeprom_vcc(PC_6, 0);
    DigitalOut eeprom_wp(PC_7, 0);
    DigitalInOut eeprom_scl(PC_8, PIN_INPUT, OpenDrainPullUp, 0);
    DigitalInOut eeprom_sda(PC_9, PIN_INPUT, OpenDrainPullUp, 0);

    static const uint32_t eeprom_i2c_settle_us = 5;
    static const uint32_t eeprom_i2c_reset_power_off_us = 1200; // 1ms min
    static const uint32_t eeprom_i2c_reset_power_on_us = 200; // 100us min

    static bool eeprom_in_transaction = false;

    static void debug_ser_init() {
      static bool initialized = false;
      if(!initialized) {
        initialized = true;
        new(&debug_ser) Serial(PA_2, PA_3, 115200);
        debug_ser.puts("!DEBUG OUTPUT BEGIN\r\n");
      }
    }

    static void eeprom_i2c_reset()
    {
      eeprom_sda.write(0);
      eeprom_sda.input();
      eeprom_sda.mode(PullNone);
      eeprom_scl.write(0);
      eeprom_scl.input();
      eeprom_scl.mode(PullNone);
      eeprom_wp.write(0);
      eeprom_vcc.write(0);
      wait_us(eeprom_i2c_reset_power_off_us);

      eeprom_vcc.write(1);
      eeprom_wp.write(0);
      eeprom_sda.input();
      eeprom_sda.mode(PullUp);
      eeprom_scl.input();
      eeprom_scl.mode(PullUp);
      wait_us(eeprom_i2c_reset_power_on_us);
    }

    static bool eeprom_i2c_clear_bus()
    {
      eeprom_sda.input();
      eeprom_scl.input();
      if(eeprom_scl.read() == 0) {
        Debug::trace("EEPROM SCL stuck low, hardware fault!");
        return false;
      }

      for(int i = 0; i < 20; ++i) {
        if(eeprom_sda.read() != 0) {
          // Bus released!
          return true;
        }
        eeprom_scl.output();
        wait_us(eeprom_i2c_settle_us);
        eeprom_scl.input();
        wait_us(eeprom_i2c_settle_us);
      }

      Debug::trace("EEPROM SDA stuck low, bus stuck!");
      return false;
    }

    static bool eeprom_i2c_bus_error() {
      eeprom_scl.input();
      eeprom_sda.input();
      eeprom_in_transaction = false;
      return false;
    }

    static bool eeprom_i2c_start() {
      if(!eeprom_in_transaction) {
        if((eeprom_scl.read() == 0) || (eeprom_sda.read() == 0)) {
          return eeprom_i2c_bus_error();
        }
        eeprom_sda.output();
        wait_us(eeprom_i2c_settle_us);
        if(eeprom_sda.read() != 0) {
          return eeprom_i2c_bus_error();
        }
        eeprom_scl.output();
        wait_us(eeprom_i2c_settle_us);
        if(eeprom_scl.read() != 0) {
          return eeprom_i2c_bus_error();
        }

        eeprom_in_transaction = true;
      }
      else {
        if((eeprom_scl.read() != 0) || (eeprom_sda.read() != 0)) {
          return eeprom_i2c_bus_error();
        }

        eeprom_sda.input();
        wait_us(eeprom_i2c_settle_us);
        if(eeprom_sda.read() == 0) {
          return eeprom_i2c_bus_error();
        }
        eeprom_scl.input();
        wait_us(eeprom_i2c_settle_us);
        if(eeprom_scl.read() == 0) {
          return eeprom_i2c_bus_error();
        }
        eeprom_sda.output();
        wait_us(eeprom_i2c_settle_us);
        if(eeprom_sda.read() != 0) {
          return eeprom_i2c_bus_error();
        }
        eeprom_scl.output();
        wait_us(eeprom_i2c_settle_us);
        if(eeprom_scl.read() != 0) {
          return eeprom_i2c_bus_error();
        }
      }

      return true;
    }

    static bool eeprom_i2c_stop() {
      if(!eeprom_in_transaction) {
        return true;
      }

      eeprom_in_transaction = false;

      if((eeprom_scl.read() != 0) || (eeprom_sda.read() != 0)) {
        return eeprom_i2c_bus_error();
      }

      eeprom_scl.input();
      wait_us(eeprom_i2c_settle_us);
      if(eeprom_scl.read() == 0) {
        return eeprom_i2c_bus_error();
      }
      eeprom_sda.input();
      wait_us(eeprom_i2c_settle_us);
      if(eeprom_sda.read() == 0) {
        return eeprom_i2c_bus_error();
      }

      return true;
    }

    static bool eeprom_i2c_read(uint8_t * val, bool ack) {
      *val = 0;

      if(!eeprom_in_transaction) {
        // In a transaction SCL would already be low, and slave data stable,
        // but if we are not in a transaction clock will be high -- bring it
        // low and wait for the lines to settle
        eeprom_scl.output();
        wait_us(eeprom_i2c_settle_us);
        if(eeprom_scl.read() != 0) {
          return eeprom_i2c_bus_error();
        }
      }

      // Prepare to receive input -- slave should be controlling SDA already
      eeprom_sda.input();
      wait_us(eeprom_i2c_settle_us);

      // Clock high, transfer data (first bit)
      eeprom_scl.input();
      uint32_t val_temp = eeprom_sda.read() != 0 ? 1 : 0;
      wait_us(eeprom_i2c_settle_us);
      if(eeprom_scl.read() == 0) {
        return eeprom_i2c_bus_error();
      }

      for(int i = 1; i < 8; ++i) {
        val_temp = val_temp << 1;

        // Clock low, slave sets up data
        eeprom_scl.output();
        wait_us(eeprom_i2c_settle_us);
        if(eeprom_scl.read() != 0) {
          return eeprom_i2c_bus_error();
        }
        wait_us(eeprom_i2c_settle_us);

        // Clock high, transfer data
        eeprom_scl.input();
        if(eeprom_sda.read() != 0) {
          val_temp |= 1;
        }
        wait_us(eeprom_i2c_settle_us);
        if(eeprom_scl.read() == 0) {
          return eeprom_i2c_bus_error();
        }
        wait_us(eeprom_i2c_settle_us);
      }


      // Clock low, set up ack
      eeprom_scl.output();
      wait_us(eeprom_i2c_settle_us);
      if(eeprom_scl.read() != 0) {
        return eeprom_i2c_bus_error();
      }
      if(ack) {
        eeprom_sda.output();
      }
      wait_us(eeprom_i2c_settle_us);
      bool read_ack = (eeprom_sda.read() == 0);
      if(read_ack != ack) {
        return eeprom_i2c_bus_error();
      }

      // Clock high, transfer data
      eeprom_scl.input();
      wait_us(eeprom_i2c_settle_us);
      if(eeprom_scl.read() == 0) {
        return eeprom_i2c_bus_error();
      }
      wait_us(eeprom_i2c_settle_us);

      if(eeprom_in_transaction) {
        eeprom_scl.output();
        wait_us(eeprom_i2c_settle_us);
        eeprom_sda.output();
        wait_us(eeprom_i2c_settle_us);
      }

      *val = val_temp & 0xFF;
      return true;
    }

    static bool eeprom_i2c_write(uint8_t val, bool * ack) {
      if(!eeprom_in_transaction) {
        // In a transaction SCL would already be low, and slave data stable,
        // but if we are not in a transaction clock will be high -- bring it
        // low and wait for the lines to settle
        eeprom_scl.output();
        wait_us(eeprom_i2c_settle_us);
        if(eeprom_scl.read() != 0) {
          return eeprom_i2c_bus_error();
        }
      }

      uint32_t val_temp = val;

      for(int i = 0; i < 8; ++i) {
        // Clock low, set up data
        if((val_temp & 0x80) != 0) {
          eeprom_sda.input();
        }
        else {
          eeprom_sda.output();
        }
        wait_us(eeprom_i2c_settle_us);
        if((eeprom_sda.read() != 0) != ((val_temp & 0x80) != 0)) {
          return eeprom_i2c_bus_error();
        }

        val_temp = val_temp << 1;

        // Clock high, transfer data
        eeprom_scl.input();
        wait_us(eeprom_i2c_settle_us);
        if(eeprom_scl.read() == 0) {
          return eeprom_i2c_bus_error();
        }
        wait_us(eeprom_i2c_settle_us);

        // Set clock low
        eeprom_scl.output();
        wait_us(eeprom_i2c_settle_us);
        if(eeprom_scl.read() != 0) {
          return eeprom_i2c_bus_error();
        }
      }

      // Clock high, transfer ack
      eeprom_scl.input();
      bool ack_temp = (eeprom_sda.read() == 0);
      wait_us(eeprom_i2c_settle_us);
      if(eeprom_scl.read() == 0) {
        return eeprom_i2c_bus_error();
      }
      wait_us(eeprom_i2c_settle_us);

      if(eeprom_in_transaction) {
        // Leave SCL low to hold bus for ourselves
        eeprom_scl.output();
        wait_us(eeprom_i2c_settle_us);
        if(eeprom_scl.read() != 0) {
          return eeprom_i2c_bus_error();
        }
        eeprom_sda.output();
        wait_us(eeprom_i2c_settle_us);
      }

      *ack = ack_temp;
      return true;
    }
  }


  int32_t Debug::_indent_depth;


  void Debug::pause() {
    // TODO debug breakpoint
    //__BKPT(0);
    trace("paused, any input to resume");
    for(;;) {
      System::service_watchdog();
      if(hw::debug_ser.readable()) {
        hw::debug_ser.getc();
        break;
      }
    }
  }


  void Debug::abort() {
    // TODO reboot
    trace("aborting execution");
    for(;;) {
      // wait forever -- watchdog will reboot us
    }
  }

  void Debug::assert(bool expr, char const * fail_message) {
    if(!expr) {
      error(fail_message);
    }
  }


  void Debug::assertf(bool expr, char const * fail_format, ...) {
    if(!expr) {
      va_list va;
      va_start(va, fail_format);
      verrorf(fail_format, va);
      va_end(va);
    }
  }


  void Debug::vassertf(bool expr, char const * fail_format, va_list va) {
    if(!expr) {
      verrorf(fail_format, va);
    }
  }


  void Debug::trace(char const * message) {
    hw::debug_ser_init();

    char const * q = message;
    char const * p;
    for(;;) {
      fwrite(indent_chars,
        _indent_depth < AA_MAX_INDENT ? _indent_depth : AA_MAX_INDENT, 1,
        stdout);
      p = q;
      if(p == message) {
        putchar('>');
      }
      else {
        putchar(':');
      }
      while(*q != 0 && *q != '\n') {
        ++q;
      }
      fwrite(p, q - p, 1, stdout);
      fputs("\r\n", stdout);
      if(*q == 0) {
        break;
      }
      ++q;
    }
  }


  void Debug::tracef(char const * format, ...) {
    va_list va;
    va_start(va, format);
    vtracef(format, va);
    va_end(va);
  }


  void Debug::vtracef(char const * format, va_list va) {
    vsnprintf(trace_buf, sizeof trace_buf, format, va);
    trace(trace_buf);
  }


  void Debug::error(char const * message) {
    trace(message);
    abort();
  }


  void Debug::errorf(char const * format, ...) {
    va_list va;
    va_start(va, format);
    verrorf(format, va);
    va_end(va);
  }


  void Debug::verrorf(char const * format, va_list va) {
    vtracef(format, va);
    abort();
  }


  void Debug::push_context(char const * name) {
    /*
    fwrite(indent_chars,
      _indent_depth < AA_MAX_INDENT ? _indent_depth : AA_MAX_INDENT, 1,
      stdout);
    putchar('[');
    fputs(name, stdout);
    putchar("]\r\n");
    */
    assertf(AA_AUTO_ASSERT(_indent_depth < AA_MAX_INDENT));
    ++_indent_depth;
  }


  void Debug::pop_context() {
    assertf(AA_AUTO_ASSERT(_indent_depth > 0));
    if(_indent_depth > 0) {
      --_indent_depth;
    }
  }


  bool Debug::in_available() {
    hw::debug_ser_init();

    return hw::debug_ser.readable();
  }


  int Debug::in_read_nb() {
    hw::debug_ser_init();

    if(hw::debug_ser.readable()) {
      return hw::debug_ser.getc();
    }
    else {
      return -1;
    }
  }


  char Debug::in_read() {
    hw::debug_ser_init();

    return hw::debug_ser.getc();
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
    hw::eeprom_i2c_reset();
    if(!hw::eeprom_i2c_clear_bus()) {
      Debug::trace("Bus locked, aborting");
      return;
    }
    Debug::trace("Reset complete, bus clear");

    uint16_t mem_addr = 0x000;
    uint8_t n_wr = 0;
    uint8_t addr0 = 0b10100000 | (((mem_addr >> 8) & 0b11) << 1) | n_wr;
    uint8_t addr1 = (mem_addr >> 0) & 0xFF;
    bool ack;

    if(!hw::eeprom_i2c_start()) {
      Debug::trace("EEPROM I2C start bus fault");
    }

    if(!hw::eeprom_i2c_write(addr0, &ack)) {
      Debug::trace("EEPROM I2C write device address write bus fault");
      return;
    }
    if(!ack) {
      Debug::trace("EEPROM I2C write device address write NAK");
      return;
    }

    if(!hw::eeprom_i2c_write(addr1, &ack)) {
      Debug::trace("EEPROM I2C write mem address write bus fault");
      return;
    }
    if(!ack) {
      Debug::trace("EEPROM I2C write mem address NAK");
      return;
    }

    if(!hw::eeprom_i2c_write(0x23, &ack)) {
      Debug::trace("EEPROM I2C write data bus fault");
      return;
    }
    if(!ack) {
      Debug::trace("EEPROM I2C write data NAK");
      return;
    }

    if(!hw::eeprom_i2c_stop()) {
      Debug::trace("EEPROM I2C write stop bus fault");
    }

    wait_ms(5);

    if(!hw::eeprom_i2c_start()) {
      Debug::trace("EEPROM I2C read start bus fault");
    }

    if(!hw::eeprom_i2c_write(addr0, &ack)) {
      Debug::trace("EEPROM I2C read device address write bus fault");
      return;
    }
    if(!ack) {
      Debug::trace("EEPROM I2C read device address write NAK");
      return;
    }

    if(!hw::eeprom_i2c_write(addr1, &ack)) {
      Debug::trace("EEPROM I2C read mem address write bus fault");
      return;
    }
    if(!ack) {
      Debug::trace("EEPROM I2C read mem address NAK");
      return;
    }

    if(!hw::eeprom_i2c_start()) {
      Debug::trace("EEPROM I2C read restart bus fault");
    }

    if(!hw::eeprom_i2c_write(0b10100001, &ack)) {
      Debug::trace("EEPROM I2C read device address read bus fault");
      return;
    }
    if(!ack) {
      Debug::trace("EEPROM I2C read device address read NAK");
      return;
    }

    uint8_t val = 0;
    if(!hw::eeprom_i2c_read(&val, false)) {
      Debug::tracef("EEPROM I2C read data bus fault");
      return;
    }
    Debug::tracef("EEPROM read 0: 0x%02X", val);

    if(!hw::eeprom_i2c_stop()) {
      Debug::trace("EEPROM I2C read stop bus fault");
    }
  }


  void System::write_nv(uint32_t id, void const * data, size_t size) {
  }


  void const * System::read_nv(uint32_t id, size_t * size) {
    return nullptr;
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
    wait_ms(500);
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

      // Frame timing is somewhat subtle: Lights::output() disables
      // interrupts for a long period (~5ms as of this writing), while it is
      // engaged in bit-banging that is timing sensitive at the 100ns level;
      // however, Input::read_buttons() depends on the longest wait between
      // interrupt service being no more than one full serial character at
      // 115.2kbps, or ~87us.
      // These two timing contraints are obviously incompatible in the
      // general case! However, we can get away with fudging this, knowing a
      // little about the particular controller hooked up to the Input
      // serial input.
      // Input::read_buttons() sends a request to poll the controller, and it
      // is guaranteed to respond within ~10ms. The request consists of a
      // single character, and the response varies but is not more than
      // 60. That means worst case we will need approximately 16ms to
      // complete the entire round-trip -- after that, serial communications
      // from the controller should be silent, and it won't matter if
      // interrupts are disabled because none should be raised!
      //
      // So the general strategy here is to put Input::read_buttons()
      // immediately after Lights::output() to give it maximum clearance
      // before the next Lights::output(). As long as AA_FRAME_MICROS >=
      // Lights::update time + 16000, i.e. ~22000, we should be fine.
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
