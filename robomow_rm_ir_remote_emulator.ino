#include <stdint.h>

#define IR_OUTPUT 3

#define BIT_LEN_H 900
#define BIT_LEN_L 670

static uint8_t incomingByte = 0;
static bool irState = false;

void changeSignalEdge() {
    digitalWrite(IR_OUTPUT, irState);
    irState = !irState;
}

void sendDataBits(uint32_t data, uint8_t bits) {
  for (uint8_t i = 0; i < bits; i++) {
    if (data & ((uint32_t)1u << i)) {
      delayMicroseconds(BIT_LEN_H);
    } else {
      delayMicroseconds(BIT_LEN_L);
    }
    changeSignalEdge();
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
  changeSignalEdge();
  delay(12);
  changeSignalEdge();
  // header
  sendDataBits(header, 12);
  // body
  sendDataBits(body, 23);
  // close
  delay(13);
  changeSignalEdge();

  delay(100);
}
