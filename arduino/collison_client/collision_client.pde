#include "TimerOne.h"

/*
  Blink
 Turns on an LED on for one second, then off for one second, repeatedly.
 
 This example code is in the public domain.
 */
int fireFlag = false;
unsigned int lastShotFired;
const int numBitsUsedInMessage = 11;
  
enum ir_state {
  neutral = 0,
  tx = 1,
  rx = 2,
};

enum pins {
  IR_IN = 2,
  BUTTON = 3,
  IROUT = 11,
};

//IR related globals
unsigned int irState = neutral;
unsigned int transmitBuffer = 0;
unsigned int receiveBuffer = 0;
unsigned int txrxCount = 0;
boolean hasMessage;

boolean freqState;

struct settings {
  boolean autoFireAllowed;
  boolean fireAllowed;
  unsigned int fireDelay;
};

settings settings;

void setup() {
  // initialize the digital pin as an output.
  // Pin 13 has an LED connected on most Arduino boards:
  pinMode(IROUT, OUTPUT);
  pinMode(BUTTON, INPUT);  
  pinMode(IR_IN, INPUT);  

  Serial.begin(9600);

  lastShotFired = 0;
  settings.autoFireAllowed = false; // Not implemented
  settings.fireAllowed = true;
  settings.fireDelay = 750;
  
  Timer1.initialize(240);
  Timer1.attachInterrupt(irHandler);
}

void loop() {
  unsigned int loopStartTime = millis();

  if (digitalRead(BUTTON)) {
    if (settings.fireAllowed && (loopStartTime - lastShotFired > settings.fireDelay)) {
      lastShotFired = loopStartTime;
        settings.fireAllowed = false;
      fire();
    }
  } 
  else {
    settings.fireAllowed = true;
  }
}

void fire() {
  transmitBuffer = prepareMessage(0xaa);
//  transmitBuffer = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87 };
  hasMessage = true;
}

/*
 * Start bit + message + checksum
 */
int prepareMessage(const byte message) {
  return 2048 + (message << 2) + ((256 - message) % 4);
}

// @todo Append message to transmit buffer for queue ability?
void irHandler() {
  if(irState == rx) {
    receive();
  //} else if (irState == tx || hasMessage) {
  //  irState = tx;
  //  transmit();
  } else if (digitalRead(IR_IN)) {
    irState = rx;
  }
}

void transmit() {
  // IR receiver filters on 38khz. 13 + 13 microseconds for pulses gives this.
  if (!!(transmitBuffer & (1 << (numBitsUsedInMessage - txrxCount)))) {
    for (unsigned int repeat = 0; repeat < 7; repeat++) {
      digitalWrite(IROUT, HIGH);
      delayMicroseconds(13);
      digitalWrite(IROUT, LOW);
      delayMicroseconds(13); 
    }
  }
  
  txrxCount++;
  if (txrxCount == numBitsUsedInMessage) {
    txrxCount = 0;
    hasMessage = false;
    irState = neutral;
  }
}

void receive() { 
  txrxCount++;
  receiveBuffer << 1;
  if (digitalRead(IR_IN)) {
    receiveBuffer++;
  }
  
  if (txrxCount == numBitsUsedInMessage - 1) {
    txrxCount = 0;
    irState = neutral;
    Serial.println(receiveBuffer);
    Serial.println(receiveBuffer, BIN);
    receiveBuffer = 0;
  }
}
