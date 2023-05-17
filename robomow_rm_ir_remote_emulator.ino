#include <stdint.h>

#define IR_OUTPUT 3

#define BIT_LEN_H 900
#define BIT_LEN_L 670

static uint8_t incomingByte = 0;
static bool irState = false;

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

void receiveSerialCommand() {
  while (Serial.available() > 0) {
    incomingByte = Serial.read();
    Serial.write(incomingByte);
  }
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
  receiveSerialCommand();
    
  uint32_t header = 0x5A5;
  uint32_t body = 0xAAAAAA;

  if (incomingByte == '1')
    body = 0x5AAAAA;
  else if (incomingByte == '2')
    body = 0xA5AAAA;
  else if (incomingByte == '3')
    body = 0xAA5AAA;
  else if (incomingByte == '4')
    body = 0xAAA5AA;
  else if (incomingByte == '5')
    body = 0xAAAA5A;
  else if (incomingByte == '6')
    body = 0xAAAAA5;

  // preamble
  toggleSignalLevel();
  delay(12);
  toggleSignalLevel();
  // header
  sendDataBits(header, 12);
  // body
  sendDataBits(body, 23);
  // close
  delay(13);
  toggleSignalLevel();

  delay(100);
}
