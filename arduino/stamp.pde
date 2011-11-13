#include <XBee.h> 

XBee xbee = XBee();

XBeeResponse response = XBeeResponse();
Rx64Response rx64 = Rx64Response();

uint8_t data = 0;

boolean isOnline = false;
byte targetColor[3];
byte teamColor[3];
byte currentColor[3];
unsigned long nextRgbCall;
unsigned long nextSoundSwitch;
unsigned long soundDelay;
boolean soundState;

// Game parameters
unsigned long isDeadUntil;
unsigned long isSafeUntil;
unsigned int  shotsLeft;
unsigned int  lifeLeft;

void setup()
{
  Serial.begin(9600);
  pinMode(4, OUTPUT);
  digitalWrite(4,true);
  xbee.begin(9600);
  
  analogWrite(9, 255);    
  analogWrite(10, 0);
  analogWrite(11, 0);
  delay(5000);
  sendHello();
}

void loop()
{
    xbee.readPacket();
    if (xbee.getResponse().isAvailable()) {
      // got something
      if (xbee.getResponse().getApiId() == 0x90) {
        // got a rx packet
        xbee.getResponse().getRx64Response(rx64);
        parseCommand();
      }
    }
    
    // Fade RGB each 10 ms
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
}

void parseCommand() {
  switch(rx64.getData(1)) {
    case 0x01: // Startup package
      // Green
      targetColor[0] = 0;
      targetColor[1] = 255;
      targetColor[2] = 0;
      isOnline = true;
      break;
    case 0x02: // Set rgb color
      teamColor[0] = rx64.getData(2);
      teamColor[1] = rx64.getData(3);
      teamColor[2] = rx64.getData(4);
      break;
    case 0x03:
      soundDelay = 100;
      isDeadUntil = millis() + 3000;
      isSafeUntil = millis() + 6000;
      currentColor[0] = 255;
      currentColor[1] = 255;
      currentColor[2] = 255;
      break;
    case 0xff: // Startup package
      isOnline = false;
      currentColor[0] = 255;
      currentColor[1] = 255;
      currentColor[2] = 0;
      break;
    default:
      sendUnknownCommand();
      break;
  }
}

void sendHello() {
  uint8_t payload[] = { 'h','e','l','l','o' };
  // Specify the address of the remote XBee (this is the SH + SL)
  XBeeAddress64 addr64 = XBeeAddress64(0x00000000, 0x0000FFFF);
  // Create a TX Request
  ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
  xbee.send(zbTx);
}

void sendUnknownCommand() {
  uint8_t payload[] = { 0 };
  XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x4069839a);
  // Specify the address of the remote XBee (this is the SH + SL)
  // Create a TX Request
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
