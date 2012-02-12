XBee xbee = XBee();

XBeeResponse response = XBeeResponse();
Rx64Response rx64 = Rx64Response();

/* Stamp vars*/
boolean isOnline = true;
byte targetColor[3];
byte teamColor[3];
byte currentColor[3];
unsigned long nextRgbCall;
unsigned long nextSoundSwitch;
unsigned long nextBatteryCheck;
int battery;
unsigned long soundDelay;
boolean soundState;

/* Leths flags */
int fireFlag = false;
unsigned int lastShotFired;
const int numBitsUsedInMessage = 11;

// Game parameters
unsigned long isDeadUntil;
unsigned long isSafeUntil;
unsigned int  shotsLeft;
unsigned int  lifeLeft;
byte playerId = 0x55;

enum ir_state {
  neutral = 0,
  tx = 1,
  rx = 2,
};

enum pins {
  IR_IN = 2,
  BUTTON = 4,
  VIBRATE = 6,
  SPEAKER = 8,
  RED = 11,
  GREEN = 10,
  BLUE = 9,
  IROUT = 12,
};

//IR related globals
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



void setup();
void loop();
void fire();
int prepareMessage(byte message);
void irHandler();
void transmit();
void receive();
void parseCommand( );
void sendStatus( XBeeAddress64 addr64 );
void sendShoot( XBeeAddress64 addr64 );
void sendUnknownCommand( XBeeAddress64 addr64 );
void fadeRgb();
