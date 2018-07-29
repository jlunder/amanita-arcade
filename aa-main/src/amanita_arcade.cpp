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
  }


  namespace hw {
    Serial * debug_ser = nullptr; // (PA_2, PA_3); // USART2 -- also accessible via stdio?
    Serial input_ser(PB_10, PB_11); // USART3 -- USART1 doesn't work?
    PortOut lights_ws2812_port(PortE, 0xFFFF);
    DigitalOut debug_amber_led(LED3);
    DigitalOut debug_green_led(LED4);
    DigitalOut debug_red_led(LED5);
    DigitalOut debug_blue_led(LED6);
    DigitalOut debug_frame_sync(PD_0);
    DigitalOut debug_lights_sync(PD_1);
  }


  int32_t Debug::_indent_depth;


  void Debug::pause() {
    // TODO debug breakpoint
    //__BKPT(0);
    trace("paused, any input to resume");
    getchar();
  }


  void Debug::abort() {
    // TODO reboot
    trace("aborting execution");
    for(;;) {
      // wait forever
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
    static Serial debug_ser_actual(PA_2, PA_3); // USART2 -- also accessible via stdio

    if(hw::debug_ser == nullptr) {
      debug_ser_actual.baud(115200);
      hw::debug_ser = &debug_ser_actual;
      debug_ser_actual.puts("!DEBUG OUTPUT BEGIN\r\n");
    }

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
    FLASH_PageErase(EEPROM_START_ADDRESS); // required to re-write
    CLEAR_BIT(FLASH->CR, FLASH_CR_PER); // Bug fix: bit PER has been set in Flash_PageErase(), must clear it here
    //...
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
    /*
    // see http://embedded-lab.com/blog/?p=9662
    static uint64_t const lsi_freq = 45000;

    uint64_t timeout_micros = timeout.to_micros();
    uint16_t prescaler_code;
    uint16_t prescaler;
    uint16_t reload_value;
    uint32_t calculated_timeout_micros;

    if ((timeout_micros * (LsiFreq/4)) < 0x7FF * 1000000) {
      prescaler_code = IWDG_PRESCALER_4;
      prescaler = 4;
    }
    else if ((timeout_micros * (LsiFreq/8)) < 0xFF0 * 1000000) {
      prescaler_code = IWDG_PRESCALER_8;
      prescaler = 8;
    }
    else if ((timeout_micros * (LsiFreq/16)) < 0xFF0 * 1000000) {
      prescaler_code = IWDG_PRESCALER_16;
      prescaler = 16;
    }
    else if ((timeout_micros * (LsiFreq/32)) < 0xFF0 * 1000000) {
      prescaler_code = IWDG_PRESCALER_32;
      prescaler = 32;
    }
    else if ((timeout_micros * (LsiFreq/64)) < 0xFF0 * 1000000) {
      prescaler_code = IWDG_PRESCALER_64;
      prescaler = 64;
    }
    else if ((timeout_micros * (LsiFreq/128)) < 0xFF0 * 1000000) {
      prescaler_code = IWDG_PRESCALER_128;
      prescaler = 128;
    }
    else {
      prescaler_code = IWDG_PRESCALER_256;
      prescaler = 256;
    }

    // specifies the IWDG Reload value. This parameter must be a number between 0 and 0x0FFF.
    reload_value =
      (uint32_t)((timeout_micros * (lsi_freq / prescaler) + 500000) / 1000000);

    Calculated_timeout = ((float)(prescaler * reload_value)) / lsi_freq;
    Debug::tracef("Set WDT to %dx%d from desired timeout %dus; actual %dus",
      prescaler, reload_value, calculated_timeout_micros);

    IWDG->KR = 0x5555; //Disable write protection of IWDG registers
    IWDG->PR = prescaler_code;      //Set PR value
    IWDG->RLR = reload_value;      //Set RLR value
    IWDG->KR = 0xAAAA;    //Reload IWDG
    IWDG->KR = 0xCCCC;    //Start IWDG - See more at: http://embedded-lab.com/blog/?p=9662#sthash.6VNxVSn0.dpuf

    service_watchdog();
    */
  }


  void System::service_watchdog() {
    //IWDG->KR = 0xAAAA;         //Reload IWDG - See more at: http://embedded-lab.com/blog/?p=9662#sthash.6VNxVSn0.dpuf
  }


  void Program::main() {
    System::start_watchdog(ShortTimeSpan::from_millis(10000));

    // This line is important -- it implicitly inits debug_ser
    Debug::trace("Amanita Arcade 2018 initializing");
    // implicitly init uptime()
    System::uptime();

    // Give external hardware time to wake up... some of it is sloooow
    wait_ms(500);

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

      // Frame timing is somewhat subtle: Lights::update() disables
      // interrupts for a long period (~5ms as of this writing), while it is
      // engaged in bit-banging that is timing sensitive at the 100ns level;
      // however, Input::read_buttons depends on the longest wait between
      // interrupt service being no more than one full serial character at
      // 115.2kbps, or ~87us.
      // These two timing contraints are obviously incompatible in the
      // general case! However, we can get away with fudging this, knowing a
      // little about the particular controller hooked up to the Input
      // serial input.
      // Input::read_buttons sends a request to poll the controller, and it
      // is guaranteed to respond within ~10ms. The request consists of a
      // single character, and the response varies but is not more than
      // 60. That means worst case we will need approximately 16ms to
      // complete the entire round-trip -- after that, serial communications
      // from the controller should be silent, and it won't matter if
      // interrupts are disabled because none should be raised!
      //
      // So the general strategy here is to put Input::read_buttons()
      // immediately after Lights::update() to give it maximum clearance
      // before the next Lights::update(). As long as AA_FRAME_MICROS >=
      // Lights::update time + 16000, i.e. ~22000, we should be fine.
      Lights::output();
      Input::read_buttons();
      Game::update(ShortTimeSpan(delta));
      Lights::update(ShortTimeSpan(delta));
      hw::debug_frame_sync = 0;

      now = System::uptime();
      uint32_t frame_us = (now - frame_start).to_micros();
      if(frame_us > AA_FRAME_MICROS - 2000) {
        Debug::tracef("Frame time of %luus exceeds budget (%luus) -> %lldus, %lldus", frame_us,
          AA_FRAME_MICROS - 2000, now.to_micros(), frame_start.to_micros());
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
