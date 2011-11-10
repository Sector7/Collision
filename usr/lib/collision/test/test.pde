#include <XBee.h> 

int brightness = 0;    // how bright the LED is
int fadeAmount = 5;    // how many points to fade the LED by

XBee xbee = XBee();
XBeeResponse response = XBeeResponse();
Rx64Response rx64 = Rx64Response();
uint8_t data = 0;
void setup()
{
  Serial.begin(9600);
  pinMode(11, OUTPUT);
  xbee.begin(9600);
  analogWrite(11, 128);    
}

void loop()
{
    xbee.readPacket();
    
    if (xbee.getResponse().isAvailable()) {
      // got something
     
      
      
      
      if (xbee.getResponse().getApiId() == 0x90) {
        // got a rx packet
        xbee.getResponse().getRx64Response(rx64);
        data = rx64.getData(1);
        analogWrite(9, data);
        data = rx64.getData(2);
        analogWrite(10, data);
        data = rx64.getData(3);
        analogWrite(11, data);
        
        uint8_t payload[] = {  rx64.getData(0), rx64.getData(1), rx64.getData(2), rx64.getDataLength(),'l','l','o' };
        // Specify the address of the remote XBee (this is the SH + SL)
        XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x4069839a);
        // Create a TX Request
        ZBTxRequest zbTx = ZBTxRequest(addr64, payload, sizeof(payload));
        xbee.send(zbTx);

      }
    }
       
}
