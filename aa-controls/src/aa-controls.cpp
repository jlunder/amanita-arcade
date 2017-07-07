#include <Arduino.h>
#include <PSGamepad.h>


#define PSG_ATTENTION_PIN 10

#define BUTTON_RED_PIN 2
#define BUTTON_GREEN_PIN 4
#define BUTTON_BLUE_PIN 7
#define BUTTON_PINK_PIN 8

#define PWM_RED_PIN 3
#define PWM_GREEN_PIN 5
#define PWM_BLUE_PIN 6
#define PWM_PINK_PIN 9

#define BUTTON_IDLE_PWM 64
#define BUTTON_PRESSED_PWM 255


enum ParserState {
  PS_IDLE,
  PS_AUTO_PARAM,
  PS_MODE_PARAM,
  PS_RUMBLE_PARAM_0,
  PS_RUMBLE_PARAM_1,
  PS_RUMBLE_PARAM_2,
};


char const hexDigits[16] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};


uint32_t const pollMicros = 10000;
uint32_t lastMicros;

PSGamepad psg;

uint32_t lastReadMicros;
uint32_t lastReadDuration;
uint32_t lastLoopDuration;

ParserState parserState = PS_IDLE;
bool autoPoll = false;
bool useAnalog = true;
bool usePressure = false;
bool rumbleMotor0Staging = false;
bool rumbleMotor0 = false;
uint8_t rumbleMotor1Staging = 0;
uint8_t rumbleMotor1 = 0;
uint16_t extraButtons = 0;

bool doPrint;
bool doReset;


void pollGamepad();
void executeCommands();
void parseCommand(uint8_t command);
void parseIdleCommand(uint8_t command);
void parseModeParam(uint8_t command);
uint8_t parseHexDigit(uint8_t digit);
void printGamepadValues();
char * printHexUint32(char * p, uint32_t val);
char * printHexUint16(char * p, uint16_t val);
char * printHexUint8(char * p, uint8_t val);


void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_RED_PIN, INPUT_PULLUP);
  pinMode(BUTTON_GREEN_PIN, INPUT_PULLUP);
  pinMode(BUTTON_BLUE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PINK_PIN, INPUT_PULLUP);

  pinMode(PWM_RED_PIN, OUTPUT);
  pinMode(PWM_GREEN_PIN, OUTPUT);
  pinMode(PWM_BLUE_PIN, OUTPUT);
  pinMode(PWM_PINK_PIN, OUTPUT);

  analogWrite(PWM_RED_PIN, BUTTON_IDLE_PWM);
  analogWrite(PWM_GREEN_PIN, BUTTON_IDLE_PWM);
  analogWrite(PWM_BLUE_PIN, BUTTON_IDLE_PWM);
  analogWrite(PWM_PINK_PIN, BUTTON_IDLE_PWM);

  lastMicros = micros();
  psg.begin(PSG_ATTENTION_PIN, useAnalog, usePressure, true);
}


void loop() {
  uint32_t loopMicros;
  uint32_t microsSinceLastPoll;

  do {
    loopMicros = micros();
    microsSinceLastPoll = (loopMicros - lastMicros);
  } while(microsSinceLastPoll < pollMicros);

  if(microsSinceLastPoll >= pollMicros * 2) {
    lastMicros = loopMicros;
  } else {
    lastMicros += pollMicros;
  }

  //digitalWrite(2, 1);
  pollGamepad();
  executeCommands();
  //digitalWrite(2, 0);

  lastLoopDuration = micros() - loopMicros;
}


void pollGamepad() {
  //digitalWrite(3, 1);
  lastReadMicros = micros();
  psg.poll(rumbleMotor0, rumbleMotor1);
  lastReadDuration = micros() - lastReadMicros;

  extraButtons = 0;
  if(!digitalRead(BUTTON_RED_PIN)) {
    extraButtons |= PSB_RED;
    analogWrite(PWM_RED_PIN, BUTTON_PRESSED_PWM);
  } else {
    analogWrite(PWM_RED_PIN, BUTTON_IDLE_PWM);
  }
  if(!digitalRead(BUTTON_GREEN_PIN)) {
    extraButtons |= PSB_GREEN;
    analogWrite(PWM_GREEN_PIN, BUTTON_PRESSED_PWM);
  } else {
    analogWrite(PWM_GREEN_PIN, BUTTON_IDLE_PWM);
  }
  if(!digitalRead(BUTTON_BLUE_PIN)) {
    extraButtons |= PSB_BLUE;
    analogWrite(PWM_BLUE_PIN, BUTTON_PRESSED_PWM);
  } else {
    analogWrite(PWM_BLUE_PIN, BUTTON_IDLE_PWM);
  }
  if(!digitalRead(BUTTON_PINK_PIN)) {
    extraButtons |= PSB_PINK;
    analogWrite(PWM_PINK_PIN, BUTTON_PRESSED_PWM);
  } else {
    analogWrite(PWM_PINK_PIN, BUTTON_IDLE_PWM);
  }
  //digitalWrite(3, 0);
}


