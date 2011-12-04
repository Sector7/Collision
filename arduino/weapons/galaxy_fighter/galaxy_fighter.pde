#include <Xbee.h>

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
  pinMode(8, OUTPUT);
  digitalWrite(8,false);
  pinMode(6, OUTPUT);
  digitalWrite(6,false);
  pinMode(12, OUTPUT);
  digitalWrite(12,false);
  xbee.begin(9600);
  
  analogWrite(9, 255);    
  analogWrite(10, 0);
  analogWrite(11, 0);
  //delay(5000);
  sendStatus( XBeeAddress64(0x00000000, 0x0000FFFF) );
  
  
  teamColor[0] = 0;
  teamColor[1] = 100;
  teamColor[2] = 100;
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
      digitalWrite(8,soundState);
      if (soundDelay<1000) {
        soundDelay += 2;
        if (soundDelay > 500) {
          analogWrite(6,80);
        }
      } else {
        soundDelay = 0;
        digitalWrite(8,false);
        analogWrite(6,0);
      }
    } 
       
    if(digitalRead(4) and soundDelay == 0){
      soundDelay = 100;
      nextSoundSwitch = 0;
      isDeadUntil = millis() + 3000;
      isSafeUntil = millis() + 6000;
      currentColor[0] = 255;
      currentColor[1] = 255;
      currentColor[2] = 255;
      analogWrite(6,255);
    }
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
      tone(8, 440, 500);
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
