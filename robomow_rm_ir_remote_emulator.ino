#include <stdint.h>

#define IR_OUTPUT 3

#define BIT_LEN_H     1000
#define BIT_LEN_L     500
#define DEFAULT_BODY  0xAAAAAA

static bool irState = false;
static uint32_t body = DEFAULT_BODY;

/*
 * If oscOn = true, then delivers a X kHz 50% duty cycle signal on OC2B (pin 3), otherwise 3 held low
 */
void modulateCarrier(bool oscOn)
{
  if (oscOn) {
    // switch on carrier
    TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20); //  Mode 7 (fast PWM),  Output to pin 3 (OC2B)
  } else {
    // switch off carrier
    TCCR2A = 0;
    digitalWrite(IR_OUTPUT, LOW); // ensure pin is not left HIGH
  }
}

void toggleSignalLevel() {
  irState = !irState;
  modulateCarrier(irState);
}

void sendDataBits(uint32_t data, uint8_t bits) {
  for (uint8_t i = 0; i < bits; i++) {
    if (data & ((uint32_t)1u << i)) {
      delayMicroseconds(BIT_LEN_H);
    } else {
      delayMicroseconds(BIT_LEN_L);
    }
    toggleSignalLevel();
  }
}

uint8_t receiveSerialCommand() {
  uint8_t incomingByte = 0;
  while (Serial.available() > 0) {
    incomingByte = Serial.read();
    Serial.write(incomingByte);
  }
  return incomingByte;
}

uint32_t generateBodyFromSerialCommand(uint32_t prevBody) {
  
}

void setup() {
  pinMode(IR_OUTPUT, OUTPUT);

  // Controls timer2. Initial configuration with prescaler 1:8
  TCCR2B = _BV(WGM22) | _BV(CS21);
  OCR2A = ((F_CPU / 1000000L)  * 55) / 17;  // 38.4kHz
  OCR2B = OCR2A / 2;  // 50% duty cycle

  Serial.begin(115200);
}

void loop() {
  uint8_t incomingByte = receiveSerialCommand();

  if (incomingByte == '1') {
    body ^= 0xF00000; // safety
  } else if (incomingByte == '2') {
    body ^= 0x0F0000; // mow
  } else if (incomingByte == '3') {
    body ^= 0x00F000; // left
  } else if (incomingByte == '4') {
    body ^= 0x000F00; // backward
  } else if (incomingByte == '5') {
    body ^= 0x0000F0; // right
  } else if (incomingByte == '6') {
    body ^= 0x00000F; // forward
  } else if (incomingByte == '0') {
    body = DEFAULT_BODY; // reset to default
  }

  // preamble
  toggleSignalLevel();
  delay(12);
  toggleSignalLevel();
  // header
  uint32_t header = 0x5A5;
  sendDataBits(header, 12);
  // body
  sendDataBits(body, 23);
  // close
  delay(13);
  toggleSignalLevel();

  delay(100);
}
