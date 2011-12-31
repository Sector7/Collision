#include "TimerOne.h"

/**
 * Collision game handler
 */

int fireFlag = false;
unsigned int lastShotFired;
const int numBitsUsedInMessage = 11;
const int IRpulseTimer = 480;

enum ir_state {
  neutral = 0,
  tx = 1,
  rx = 2,
};

enum pins {
  IR_IN = 2,
  BUTTON = 3,
  IR_OUT = 11,
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
  pinMode(IR_OUT, OUTPUT);
  pinMode(BUTTON, INPUT);
  pinMode(IR_IN, INPUT);

  Serial.begin(9600);

  lastShotFired = 0;
  settings.autoFireAllowed = false; // Not implemented
  settings.fireAllowed = true;
  settings.fireDelay = 750;

  attachInterrupt(0,startIrRecv,RISING);
  Timer1.initialize(IRpulseTimer);
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
  hasMessage = true;
}

/*
 * Start bit + message + checksum
 */
int prepareMessage(const byte message) {
  return 2048 + (message << 2) + ((256 - message) % 4);
}

void irHandler() {
  if(irState == rx) {
    receive();
  } 
  else if (irState == tx || hasMessage) {
    irState = tx;
    transmit();
  }
}

void startIrRecv(){
  if (irState == neutral) {
    irState = rx;
    delayMicroseconds(IRpulseTimer / 2);
    Timer1.restart();
  }
}

void transmit() {
  // IR receiver filters on 38khz. 13 + 13 microseconds for pulses gives this.
  if (!!(transmitBuffer & (1 << (numBitsUsedInMessage - txrxCount)))) {
    for (unsigned int repeat = 0; repeat < 14; repeat++) {
      digitalWrite(IR_OUT, HIGH);
      delayMicroseconds(13);
      digitalWrite(IR_OUT, LOW);
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
  receiveBuffer = (receiveBuffer << 1);
  if (digitalRead(IR_IN)) {
    receiveBuffer++;
  }

  if (txrxCount == numBitsUsedInMessage) {
    unsigned int chkRecv = 0;
    txrxCount = 0;
    irState = neutral;
    //receiveBuffer = receiveBuffer % 256;
    receiveBuffer &= 1023;
    chkRecv = receiveBuffer & 3;
    receiveBuffer = (receiveBuffer >> 2);
    if((receiveBuffer + chkRecv) % 4 == 3) {
      Serial.println(receiveBuffer,HEX);
    }
    receiveBuffer = 0;
    //Timer1.stop();
  }  
}