void executeCommands() {
  //digitalWrite(4, 1);
  doPrint = false;
  doReset = false;

  while(Serial.available()) {
    parseCommand(Serial.read());
  }
  //digitalWrite(4, 0);

  //digitalWrite(5, 1);
  if(doReset) {
    autoPoll = false;
    rumbleMotor0 = false;
    rumbleMotor1 = 0;
    psg.end();
    psg.begin(PSG_ATTENTION_PIN, useAnalog, usePressure, true);
  } else {
    if(doPrint || autoPoll) {
      printGamepadValues();
    }
  }
  //digitalWrite(5, 0);
}


void parseCommand(uint8_t command) {
  switch(parserState) {
    case PS_IDLE:
      parseIdleCommand(command);
      break;
    case PS_AUTO_PARAM:
      autoPoll = (command == '1');
      parserState = PS_IDLE;
      break;
    case PS_MODE_PARAM:
      parseModeParam(command);
      parserState = PS_IDLE;
      break;
    case PS_RUMBLE_PARAM_0:
      if(command == '\n') {
        parserState = PS_IDLE;
      } else {
        rumbleMotor0Staging = (command == '1');
        parserState = PS_RUMBLE_PARAM_1;
      }
      break;
    case PS_RUMBLE_PARAM_1:
      if(command == '\n') {
        parserState = PS_IDLE;
      } else {
        rumbleMotor1Staging |= parseHexDigit(command) << 4;
        parserState = PS_RUMBLE_PARAM_2;
      }
      break;
    case PS_RUMBLE_PARAM_2:
      if(command == '\n') {
        parserState = PS_IDLE;
      } else {
        rumbleMotor1Staging |= parseHexDigit(command) << 0;
        rumbleMotor0 = rumbleMotor0Staging;
        rumbleMotor1 = rumbleMotor1Staging;
        parserState = PS_IDLE;
      }
      break;
    default:
      parserState = PS_IDLE;
      break;
  }
}


void parseIdleCommand(uint8_t command) {
  switch(command) {
    case 'a': case 'A':
      parserState = PS_AUTO_PARAM;
      autoPoll = false;
      break;
    case 'm': case 'M':
      parserState = PS_MODE_PARAM;
      break;
    case 'p': case 'P':
      doPrint = true;
      break;
    case 'r': case 'R':
      doReset = true;
      break;
    case 'v': case 'V':
      parserState = PS_RUMBLE_PARAM_0;
      rumbleMotor0Staging = false;
      rumbleMotor1Staging = 0;
      break;
    default:
      break;
  }
}


void parseModeParam(uint8_t command) {
  switch(command) {
    default:
    case '0':
      doReset = true;
      useAnalog = false;
      usePressure = false;
      break;
    case '1':
      doReset = true;
      useAnalog = true;
      usePressure = false;
      break;
    case '2':
      doReset = true;
      useAnalog = true;
      usePressure = true;
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
  char temp[100];
  char * p = temp;
  *(p++) = 'P';
  *(p++) = ' ';
  p = printHexUint32(p, lastReadMicros);
#if 0
  *(p++) = '/';
  p = printHexUint16(p, lastReadDuration);
  *(p++) = ',';
  p = printHexUint16(p, lastLoopDuration);
#endif
  *(p++) = ':';
  uint8_t status = psg.getStatus();
  p = printHexUint8(p, status);
  *(p++) = ':';
  p = printHexUint16(p, psg.getButtons() & ~extraButtons);
  if(status == PSCS_ANALOG || status == PSCS_PRESSURE) {
    *(p++) = '/';
    p = printHexUint8(p, psg.getAnalog(PSS_LX));
    *(p++) = ',';
    p = printHexUint8(p, psg.getAnalog(PSS_LY));
    *(p++) = ';';
    p = printHexUint8(p, psg.getAnalog(PSS_RX));
    *(p++) = ',';
    p = printHexUint8(p, psg.getAnalog(PSS_RY));
  }
  if(status == PSCS_PRESSURE) {
    *(p++) = '/';
    p = printHexUint8(p, psg.getAnalog(PSAB_PAD_RIGHT));
    for(uint8_t i = 1; i < 12; ++i) {
      *(p++) = ',';
      p = printHexUint8(p, psg.getAnalog(i + PSAB_PAD_RIGHT));
    }
  }
  *(p++) = 0;
  Serial.println(temp);
}


char * printHexUint32(char * p, uint32_t val) {
  *(p++) = hexDigits[(val >> 28) & 0xF];
  *(p++) = hexDigits[(val >> 24) & 0xF];
  *(p++) = hexDigits[(val >> 20) & 0xF];
  *(p++) = hexDigits[(val >> 16) & 0xF];
  *(p++) = hexDigits[(val >> 12) & 0xF];
  *(p++) = hexDigits[(val >>  8) & 0xF];
  *(p++) = hexDigits[(val >>  4) & 0xF];
  *(p++) = hexDigits[(val >>  0) & 0xF];
  return p;
}

char * printHexUint16(char * p, uint16_t val) {
  *(p++) = hexDigits[(val >> 12) & 0xF];
  *(p++) = hexDigits[(val >>  8) & 0xF];
  *(p++) = hexDigits[(val >>  4) & 0xF];
  *(p++) = hexDigits[(val >>  0) & 0xF];
  return p;
}

char * printHexUint8(char * p, uint8_t val) {
  *(p++) = hexDigits[(val >>  4) & 0xF];
  *(p++) = hexDigits[(val >>  0) & 0xF];
  return p;
}
