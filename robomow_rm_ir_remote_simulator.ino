#include <stdint.h>

#define IR_OUTPUT 3

#define BIT_LEN_H 900
#define BIT_LEN_L 670

void setup() {
  // put your setup code here, to run once:
  pinMode(IR_OUTPUT, OUTPUT);
  Serial.begin(115200);
}

void loop() 
{

  static uint8_t incomingByte = 0;
  static bool irState = false;
  // put your main code here, to run repeatedly:
  // send data only when you receive data:
  while (Serial.available() > 0) {
    incomingByte = Serial.read();
    Serial.write(incomingByte);
  }
    
  // preamble
  for (uint8_t i = 0; i<1; i++) {
    digitalWrite(IR_OUTPUT, irState);
    irState = !irState;
    delay(12);
  }

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

  digitalWrite(IR_OUTPUT, irState);
  irState = !irState;

  for (uint8_t i = 0; i<12; i++) {
    if (header & ((uint32_t)1u<<i))
      delayMicroseconds(BIT_LEN_H);
    else
      delayMicroseconds(BIT_LEN_L);
    digitalWrite(IR_OUTPUT, irState);
    irState = !irState;
  }

  for (uint8_t i = 0; i<23; i++) {
    if (body & ((uint32_t)1u<<i))
      delayMicroseconds(BIT_LEN_H);
    else
      delayMicroseconds(BIT_LEN_L);
    digitalWrite(IR_OUTPUT, irState);
    irState = !irState;        
  }


  delay(13);
  digitalWrite(IR_OUTPUT, irState);
  irState = !irState;
  // close
  /*for (uint8_t i = 0; i<2; i++) {
    digitalWrite(IR_OUTPUT, irState);
    irState = !irState;
    delay(12);
  }*/
  delay(100);
}
