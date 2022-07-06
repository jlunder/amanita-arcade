#include "aa_input_controller.h"
#include "PSGamepad.h"


// 100Hz PWM period
#define PWM_PERIOD_US 10000
#define PWM_BUTTON_OFF_PULSE_WIDTH_US 5000
#define PWM_BUTTON_ON_PULSE_WIDTH_US 10000
#define POLL_PERIOD_US 10000
#define LOOP_TIMING_EPSILON_US 50
#define WATCHDOG_TIMEOUT_US 50000


PwmOut light_red(PB_1);
PwmOut light_green(PB_0);
PwmOut light_blue(PA_7);
PwmOut light_pink(PA_6);

DigitalOut debug_led(PC_13);

DigitalIn button_red(PB_6);
DigitalIn button_green(PB_7);
DigitalIn button_blue(PB_8);
DigitalIn button_pink(PB_9);

PSGamepad psg(PB_15, PB_14, PB_13, PB_12);
UARTSerial serial_comms(PA_9, PA_10, 115200);


enum ParserState {
  PS_IDLE,
  PS_AUTO_PARAM,
  PS_MODE_PARAM,
  PS_RUMBLE_PARAM_0,
  PS_RUMBLE_PARAM_1,
  PS_RUMBLE_PARAM_2,
};


us_timestamp_t last_poll_us;
us_timestamp_t last_loop_start_us;
us_timestamp_t last_loop_work_us;
us_timestamp_t last_read_work_us;

ParserState parser_state = PS_IDLE;
bool auto_poll = false;
bool use_analog = true;
bool use_pressure = false;
bool rumble_motor_0_staging = false;
bool rumble_motor_0;
uint8_t rumble_motor_1_staging = 0;
uint8_t rumble_motor_1;
uint16_t extra_buttons;

bool do_print;
bool do_reset;


void init_watchdog(uint64_t timeout_micros);
void service_watchdog();
void do_main_loop();
void pollGamepad();
void executeCommands();
void parseCommand(uint8_t command);
void parseIdleCommand(uint8_t command);
void parseModeParam(uint8_t command);
uint8_t parseHexDigit(uint8_t digit);
void printGamepadValues();


namespace mbed
{
    FileHandle *mbed_target_override_console(int) {
      // PB_10, PB_11: USART3
      static UARTSerial debug_out(PB_10, PB_11, 115200);
      static bool configured = false;
      if(!configured) {
        configured = true;
        debug_out.set_blocking(true);
      }
      return &debug_out;
    }
}


void logp(char const * fmt, ...) {
  char buf[200];

  va_list va;
  va_start(va, fmt);
  int len = vsnprintf(buf, sizeof buf, fmt, va);
  if(len > 0) {
    mbed::mbed_target_override_console(1)->write(buf, len);
  }
  va_end(va);
}



int main() {
  debug_led.write(0);
  logp("Amanita Arcade 2019 input controller initializing\r\n");
  wait_us(500000);

  serial_comms.set_blocking(true);

  light_red.period_us(PWM_PERIOD_US);
  light_green.period_us(PWM_PERIOD_US);
  light_blue.period_us(PWM_PERIOD_US);
  light_pink.period_us(PWM_PERIOD_US);

  light_red.pulsewidth_us(PWM_BUTTON_OFF_PULSE_WIDTH_US);
  light_green.pulsewidth_us(PWM_BUTTON_OFF_PULSE_WIDTH_US);
  light_blue.pulsewidth_us(PWM_BUTTON_OFF_PULSE_WIDTH_US);
  light_pink.pulsewidth_us(PWM_BUTTON_OFF_PULSE_WIDTH_US);

  psg.begin(use_analog, use_pressure);

  init_watchdog(WATCHDOG_TIMEOUT_US);

  debug_led.write(1);

  do_main_loop();
}


