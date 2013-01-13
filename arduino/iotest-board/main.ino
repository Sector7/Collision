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
void triggerEvent(unsigned int eventId, unsigned int eventData) {/*{{{*/
    switch (eventId) {
        case 0:
            break;
        case SET_EVENT:
            digitalWrite(eventData & 0xFF,!digitalRead(eventData & 0xFF));
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
}/*}}}*/

void triggerEventRemote(unsigned int addr_low, unsigned int addr_high, unsigned int eventId, unsigned int eventData) {/*{{{*/
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
}/*}}}*/


void setup() {/*{{{*/
  // Setup XBee and send hello
  Serial.begin(9600);
  xbee.setSerial(Serial);

  //sendIndentify( XBeeAddress64(0x00000000, 0x0000FFFF), 0x0002 );

  //triggerEventRemote(0x00000000, 0x0000FFFF, 0x0000, 0x1234 );

  pinMode(2,INPUT); // Fire knapp
  pinMode(3,OUTPUT);
  pinMode(4,OUTPUT);
  pinMode(5,OUTPUT);
  pinMode(6,OUTPUT);
  pinMode(7,OUTPUT);
  pinMode(8,OUTPUT);
  pinMode(9,OUTPUT);
  pinMode(10,OUTPUT);
  pinMode(11,OUTPUT);
  pinMode(12,INPUT); // Extra
  pinMode(13,OUTPUT);

}/*}}}*/

bool state2;
bool state12;

bool prevState2;
bool prevState12;

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

    state2 = digitalRead(2);
    state12 = digitalRead(12);

    if (state2 != prevState2) {
        prevState2 = state2;
        if (!state2) {
            triggerEventRemote(0x00000000, 0x0000FFFF, 0x0002, 0x0002 );
        }
    }

    if (state12 != prevState12) {
        prevState12 = state12;
        if (!state12) {
            triggerEventRemote(0x00000000, 0x0000FFFF, 0x0002, 0x000C );
        }
    }
}/*}}}*/


void parseCommand( ) {/*{{{*/
  switch(rx64.getData(1)) {
    case 0x00: // Identify
      sendIndentify( rx64.getRemoteAddress64(), 0x0002 );
      break;
    case 0x01: // Remote event
      triggerEvent(
        (rx64.getData(2)<<8)+rx64.getData(3),
        (rx64.getData(4)<<8)+rx64.getData(5)
      );
      break;
    case 0xff: // Coordinator/Gameserver went offline
      break;
    default:
      //sendUnknownCommand( rx64.getRemoteAddress64() );
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

