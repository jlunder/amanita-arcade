#include "PSGamepad.h"

//#define LOG_COMMANDS

#define PSG_CTRL_BYTE_DELAY 20
#define PSG_READ_DELAY 5000

#define PSC_GET_CONTROL_MAP 0x41
#define PSC_GET_CONTROLS    0x42
#define PSC_SET_CONFIG_MODE 0x43
#define PSC_SET_ANALOG_MODE 0x44
#define PSC_SET_MOTOR_MAP   0x4D
#define PSC_SET_CONTROL_MAP 0x4F


static inline uint8_t rev8(uint8_t x) {
  return __RBIT(x) >> 24;
}


PSGamepad::PSGamepad(PinName commandPin, PinName dataPin, PinName clockPin,
    PinName attentionPin)
    : _spi(commandPin, dataPin, clockPin)
    , _attention(attentionPin) {
}

void PSGamepad::begin(bool useAnalog, bool usePressure, bool useRumble) {
  _buttons = 0xFFFF;
  _lastButtons = 0xFFFF;
  for(size_t i = 0; i < NUM_ANALOG; ++i) {
    _analogData[i] = 0x80;
  }

  _rumbleEnabled = useRumble;
  _analogEnabled = useAnalog;
  _pressureEnabled = usePressure;

  _lastRead.start();

  _spi.frequency(250000);
  _spi.format(8, 3);

  _attention.write(1);

  // Cause the next poll() to configure the gamepad
  _status = PSCS_DISCONNECTED;
}


void PSGamepad::end() {
  _spi.abort_all_transfers();
}


void PSGamepad::poll() {
  poll(false, 0x00);
}


void PSGamepad::poll(bool rumbleMotor0, uint8_t rumbleMotor1) {
  uint32_t deltaMillis = _lastRead.read_ms();

  if(deltaMillis < 7) {
    return;
  }

  _lastButtons = _buttons;
  if(_status == PSCS_DISCONNECTED || deltaMillis > 1000) {
    configureGamepad();
  } else {
    readGamepad(rumbleMotor0, rumbleMotor1);
  }

  if(_status == PSCS_CONFIGURING) {
    if(_configureStart.read_ms() > 500) {
      // Something is wrong, trigger a reconfig next poll
      logp("timeout on config\r\n");
      _status = PSCS_DISCONNECTED;
    }
  } else if(_status == PSCS_DISCONNECTED) {
    // Give up for now
  } else if((_pressureEnabled && _status != PSCS_PRESSURE) ||
      (!_pressureEnabled && _analogEnabled && _status != PSCS_ANALOG) ||
      (!_pressureEnabled && !_analogEnabled && _status != PSCS_DIGITAL)) {
    // Controller is not in the right mode, force it there
    logp("controller in weird state\r\n");
    _status = PSCS_DISCONNECTED;
  }

  _lastRead.reset();
}


void PSGamepad::configureGamepad() {
  // Read state, but with no side effects
  sendCommand(PSC_GET_CONTROLS, NULL, 0, 0x00, NULL, 0);

  bool connected = true;
  _status = PSCS_DISCONNECTED;
  connected = connected && setConfigMode(true);
  if(!connected) {
    return;
  }
  connected = connected && setAnalogMode(_analogEnabled, true);
  connected = connected && setMotorMap();
  connected = connected && setControlMap(_analogEnabled, _pressureEnabled);
  connected = connected && setConfigMode(false);
  if(connected) {
    _status = PSCS_CONFIGURING;
    _configureStart.reset();
    _configureStart.start();
  } else {
    logp("configure didn't all succeed\r\n");
  }
}


