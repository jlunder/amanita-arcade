#include "amanita_arcade.h"

#include "aa_game.h"
#include "aa_input.h"
#include "aa_lights.h"
#include "aa_time_span.h"


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

    static void debug_ser_init() {
      static bool initialized = false;
      if(!initialized) {
        initialized = true;
        new(&debug_ser) Serial(PA_2, PA_3, 115200);
        debug_ser.puts("!DEBUG OUTPUT BEGIN\r\n");
      }
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
      Debug::tracef("Starting timer for the first time");
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


  void System::write_nv(uint32_t id, void const * data, size_t size) {
    /*
    HAL_StatusTypeDef status = HAL_FLASH_Unlock();
    //FLASH_Erase_Sector((uint32_t)nv_data, FLASH_VOLTAGE_RANGE_3);
    //FLASH->CR &= ~FLASH_CR_PER; // Bug fix: bit PER has been set in Flash_PageErase(), must clear it here
    HAL_FLASH_Lock();
    */
  }


  void const * System::read_nv(uint32_t id, size_t * size) {
    /*
    HAL_StatusTypeDef status;
    address = address + EEPROM_START_ADDRESS;

    status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, data);

    return status;
    */
    return nullptr;
  }


  void System::start_watchdog(ShortTimeSpan timeout) {
    // see http://embedded-lab.com/blog/?p=9662
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
    System::start_watchdog(ShortTimeSpan::from_millis(15000));

    // This line is important -- it implicitly inits debug_ser
    Debug::trace("Amanita Arcade 2018 initializing");
    // implicitly init uptime()
    System::uptime();

    // Give external hardware time to wake up... some of it is sloooow
    wait_ms(500);
    System::service_watchdog();

    Input::init();
    Lights::init();
    Game::init();

    Debug::trace("Beginning main loop");

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
      hw::debug_frame_sync = 1;

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
      Input::read_buttons(ShortTimeSpan::from_micros(delta));
      Game::update(ShortTimeSpan::from_micros(delta));
      Lights::update(ShortTimeSpan::from_micros(delta));
      hw::debug_frame_sync = 0;

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
