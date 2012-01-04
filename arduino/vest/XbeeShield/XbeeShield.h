void irHandler();
void sendStatus(XBeeAddress64);
void fire();
int prepareMessage(byte);
void receive();
void transmit();
void parseCommand();
void fadeRgb();
void triggerEvent(unsigned int,unsigned  int);
void triggerEventRemote(unsigned int, unsigned int, unsigned int, unsigned int);

void sendShoot(XBeeAddress64 addr64);
void sendUnknownCommand(XBeeAddress64 addr64);

void startIrRecv();

const unsigned int BROADCAST_LOW = 0x00000000;
const unsigned int BROADCAST_HIGH = 0x0000FFFF;

namespace collision {
    enum ir_state {
        neutral = 0,
        tx = 1,
        rx = 2,
    };

    enum pins {
        IR_IN = 2,
        BUTTON = 11,
        VIBRATE = 6,
        SPEAKER = 8,
        RED = 3,
        GREEN = 6,
        BLUE = 5,
        IROUT = 19,
    };

    enum events {
        IR_EVENT = 1,
    };

    class event {
        public:
            unsigned int id;
            unsigned int delay;
            unsigned int addr_low;
            unsigned int addr_high;
            unsigned int event;
            bool targetSelf();
    };

    bool event::targetSelf() {
        return (addr_low == 0 && addr_high == 0);
    }
}
