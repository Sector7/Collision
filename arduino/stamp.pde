#include <XBee.h> 

XBee xbee = XBee();

XBeeResponse response = XBeeResponse();
Rx64Response rx = Rx64Response();

uint8_t data = 0;

boolean isOnline = true;
byte targetColor[3];
byte teamColor[3];
byte currentColor[3];
unsigned long nextRgbCall;
unsigned long nextSoundSwitch;
unsigned long soundDelay;
boolean soundState;

unsigned long irdiff;
unsigned long timestamp;
unsigned long prevTimestamp;
unsigned long pair[2];
boolean irstate = false;

int irpointer = 0;
boolean irdata[33];
boolean validIrCode = false;

// Game parameters
unsigned long isDeadUntil;
unsigned long isSafeUntil;
unsigned int  shotsLeft;
unsigned int  lifeLeft;

void setup()
{
  Serial.begin(9600);
  pinMode(4, OUTPUT);
  digitalWrite(4,false);
  xbee.begin(9600);
  
  analogWrite(9, 255);    
  analogWrite(10, 0);
  analogWrite(11, 0);
  //delay(5000);
  sendStatus( XBeeAddress64(0x00000000, 0x0000FFFF) );
  
  attachInterrupt(0, ir_check, CHANGE);
}

void ir_check(){
  timestamp = micros();
  irdiff = timestamp - prevTimestamp;
  prevTimestamp = timestamp;
  
  if ( irstate ) {
    if ( pair[0] == 0 ) {
      pair[0] = irdiff;
    } else {
      pair[1] = irdiff;
      if( pair[0] < 800 && irdiff < 800 ) {
        irdata[irpointer] = false;      
      } else if( pair[0] < 800 && irdiff > 800 ) {
        irdata[irpointer] = true;
      } else {
        irstate = false;
      }
      irpointer++;
      pair[0] = 0;
        
      if (irpointer == 33) {
        validIrCode = true;
      }        
    }
  } else if ( irdiff > 8000 && irdiff < 10000 ) {
    pair[0] = irdiff;
  } else if ( irdiff > 4000 && irdiff < 10000 && pair[0] > 0 ) {
    pair[0] = 0;
    irstate = true;
    irpointer = 0;
  } else {
    pair[0] = 0;
  }
}

void loop()
{      
    xbee.readPacket();
    if (xbee.getResponse().isAvailable()) {
      // got something
      if (xbee.getResponse().getApiId() == 0x90) {
        // got a rx packet
        xbee.getResponse().getRx64Response(rx);
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
      digitalWrite(4,soundState);
      if (soundDelay<1000) {
        soundDelay += 2;
      } else {
        soundDelay = 0;
        digitalWrite(4,false);
      }
    } 
    
    
    if(digitalRead(12)){
        teamColor[0]=0; 
        teamColor[1]=0; 
        teamColor[2]=255; 
        soundDelay = 100;
        nextSoundSwitch = 0;
    }
    if(digitalRead(13)){
        teamColor[0]=0; 
        teamColor[1]=255; 
        teamColor[2]=0; 
        soundDelay = 100;
        nextSoundSwitch = 0;
    }
    
    if(digitalRead(7)){
        teamColor[0]=255; 
        teamColor[1]=0; 
        teamColor[2]=0; 
        soundDelay = 100;
        nextSoundSwitch = 0;
    }
    
    if ( validIrCode ) {
      decodeIR();
    }
}

void decodeIR() {
  int x=0;
  unsigned long manufacture = 0;
  unsigned long code = 0;
  int val = 1;

  for(x=0;x<33;x++) {
    if ( x < 15 ) {
      manufacture += val * irdata[x];
    } else {
      code += val * irdata[x];
    }
    
    if ( x == 14 ) {
      val = 1;
    } else {
      val *= 2;
    }
  }
  validIrCode = false;
  
  if ( manufacture == 1402 && code == 4294953525 ) {
    soundDelay = 100;
    nextSoundSwitch = 0;
  }
  
  sendIR( manufacture, code );
}

void parseCommand( ) {
  switch(rx.getData(1)) {
    case 0x01: // Startup package
      // Green
      targetColor[0] = 0;
      targetColor[1] = 255;
      targetColor[2] = 0;
      isOnline = true;
      sendStatus( rx.getRemoteAddress64() );
      break;
    case 0x02: // Set rgb color
      teamColor[0] = rx.getData(2);
      teamColor[1] = rx.getData(3);
      teamColor[2] = rx.getData(4);
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
      sendStatus( rx.getRemoteAddress64() );
      break;
    case 0xff: // Coordinator/Gameserver went offline
      tone(4, 440, 500);
      isOnline = false;
      currentColor[0] = 255;
      currentColor[1] = 255;
      currentColor[2] = 0;
      break;
    default:
      sendUnknownCommand( rx.getRemoteAddress64() );
      break;
  }
}

void sendStatus( XBeeAddress64 addr64 ) {
  uint8_t payload[] = { 
    0x01, // WhoAmI cmd
    0x01, // Standard weapon
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

void sendUnknownCommand( XBeeAddress64 addr64 ) {
  uint8_t payload[] = { 
    0x00, // Unknown command
    rx.getData(0), 
    rx.getData(1), 
    rx.getData(2), 
    rx.getData(3), 
    rx.getData(4) 
  };
  ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
  xbee.send(zbTx);
}

void sendIR( unsigned long manufacture,unsigned long code ) {

  uint8_t payload[] = { 
    0x02, // IR command
    manufacture, 
    code
  };
  
  XBeeAddress64 addr64 = XBeeAddress64(0x00000000, 0x0000FFFF);
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
        analogWrite(9, currentColor[0]);
      }
      if ( currentColor[1] != targetColor[1] ) {
        if ( currentColor[1]<targetColor[1] ) {
          currentColor[1]++;
        } else {
          currentColor[1]--;        
        }
        analogWrite(10, currentColor[1]);
      }
      if ( currentColor[2] != targetColor[2] ) {
        if ( currentColor[2]<targetColor[2] ) {
          currentColor[2]++;
        } else {
          currentColor[2]--;        
        }
        analogWrite(11, currentColor[2]);
      }
    }
  } else {
    currentColor[0] += 3;
    currentColor[1] += 3;
    currentColor[2] = 0;
    analogWrite(9, currentColor[0]);    
    analogWrite(10, currentColor[1]);
    analogWrite(11, currentColor[2]);
  }
}
