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
        BUTTON = 9,
        VIBRATE = 8,
        SPEAKER = 8,
        RED = 3,
        GREEN = 6,
        BLUE = 5,
        IROUT = 19,
    };

    enum events {
        IR_EVENT = 1,
        FIRE_EVENT = 2,
        DIE_EVENT = 3,
    };

    class event {
        public:
            event();
            unsigned int id; // Trigger event id
            unsigned int delay;
            unsigned int addr_low;
            unsigned int addr_high;
            unsigned int event_id; // Send event id
            bool targetSelf();
    };

    event::event() {
        id = 0;
        delay = 0;
        addr_low = 0;
        addr_high = 0;
        event_id = 0;
    }

    bool event::targetSelf() {
        return (addr_low == 0 && addr_high == 0);
    }
}
