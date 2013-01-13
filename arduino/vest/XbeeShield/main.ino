#include <XBee.h>
#include <TimerOne.h>
#include "main.h"


XBee xbee = XBee();

XBeeResponse response = XBeeResponse();
Rx64Response rx64 = Rx64Response();

using namespace collision;

/* Stamp vars*/
boolean isOnline = true;
byte targetColor[3];
byte teamColor[3];
byte currentColor[3];
unsigned long nextRgbCall;
unsigned long nextSoundSwitch;
unsigned long soundDelay;
boolean soundState;

/* Leths flags */
int fireFlag = false;
unsigned int lastShotFired;
const int numBitsUsedInMessage = 11;

// Game parameters
// @TODO Merge into a struct/hashmap?
unsigned long isDeadUntil;
unsigned long isSafeUntil;
unsigned int  shotsLeft;
unsigned int  lifeLeft;
byte playerId = 0x04;

//IR related globals
const int IRpulseTimer = 480;
unsigned int irState = neutral;
unsigned int transmitBuffer = 0;
unsigned int receiveBuffer = 0;
unsigned int txrxCount = 0;
boolean hasMessage;
unsigned int irTxCount = 0;
unsigned int irTxDelay = 0;

boolean freqState;

struct settings {
  boolean autoFireAllowed;
  boolean fireAllowed;
  unsigned int fireDelay;
};

settings settings;


collision::event eventList[100];

/**
 * Called by IR or Xbee receives, timers etc
 */
void triggerEvent(unsigned int eventId, unsigned int eventData) {
    switch (eventId) {
        case 0:
            break;
        // Hard coded events first
        case IR_EVENT : //1
            currentColor[2] = 255;
            triggerEventRemote(BROADCAST_LOW, BROADCAST_HIGH, eventId, eventData);
            break;
        case FIRE_EVENT:
            transmitBuffer = prepareMessage(playerId);
            hasMessage = true;

            soundDelay = 100;
            nextSoundSwitch = 0;
            break;
        case DIE_EVENT:
            isDeadUntil = millis() + 3000;
            isSafeUntil = millis() + 6000;
            currentColor[0] = 255;
            currentColor[1] = 255;
            currentColor[2] = 255;
            //analogWrite(VIBRATE,255);
        default: // Configurable events after
            for (int i = 0; i < 100; i++) {
                if (eventList[i].id == 0) 
                    break;

                if (eventList[i].id == eventId) {
                    if (eventList[i].targetSelf()) {
                        triggerEvent(eventList[i].event_id, eventData);
                    } else {
                        triggerEventRemote(eventList[i].addr_high, eventList[i].addr_low, eventList[i].id, eventData);
                    }
                }
            }
            break;
    }
}

void triggerEventRemote(unsigned int addr_low, unsigned int addr_high, unsigned int eventId, unsigned int eventData) {
  uint8_t payload[] = {
    0x01,
    eventId >> 8 & 0xFF,
    eventId & 0xFF,
    eventData >> 8 & 0xFF,
    eventData & 0xFF,
  };

  XBeeAddress64 addr64 = XBeeAddress64(addr_low, addr_high);

  // Specify the address of the remote XBee (this is the SH + SL)
  ZBTxRequest zbTx = ZBTxRequest(
    addr64, 
    payload, 
    sizeof(payload)
  );

  xbee.send(zbTx);
}


void setup() {/*{{{*/

  // initialize the digital pin as an output.
  // Pin 13 has an LED connected on most Arduino boards:
  pinMode(IROUT, OUTPUT);
  pinMode(SPEAKER, OUTPUT);
  pinMode(BUTTON, INPUT);
  pinMode(IR_IN, INPUT);
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);

  digitalWrite(GREEN,true);

  Serial.begin(9600);
  // Setup XBee and send hello
  xbee.setSerial(Serial);
  //sendIndentify( XBeeAddress64(0x00000000, 0x0000FFFF), 0x0003 );

  lastShotFired = 0;
  settings.autoFireAllowed = false; // Not implemented
  settings.fireAllowed = true;
  settings.fireDelay = 750;

  /*attachInterrupt(0,startIrRecv,FALLING);
  Timer1.initialize(IRpulseTimer);
  Timer1.attachInterrupt(irHandler);
    */

  teamColor[0] = 0;
  teamColor[1] = 20;
  teamColor[2] = 20;

}/*}}}*/

