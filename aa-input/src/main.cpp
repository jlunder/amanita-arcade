#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>

#include <new>

#include <mbed.h>

#include "PSGamepad.h"


// 100Hz PWM period
#define PWM_PERIOD_US 10000
#define PWM_BUTTON_OFF_PULSE_WIDTH_US 5000
#define PWM_BUTTON_ON_PULSE_WIDTH_US 10000
#define POLL_PERIOD_US 10000
#define LOOP_TIMING_EPSILON_US 50


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
Serial serial_comms(PA_9, PA_10, 115200);


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


void do_main_loop();
void pollGamepad();
void executeCommands();
void parseCommand(uint8_t command);
void parseIdleCommand(uint8_t command);
void parseModeParam(uint8_t command);
uint8_t parseHexDigit(uint8_t digit);
void printGamepadValues();


static char const hexDigits[16] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};


static inline void print(FILE * f, char const * message) { fputs(message, f); }
static void print_d(FILE * f, int32_t val);
static void print_u(FILE * f, uint32_t val);
static void print_x(FILE * f, uint32_t val, int digits);

static inline void log(char const * message) { print(stderr, message); }
static inline void log_d(int32_t val) { print_d(stderr, val); }
static inline void log_u(uint32_t val) { print_u(stderr, val); }
static inline void log_x(uint32_t val, int digits) { print_x(stderr, val, digits); }


void print_d(FILE * f, int32_t val) {
  bool negative = val < 0;
  if(negative) {
    // If val == -0x80000000, -val == val; but the cast to uint32_t still
    // gives us the right thing
    fputc('-', f);
  }
  print_u(f, (uint32_t)(negative ? -val : val));
}


void print_u(FILE * f, uint32_t val) {
  char conversion[10];
  uint32_t conv_val = val;
  size_t i = sizeof conversion;
  conversion[--i] = 0;
  if(conv_val == 0) {
    conversion[--i] = '0';
  } else {
    while((conv_val > 0) && (i > 0)) {
      conversion[--i] = (conv_val % 10) + '0';
      conv_val /= 10;
    }
  }
  fputs(conversion + i, f);
}


void print_x(FILE * f, uint32_t val, int digits) {
  if(digits > 8) {
    digits = 8;
  }
  char conversion[9];
  size_t i = digits;
  uint32_t conv_val = val;
  conversion[i] = 0;
  while(i > 0) {
    --i;
    conversion[i] = hexDigits[conv_val & 0xF];
    conv_val = conv_val >> 4;
  }
  fputs(conversion, f);
}


namespace mbed
{
    FileHandle *mbed_target_override_console(int) {
      // PB_10, PB_11: USART3
      static Serial debug_out(PB_10, PB_11, 115200);
      static bool configured = false;
      if(!configured) {
        configured = true;
        debug_out.set_blocking(true);
      }
      return &debug_out;
    }
}


int main() {
  debug_led.write(0);
  fputs("Amanita Arcade 2019 input controller initializing\r\n", stdout);
  wait_ms(500);

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

  debug_led.write(1);

  do_main_loop();
}


void do_main_loop() {
  log("Beginning main loop\r\n");

  Timer loop_timer;
  us_timestamp_t loop_start_us;
  us_timestamp_t time_since_last_poll_us;

  loop_timer.start();
  loop_start_us = loop_timer.read_high_resolution_us();
  last_loop_start_us = loop_start_us - POLL_PERIOD_US + 200;

  for(;;) {
    do {
      loop_start_us = loop_timer.read_high_resolution_us();
      time_since_last_poll_us = (loop_start_us - last_loop_start_us);
      us_timestamp_t wait_left_us = POLL_PERIOD_US - time_since_last_poll_us;

      if(wait_left_us > POLL_PERIOD_US) {
        // excessive wait time (negative would roll over) means we passed our
        // deadline
        break;
      }

      if(wait_left_us > 2000) {
        wait_ms((wait_left_us - 200) / 1000);
      } else if(wait_left_us > 50) {
        wait_us(wait_left_us - 50);
      }
      // otherwise just busy-wait
    } while(time_since_last_poll_us < POLL_PERIOD_US);

    last_loop_start_us += POLL_PERIOD_US;
    int32_t loop_start_error = (int32_t)(loop_timer.read_high_resolution_us() - last_loop_start_us);
    if(abs(loop_start_error) > LOOP_TIMING_EPSILON_US) {
      log("Excessive drift in loop start time of "); log_d(loop_start_error);
        log("us\r\n");
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
  do_print = false;
  do_reset = false;

  while(serial_comms.readable()) {
    int c = serial_comms.getc();
    parseCommand((uint8_t)c);
  }

  if(do_reset) {
    auto_poll = false;
    rumble_motor_0 = false;
    rumble_motor_1 = 0;
    psg.end();
    psg.begin(use_analog, use_pressure, true);
  } else {
    if(do_print || auto_poll) {
      printGamepadValues();
    }
  }
  //log_x(psg.getButtons() & ~extra_buttons, 4);
  //log("\r\n");
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
      parser_state = PS_AUTO_PARAM;
      auto_poll = false;
      break;
    case 'm': case 'M':
      parser_state = PS_MODE_PARAM;
      break;
    case 'p': case 'P':
      do_print = true;
      break;
    case 'r': case 'R':
      do_reset = true;
      break;
    case 'v': case 'V':
      parser_state = PS_RUMBLE_PARAM_0;
      rumble_motor_0_staging = false;
      rumble_motor_1_staging = 0;
      break;
    default:
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
  print(serial_comms, "P ");
  print_x(serial_comms, (uint32_t)last_loop_start_us, 8);
#if 0
  print(serial_comms, "/");
  print_x(serial_comms, (uint32_t)last_read_work_us, 4);
  print(serial_comms, ",");
  print_x(serial_comms, (uint32_t)last_loop_work_us, 4);
#endif
  print(serial_comms, ":");
  uint8_t status = psg.getStatus();
  print_x(serial_comms, status, 2);
  print(serial_comms, ":");
  print_x(serial_comms, psg.getButtons() & ~extra_buttons, 4);
  if(status == PSCS_ANALOG || status == PSCS_PRESSURE) {
    print(serial_comms, "/");
    print_x(serial_comms, psg.getAnalog(PSS_LX), 2);
    print(serial_comms, ",");
    print_x(serial_comms, psg.getAnalog(PSS_LY), 2);
    print(serial_comms, ";");
    print_x(serial_comms, psg.getAnalog(PSS_RX), 2);
    print(serial_comms, ",");
    print_x(serial_comms, psg.getAnalog(PSS_RY), 2);
  }
  if(status == PSCS_PRESSURE) {
    print(serial_comms, "/");
    print_x(serial_comms, psg.getAnalog(PSAB_PAD_RIGHT), 2);
    for(uint8_t i = 1; i < 12; ++i) {
      print(serial_comms, ",");
      print_x(serial_comms, psg.getAnalog(i + PSAB_PAD_RIGHT), 2);
    }
  }
  print(serial_comms, "\r\n");
}


