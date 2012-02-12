#include <XBee.h>
#include <TimerOne.h>
#include <galaxy_fighter.h>

void setup() {
  // initialize the digital pin as an output.
  // Pin 13 has an LED connected on most Arduino boards:
  pinMode(IROUT, OUTPUT);
  pinMode(SPEAKER, OUTPUT);
  pinMode(BUTTON, INPUT);  
  pinMode(IR_IN, INPUT);  

  Serial.begin(9600);

  lastShotFired = 0;
  settings.autoFireAllowed = true; // Not implemented
  settings.fireAllowed = true;
  settings.fireDelay = 500;
  
  Timer1.initialize(480);
  Timer1.attachInterrupt(irHandler);
  
  // Setup XBee and send hello
  xbee.begin(9600);
  sendStatus( XBeeAddress64(0x00000000, 0x0000FFFF) );
  
  teamColor[0] = 0;
  teamColor[1] = 150;
  teamColor[2] = 0;
}


void loop() {
  unsigned int loopStartTime = millis();

  if (digitalRead(BUTTON)) {
    if (settings.fireAllowed && (loopStartTime - lastShotFired > settings.fireDelay) ) {
      lastShotFired = loopStartTime;
        settings.fireAllowed = settings.autoFireAllowed;
      fire();
    }
  } 
  else {
    settings.fireAllowed = true;
  }
  
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
        analogWrite(VIBRATE,0);
      }
    } 

    // Check battery every second
    if ( millis() > nextBatteryCheck ) {
      nextBatteryCheck = millis() + 10000;
      battery = analogRead(0);

      uint8_t payload[] = { 
        0x02,
        battery > 8 && 0xFF, 
        battery & 0xFF, 
      };
      XBeeAddress64 addr64 = XBeeAddress64(0x00000000, 0x0000FFFF);
      ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
      xbee.send(zbTx);

    }
    
    /*if ( irTxDelay > 0 && millis() > irTxDelay ) {
      hasMessage = true;
    }*/
}

void fire() {
  transmitBuffer = prepareMessage(playerId);
  //transmitBuffer = { 0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87 };
  hasMessage = true;
  
  soundDelay = 100;
  nextSoundSwitch = 0;

  isDeadUntil = millis() + 3000;
  isSafeUntil = millis() + 6000;
  currentColor[0] = 255;
  currentColor[1] = 255;
  currentColor[2] = 255;
  //analogWrite(VIBRATE,255);
  
  sendShoot( XBeeAddress64(0x00000000, 0x0000FFFF) );
}

/*
 * Start bit + message + checksum
 */
int prepareMessage(byte message) {
  return 1024 + (message << 2) + ((255 - message) % 4);
}

// @todo Append message to transmit buffer for queue ability?
void irHandler() {
  /*if(irState == rx) {
    receive();
  } else*/ if (irState == tx || hasMessage) {
    irState = tx;
    transmit();
  //} else if (digitalRead(IR_IN)) {
  //  irState = rx;
  }
}

void transmit() {
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







void parseCommand( ) {
  switch(rx64.getData(1)) {
    case 0x01: // Startup package
      // Green
      targetColor[0] = 0;
      targetColor[1] = 255;
      targetColor[2] = 0;
      isOnline = true;
      sendStatus( rx64.getRemoteAddress64() );
      break;
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
}

void sendStatus( XBeeAddress64 addr64 ) {
  uint8_t payload[] = { 
    0x01, // WhoAmI cmd
    0x00, // Standard weapon
    (isOnline * 0x01), // Status bits
    shotsLeft,
    lifeLeft,
    teamColor[0],
    teamColor[1],
    teamColor[2],
  };
  
  // Specify the address of the remote XBee (this is the SH + SL)
  ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
  xbee.send(zbTx);
}

void sendShoot( XBeeAddress64 addr64 ) {
  uint8_t payload[] = { 
    0x01, // WhoAmI cmd
    0x00,
    0x01,
    0x00,
    0x00,
  };
  
  // Specify the address of the remote XBee (this is the SH + SL)
  ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
  xbee.send(zbTx);
}

void sendUnknownCommand( XBeeAddress64 addr64 ) {
  uint8_t payload[] = { 
    0x10, // Unknown command
    rx64.getData(0), 
    rx64.getData(1), 
    rx64.getData(2), 
    rx64.getData(3), 
    rx64.getData(4) 
  };
  ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
  xbee.send(zbTx);
}

void fadeRgb() {
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
        analogWrite(RED, currentColor[0]*0.2);
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
}