void PSGamepad::readGamepad(bool rumbleMotor0, uint8_t rumbleMotor1) {
  uint8_t rxData[18];
  uint8_t result;

  if(_status == PSCS_CONFIGURING) {
    result = sendCommand(PSC_SET_CONFIG_MODE, NULL, 0, 0x00,
      rxData, sizeof rxData);
  } else {
    uint8_t txData[2] = {
      (uint8_t)(rumbleMotor0 ? 0xFF : 0x00), rumbleMotor1
    };
    result = sendCommand(PSC_GET_CONTROLS, txData, 2, 0x00,
      rxData, sizeof rxData);
  }


  switch(result) {
    case 0x41: case 0x71:
      _status = PSCS_DIGITAL;
      break;
    case 0x43: case 0x73:
      _status = PSCS_ANALOG;
      break;
    case 0xF3:
      if(_status != PSCS_CONFIGURING) {
        logp("unexpected config mode\r\n");
        _status = PSCS_DISCONNECTED;
      }
      break;
    case 0x49: case 0x79:
      _status = PSCS_PRESSURE;
      break;
    default:
      logp("bad result from poll\r\n");
      _status = PSCS_DISCONNECTED;
      break;
  }

  if(_status != PSCS_DISCONNECTED) {
    _buttons = rxData[0] | ((uint16_t)rxData[1] << 8);
    if(_status == PSCS_ANALOG) {
      memcpy(_analogData, rxData + 2, 4);
    }
    if(_status == PSCS_PRESSURE) {
      memcpy(_analogData + 4, rxData + 6, sizeof _analogData - 4);
    }
  }
}


bool PSGamepad::setConfigMode(bool configMode) {
  static uint8_t const enterTxData[] = { 0x01, 0x00 };
  static uint8_t const exitTxData[] = { 0x00, 0x00 };
  uint8_t result = sendCommand(PSC_SET_CONFIG_MODE,
    configMode ? enterTxData : exitTxData,
    sizeof enterTxData, 0x00, NULL, 0);
  return result != 0xFF;
}


bool PSGamepad::setAnalogMode(bool analogMode, bool locked) {
  uint8_t txData[] = {
    (uint8_t)(analogMode ? 0x01 : 0x00),
    (uint8_t)(locked ? 0x03 : 0x00)
  };
  uint8_t result = sendCommand(PSC_SET_ANALOG_MODE,
    txData, sizeof txData, 0x00, NULL, 0);
  return result == 0xF3;
}


bool PSGamepad::setMotorMap() {
  static uint8_t const txData[] = { 0x00, 0x01 };
  uint8_t result = sendCommand(PSC_SET_MOTOR_MAP,
    txData, sizeof txData, 0xFF, NULL, 0);
  return result == 0xF3;
}


bool PSGamepad::setControlMap(bool analog, bool pressure) {
  uint8_t txData[] = { 0x03, 0x00, 0x00, 0x00 };
  if(pressure) {
    txData[0] = 0xFF;
    txData[1] = 0xFF;
    txData[2] = 0x03;
  } else if(analog) {
    txData[0] = 0x3F;
  }

  uint8_t result = sendCommand(PSC_SET_CONTROL_MAP,
    txData, sizeof txData, 0x00, NULL, 0);
  return result == 0xF3;
}

uint8_t PSGamepad::sendCommand(uint8_t command, uint8_t const * txBuf,
    size_t txLength, uint8_t txPad, uint8_t * rxBuf, size_t rxLength) {
  _attention.write(0);

  _spi.write(rev8(0x01));
  wait_us(PSG_CTRL_BYTE_DELAY);

  uint8_t response = rev8(_spi.write(rev8(command)));
  wait_us(PSG_CTRL_BYTE_DELAY);

  _spi.write(rev8(0x00));
  wait_us(PSG_CTRL_BYTE_DELAY);

#if defined(LOG_COMMANDS)
  logp("CMD "); log_u8x2(command); logp(">"); log_u8x2(response); logp(":");
#endif

  if(response != 0xFF) {
    size_t packetSize = (response & 0x0F) << 1;
    size_t i = 0;
    for (; (i < txLength) || (i < packetSize); ++i) {
      uint8_t txByte = ((txBuf != NULL) && (i < txLength) ? txBuf[i] : txPad);
      uint8_t rxByte = rev8(_spi.write(rev8(txByte)));
#if defined(LOG_COMMANDS)
      logp(" "); log_u8x2(txByte); logp(">"); log_u8x2(rxByte);
#endif
      if((rxBuf != NULL) && (i < rxLength)) {
        rxBuf[i] = rxByte;
      }
      wait_us(PSG_CTRL_BYTE_DELAY);
    }
  }

#if defined(LOG_COMMANDS)
  logp("\r\n");
#endif
  _attention.write(1);

  wait_us(PSG_CTRL_BYTE_DELAY);

  return response;
}

