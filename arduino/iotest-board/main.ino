#include <XBee.h>
#include <TimerOne.h>
//#include "../lib/Xbee/XBee.h"
//#include "../lib/Xbee/TimerOne.h"
#include "main.h"


XBee xbee = XBee();

XBeeResponse response = XBeeResponse();
Rx64Response rx64 = Rx64Response();

using namespace collision;



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
            break;
        case FIRE_EVENT:
            break;
        case DIE_EVENT:
            break;
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
    (eventId >> 8) & 0xFF,
    eventId & 0xFF,
    (eventData >> 8) & 0xFF,
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
  // Setup XBee and send hello
  Serial.begin(9600);
  xbee.setSerial(Serial);

  triggerEventRemote(0x00000000, 0x0000FFFF, 0x0000, 0x1234 );
  //sendStatus( XBeeAddress64(0x00000000, 0x0000FFFF) );

  pinMode(1,INPUT);
  pinMode(2,INPUT);
  pinMode(3,INPUT);
  pinMode(4,INPUT);
  pinMode(5,INPUT);
  pinMode(6,INPUT);
  pinMode(7,INPUT);
  pinMode(8,INPUT);
  pinMode(9,INPUT);
  pinMode(10,INPUT);
  pinMode(11,INPUT);
  pinMode(12,INPUT);
  pinMode(13,INPUT);

}/*}}}*/

bool state1;
bool state2;
bool state3;
bool state4;
bool state5;
bool state6;
bool state7;
bool state8;
bool state9;
bool state10;
bool state11;
bool state12;
bool state13;

bool prevState1;
bool prevState2;
bool prevState3;
bool prevState4;
bool prevState5;
bool prevState6;
bool prevState7;
bool prevState8;
bool prevState9;
bool prevState10;
bool prevState11;
bool prevState12;
bool prevState13;

void loop() {/*{{{*/
   // unsigned int loopStartTime = millis();

    xbee.readPacket();
    if (xbee.getResponse().isAvailable()) {
        // got something
        if (xbee.getResponse().getApiId() == 0x90) {
            // got a rx packet
            xbee.getResponse().getRx64Response(rx64);
            parseCommand();
        }
    }

    state1 = digitalRead(1);
    state2 = digitalRead(2);
    state3 = digitalRead(3);
    state4 = digitalRead(4);
    state5 = digitalRead(5);
    state6 = digitalRead(6);
    state7 = digitalRead(7);
    state8 = digitalRead(8);
    state9 = digitalRead(9);
    state10 = digitalRead(10);
    state11 = digitalRead(11);
    state12 = digitalRead(12);
    state13 = digitalRead(13);

    /*if (state1 != prevState1) {
        prevState1 = state1;
        triggerEventRemote(0x00000000, 0x0000FFFF, 0x0001, 0x1234 );
    }


    if (state3 != prevState3) {
        prevState3 = state3;
        triggerEventRemote(0x00000000, 0x0000FFFF, 0x0003, 0x1234 );
    }

    if (state4 != prevState4) {
        prevState4 = state4;
        triggerEventRemote(0x00000000, 0x0000FFFF, 0x0004, 0x1234 );
    }

    if (state5 != prevState5) {
        prevState5 = state5;
        triggerEventRemote(0x00000000, 0x0000FFFF, 0x0005, 0x1234 );
    }

    if (state6 != prevState6) {
        prevState6 = state6;
        triggerEventRemote(0x00000000, 0x0000FFFF, 0x0006, 0x1234 );
    }

    if (state7 != prevState7) {
        prevState7 = state7;
        triggerEventRemote(0x00000000, 0x0000FFFF, 0x0007, 0x1234 );
    }

    if (state8 != prevState8) {
        prevState8 = state8;
        triggerEventRemote(0x00000000, 0x0000FFFF, 0x0008, 0x1234 );
    }

    if (state9 != prevState9) {
        prevState9 = state9;
        triggerEventRemote(0x00000000, 0x0000FFFF, 0x0009, 0x1234 );
    }

    if (state10 != prevState10) {
        prevState10 = state10;
        triggerEventRemote(0x00000000, 0x0000FFFF, 0x000A, 0x1234 );
    }

    if (state11 != prevState11) {
        prevState11 = state11;
        triggerEventRemote(0x00000000, 0x0000FFFF, 0x000B, 0x1234 );
    }


    if (state13 != prevState13) {
        prevState13 = state13;
        triggerEventRemote(0x00000000, 0x0000FFFF, 0x000D, 0x1234 );
    }*/


    if (state2 != prevState2) {
        prevState2 = state2;
        if (state2) {
            triggerEventRemote(0x00000000, 0x0000FFFF, 0x0002, 0x1234 );
        }
    }

    if (state12 != prevState12) {
        prevState12 = state12;
        if (state12) {
            triggerEventRemote(0x00000000, 0x0000FFFF, 0x000C, 0x1234 );
        }
    }
}/*}}}*/


void parseCommand( ) {/*{{{*/
  switch(rx64.getData(1)) {
    case 0x05: // Remote event
      triggerEvent(rx64.getData(2)<<8+rx64.getData(3),rx64.getData(4)<<8+rx64.getData(5));
      break;
    case 0xff: // Coordinator/Gameserver went offline
      break;
    default:
      //sendUnknownCommand( rx64.getRemoteAddress64() );
      break;
  }
}/*}}}*/


void sendStatus( XBeeAddress64 addr64 ) {/*{{{*/
  uint8_t payload[] = {
    0x00, // WhoAmI cmd
    0x01 // Standard weapon
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