void loop() {/*{{{*/
  digitalWrite(GREEN,!digitalRead(GREEN));

    /*if (digitalRead(BUTTON)) {
        if (settings.fireAllowed && (loopStartTime - lastShotFired > settings.fireDelay)) {
            lastShotFired = loopStartTime;
            settings.fireAllowed = false;
            trigger_event(FIRE_EVENT);
            fire();
        }
    }
    else {
        settings.fireAllowed = true;
    }*/


    xbee.readPacket();
    if (xbee.getResponse().isAvailable()) {
        // got something
        if (xbee.getResponse().getApiId() == 0x90) {
            // got a rx packet
            xbee.getResponse().getRx64Response(rx64);
            parseCommand();
        }
    }

    // Fade RGB each 5 ms
    if ( millis() > nextRgbCall ) {
        nextRgbCall = millis() + 5;
        fadeRgb();
    }

    // Sound, peeow peeow
    if ( soundDelay>0 && micros() > nextSoundSwitch ) {
        nextSoundSwitch = micros() + soundDelay;
        soundState = !soundState;
        digitalWrite(SPEAKER,soundState);
        if (soundDelay<1000) {
            soundDelay += 2;
            if (soundDelay > 500) {
                //analogWrite(VIBRATE,80);
            }
        } else {
            soundDelay = 0;
            digitalWrite(SPEAKER,false);
            //analogWrite(VIBRATE,0);
        }
    }
    

   // if ( irTxDelay > 0 && millis() > irTxDelay ) {
   //     hasMessage = true;
   //     irTxDelay = millis() + 100;
   // }
}/*}}}*/





void startIrRecv(){/*{{{*/
  if (irState == neutral) {
    detachInterrupt(0);
    delayMicroseconds(IRpulseTimer / 2);
    TCNT1 = 0;
    irState = rx;
  }
}/*}}}*/
/*
 * Start bit + message + checksum
 */
int prepareMessage(byte message) {/*{{{*/
  return 1024 + (message << 2) + ((255 - message) % 4);
}/*}}}*/

// @todo Append message to transmit buffer for queue ability?
void irHandler() {/*{{{*/
  if(irState == rx) {
    receive();
    currentColor[0] = 255;
  } else if (irState == tx || hasMessage) {
    irState = tx;
    //transmit();
  }
}/*}}}*/

void transmit() {/*{{{*/
  // IR receiver filters on 38khz. 13 + 13 microseconds for pulses gives this.
  if (!!(transmitBuffer & (1 << (numBitsUsedInMessage - txrxCount)))) {
    for (unsigned int repeat = 0; repeat < 14; repeat++) {
      digitalWrite(IROUT, HIGH);
      delayMicroseconds(13);
      digitalWrite(IROUT, LOW);
      delayMicroseconds(13);
    }
  }

  txrxCount++;
  if (txrxCount == numBitsUsedInMessage + 1) {
    txrxCount = 0;
    hasMessage = false;

    irState = neutral;
    irTxCount++;

    if (irTxCount > 4) {
      irTxDelay = 0;
      irTxCount = 0;
    } else {
      irTxDelay = millis() + 20;
    }
  }
}/*}}}*/





/**
 * Read a byte, bit by bit from IR_IN.
 * Then trigger IR_EVENT with the read data
 */
void receive() {/*{{{*/
  txrxCount++;
  receiveBuffer << 1;

  if (!digitalRead(IR_IN)) {
    receiveBuffer++;
  }

  if (txrxCount >= numBitsUsedInMessage - 1) {
    triggerEvent(IR_EVENT, receiveBuffer);

    txrxCount = 0;
    receiveBuffer = 0;
    irState = neutral;

    /*Serial.println("CODE");
    Serial.println(receiveBuffer, HEX);
    Serial.println(receiveBuffer, BIN);*/

    attachInterrupt(0,startIrRecv,FALLING);
  }
}/*}}}*/

