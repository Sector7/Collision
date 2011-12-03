#include "TimerOne.h"

/*
  Blink
 Turns on an LED on for one second, then off for one second, repeatedly.
 
 This example code is in the public domain.
 */
int fireFlag = false;
unsigned int lastShotFired;
byte transmitBuffer[8];
unsigned int transmitBufferPos = 0;

struct settings {
  boolean autoFireAllowed;
  boolean fireAllowed;
  unsigned int fireDelay;
};

enum pins {
  BUTTON = 3,
  IROUT = 11,
  IR_IN = 2,
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
  settings.fireDelay = 1000;
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
  
  if (digitalRead(IR_IN)) {
    Serial.println("Got data");
  }
}

void fire() {
  transmitBuffer = { 
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87   }; //, 0x95, 0xac, 0xf6, 0x1c, 0x95, 0xac, 0xaa, 0xdc };
  //byte userid[] = { 0x91, 0x9a, 0xbf, 0xd3, 0x37, 0x19, 0x4f, 0xec, 0x95, 0xac, 0xf6, 0x1c, 0x95, 0xac, 0xaa, 0xdc };
  transmitIR();
}

// @todo Append message to transmitbuffer for queue ability
void transmitIR() {

  // IR receiver filters on 38khz. 13 + 13 microseconds for pulses gives this.

  for (int transmitBufferPos = 0; transmitBufferPos < 8; transmitBufferPos++) {
    for (int bitPos = 0; bitPos < 8; bitPos++) {
      if (!!(transmitBuffer[transmitBufferPos] & (1 << (7 - bitPos)))) {
        for (unsigned int repeat = 0; repeat < 7; repeat++) {
          digitalWrite(IROUT, HIGH);
          delayMicroseconds(13);
          digitalWrite(IROUT, LOW);
          delayMicroseconds(13);
        }
      }
      else {
        delayMicroseconds(15 * 2 * 8);
      }
    }
    delayMicroseconds(15 * 2 * 8);
  }

  /*  transmitBufferPos++;
   if (transmitBufferPos >= 8) {
   transmitBufferPos = 0;
   }*/
}