void init_watchdog(uint64_t timeout_micros) {
  static uint64_t const lsi_freq = 32768;

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


void service_watchdog() {
  IWDG->KR = 0xAAAA;
}


void do_main_loop() {
  printf("Beginning main loop\r\n");

  Timer loop_timer;
  us_timestamp_t loop_start_us;
  us_timestamp_t time_since_last_poll_us;

  loop_timer.start();
  loop_start_us = loop_timer.read_high_resolution_us();
  last_loop_start_us = loop_start_us - POLL_PERIOD_US + 200;

  for(;;) {
    service_watchdog();
    
    do {
      loop_start_us = loop_timer.read_high_resolution_us();
      time_since_last_poll_us = (loop_start_us - last_loop_start_us);
      us_timestamp_t wait_left_us = POLL_PERIOD_US - time_since_last_poll_us;

      if(wait_left_us > POLL_PERIOD_US) {
        // excessive wait time (negative would roll over) means we passed our
        // deadline
        break;
      }

      // otherwise just busy-wait
    } while(time_since_last_poll_us < POLL_PERIOD_US);

    last_loop_start_us += POLL_PERIOD_US;
    int32_t loop_start_error = (int32_t)(loop_timer.read_high_resolution_us() - last_loop_start_us);
    if(abs(loop_start_error) > LOOP_TIMING_EPSILON_US) {
      logp("Excessive drift in loop start time of %luus\r\n",
        (unsigned long)loop_start_error);
      if(loop_start_error >= POLL_PERIOD_US / 2) {
        // Catch up, we're falling behind
        last_loop_start_us = loop_start_us;
      }
    }

    debug_led.write(0);

    pollGamepad();
    executeCommands();

    debug_led.write(1);
    last_loop_work_us = loop_timer.read_us() - loop_start_us;
  }
}


void pollGamepad() {
  Timer read_work_timer;
  read_work_timer.start();
  psg.poll(rumble_motor_0, rumble_motor_1);
  last_read_work_us = read_work_timer.read_high_resolution_us();

  extra_buttons = 0;
  if(!button_red.read()) {
    extra_buttons |= PSB_RED;
    light_red.pulsewidth_us(PWM_BUTTON_ON_PULSE_WIDTH_US);
  } else {
    light_red.pulsewidth_us(PWM_BUTTON_OFF_PULSE_WIDTH_US);
  }
  if(!button_green.read()) {
    extra_buttons |= PSB_GREEN;
    light_green.pulsewidth_us(PWM_BUTTON_ON_PULSE_WIDTH_US);
  } else {
    light_green.pulsewidth_us(PWM_BUTTON_OFF_PULSE_WIDTH_US);
  }
  if(!button_blue.read()) {
    extra_buttons |= PSB_BLUE;
    light_blue.pulsewidth_us(PWM_BUTTON_ON_PULSE_WIDTH_US);
  } else {
    light_blue.pulsewidth_us(PWM_BUTTON_OFF_PULSE_WIDTH_US);
  }
  if(!button_pink.read()) {
    extra_buttons |= PSB_PINK;
    light_pink.pulsewidth_us(PWM_BUTTON_ON_PULSE_WIDTH_US);
  } else {
    light_pink.pulsewidth_us(PWM_BUTTON_OFF_PULSE_WIDTH_US);
  }
}


void executeCommands() {
  while(serial_comms.readable()) {
    uint8_t c;
    serial_comms.read(&c, 1);
    parseCommand(c);
    if(do_reset) {
      logp("Reset\r\n");
      auto_poll = false;
      rumble_motor_0 = false;
      rumble_motor_1 = 0;
      psg.end();
      psg.begin(use_analog, use_pressure, true);
      do_reset = false;
    }
  }

  if(do_print || auto_poll) {
    printGamepadValues();
  }
  do_print = false;
  //log_x(psg.getButtons() & ~extra_buttons, 4);
  //logp("\r\n");
}


void parseCommand(uint8_t command) {
  switch(parser_state) {
    case PS_IDLE:
      parseIdleCommand(command);
      break;
    case PS_AUTO_PARAM:
      auto_poll = (command == '1');
      parser_state = PS_IDLE;
      break;
    case PS_MODE_PARAM:
      parseModeParam(command);
      parser_state = PS_IDLE;
      break;
    case PS_RUMBLE_PARAM_0:
      if(command == '\n') {
        parser_state = PS_IDLE;
      } else {
        rumble_motor_0_staging = (command == '1');
        parser_state = PS_RUMBLE_PARAM_1;
      }
      break;
    case PS_RUMBLE_PARAM_1:
      if(command == '\n') {
        parser_state = PS_IDLE;
      } else {
        rumble_motor_1_staging |= parseHexDigit(command) << 4;
        parser_state = PS_RUMBLE_PARAM_2;
      }
      break;
    case PS_RUMBLE_PARAM_2:
      if(command == '\n') {
        parser_state = PS_IDLE;
      } else {
        rumble_motor_1_staging |= parseHexDigit(command) << 0;
        rumble_motor_0 = rumble_motor_0_staging;
        rumble_motor_1 = rumble_motor_1_staging;
        parser_state = PS_IDLE;
      }
      break;
    default:
      parser_state = PS_IDLE;
      break;
  }
}


void parseIdleCommand(uint8_t command) {
  switch(command) {
    case 'a': case 'A':
      logp("Got 'A'\r\n");
      parser_state = PS_AUTO_PARAM;
      auto_poll = false;
      break;
    case 'm': case 'M':
      logp("Got 'M'\r\n");
      parser_state = PS_MODE_PARAM;
      break;
    case 'p': case 'P':
      logp("Got 'P'\r\n");
      do_print = true;
      break;
    case 'r': case 'R':
      logp("Got 'R'\r\n");
      do_reset = true;
      break;
    case 'v': case 'V':
      logp("Got 'V'\r\n");
      parser_state = PS_RUMBLE_PARAM_0;
      rumble_motor_0_staging = false;
      rumble_motor_1_staging = 0;
      break;
    default:
      logp("Bad command '%c'\r\n", (char)command);
      break;
    case '\r':
    case '\n':
      break;
  }
}


void parseModeParam(uint8_t command) {
  switch(command) {
    default:
    case '0':
      do_reset = true;
      use_analog = false;
      use_pressure = false;
      break;
    case '1':
      do_reset = true;
      use_analog = true;
      use_pressure = false;
      break;
    case '2':
      do_reset = true;
      use_analog = true;
      use_pressure = true;
      break;
  }
}


uint8_t parseHexDigit(uint8_t digit) {
  if(digit >= '0' && digit <= '9') {
    return digit - '0';
  } else if(digit >= 'A' && digit <= 'F') {
    return digit - 'A' + 10;
  } else if(digit >= 'a' && digit <= 'f') {
    return digit - 'a' + 10;
  } else {
    return 0;
  }
}


void printGamepadValues() {
  char buf[100];
  int32_t len;

  logp("State: ");
  len = snprintf(buf, sizeof buf, "P %08lX",
    (unsigned long)last_loop_start_us);
  logp("%s", buf);
  serial_comms.write(buf, len);

#if 0
  len = snprintf(buf, sizeof buf, "/%08lX,%08lX",
    (unsigned long)last_read_work_us, (unsigned long)last_loop_work_us);
  logp(buf);
  serial_comms.write(buf, len);
#endif

  uint8_t status = psg.getStatus();

  len = snprintf(buf, sizeof buf, ":%02X:%04X",
    (unsigned int)status,
    (unsigned int)(psg.getButtons() & ~extra_buttons));
  logp("%s", buf);
  serial_comms.write(buf, len);

  if(status == PSCS_ANALOG || status == PSCS_PRESSURE) {
    len = snprintf(buf, sizeof buf, "/%02X,%02X;%02X,%02X",
      (unsigned int)psg.getAnalog(PSS_LX),
      (unsigned int)psg.getAnalog(PSS_LY),
      (unsigned int)psg.getAnalog(PSS_RX),
      (unsigned int)psg.getAnalog(PSS_RY));
    logp("%s", buf);
    serial_comms.write(buf, len);
  }

  if(status == PSCS_PRESSURE) {
    len = snprintf(buf, sizeof buf,
      "/%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X,%02X",
      (unsigned int)psg.getAnalog(PSAB_PAD_RIGHT + 0),
      (unsigned int)psg.getAnalog(PSAB_PAD_RIGHT + 1),
      (unsigned int)psg.getAnalog(PSAB_PAD_RIGHT + 2),
      (unsigned int)psg.getAnalog(PSAB_PAD_RIGHT + 3),
      (unsigned int)psg.getAnalog(PSAB_PAD_RIGHT + 4),
      (unsigned int)psg.getAnalog(PSAB_PAD_RIGHT + 5),
      (unsigned int)psg.getAnalog(PSAB_PAD_RIGHT + 6),
      (unsigned int)psg.getAnalog(PSAB_PAD_RIGHT + 7),
      (unsigned int)psg.getAnalog(PSAB_PAD_RIGHT + 8),
      (unsigned int)psg.getAnalog(PSAB_PAD_RIGHT + 9),
      (unsigned int)psg.getAnalog(PSAB_PAD_RIGHT + 10),
      (unsigned int)psg.getAnalog(PSAB_PAD_RIGHT + 11));
    logp("%s", buf);
    serial_comms.write(buf, len);
  }

  logp("\r\n", buf);
  serial_comms.write("\r\n", 2);
}