void parseCommand( ) {/*{{{*/
  switch(rx64.getData(1)) {
/*   
    case 0x02: // Set rgb color
      teamColor[0] = rx64.getData(2);
      teamColor[1] = rx64.getData(3);
      teamColor[2] = rx64.getData(4);
      playerId = rx64.getData(5);
      //sendStatus( rx.getRemoteAddress64() );
      break;
    case 0x03: // Shoot
      soundDelay = 100;
      nextSoundSwitch = 0;
      isDeadUntil = millis() + 3000;
      isSafeUntil = millis() + 6000;
      currentColor[0] = 255;
      currentColor[1] = 255;
      currentColor[2] = 255;
      break;
    case 0x04: // WhoAmI
      sendStatus( rx64.getRemoteAddress64() );
      break;*/
   case 0x00: // Server asks for identification
      // Green
      targetColor[0] = 0;
      targetColor[1] = 255;
      targetColor[2] = 0;
      isOnline = true;
      sendIndentify( rx64.getRemoteAddress64(), 0x0003 );
      break;
    case 0x01: // Remote event
      triggerEvent(
        (rx64.getData(2)<<8+rx64.getData(3)),
        (rx64.getData(4)<<8+rx64.getData(5))
      );
      break;
    case 0xff: // Coordinator/Gameserver went offline
      tone(8, 440, 500);
      isOnline = false;
      currentColor[0] = 255;
      currentColor[1] = 255;
      currentColor[2] = 0;
      break;
    default:
      sendUnknownCommand( rx64.getRemoteAddress64() );
      break;
  }
}/*}}}*/

void sendIndentify( XBeeAddress64 addr64, unsigned int model ) {/*{{{*/
  uint8_t payload[] = {
    0x00, // Identification cmd
    (model >> 8) & 0xFF,
    model & 0xFF,
  };

  // Specify the address of the remote XBee (this is the SH + SL)
  ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
  xbee.send(zbTx);
}/*}}}*/
void sendShoot( XBeeAddress64 addr64 ) {/*{{{*/
  uint8_t payload[] = {
    0x02, // WhoAmI cmd
  };

  // Specify the address of the remote XBee (this is the SH + SL)
  ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
  xbee.send(zbTx);
}/*}}}*/
void sendUnknownCommand( XBeeAddress64 addr64 ) {/*{{{*/
  uint8_t payload[] = {
    0x00, // Unknown command
    rx64.getData(0),
    rx64.getData(1),
    rx64.getData(2),
    rx64.getData(3),
    rx64.getData(4)
  };
  ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
  xbee.send(zbTx);
}/*}}}*/

void fadeRgb() {/*{{{*/
  if ( isOnline ) {
    if ( isDeadUntil > millis() ) {
      targetColor[0] = 0;
      targetColor[1] = 0;
      targetColor[2] = 0;
    } else if ( isSafeUntil > millis() ) {
      targetColor[0] = teamColor[0]/5;
      targetColor[1] = teamColor[1]/5;
      targetColor[2] = teamColor[2]/5;
    } else {
      targetColor[0] = teamColor[0];
      targetColor[1] = teamColor[1];
      targetColor[2] = teamColor[2];
    }

    if ( currentColor != targetColor ) {
      if ( currentColor[0] != targetColor[0] ) {
        if ( currentColor[0]<targetColor[0] ) {
          currentColor[0]++;
        } else {
          currentColor[0]--;
        }
        analogWrite(RED, currentColor[0]);
      }
      if ( currentColor[1] != targetColor[1] ) {
        if ( currentColor[1]<targetColor[1] ) {
          currentColor[1]++;
        } else {
          currentColor[1]--;
        }
        analogWrite(GREEN, currentColor[1]);
      }
      if ( currentColor[2] != targetColor[2] ) {
        if ( currentColor[2]<targetColor[2] ) {
          currentColor[2]++;
        } else {
          currentColor[2]--;
        }
        analogWrite(BLUE, currentColor[2]);
      }
    }
  } else {
    currentColor[0] += 3;
    currentColor[1] += 3;
    currentColor[2] = 0;
    analogWrite(RED, currentColor[0]);
    analogWrite(GREEN, currentColor[1]);
    analogWrite(BLUE, currentColor[2]);
  }
}/*}}}*/
