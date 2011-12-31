void irHandler();
void sendStatus(XBeeAddress64);
void fire();
int prepareMessage(byte);
void receive();
void transmit();
void parseCommand();
void fadeRgb();
void triggerEventRemote(int, int, int);

void sendShoot(XBeeAddress64 addr64);
void sendUnknownCommand(XBeeAddress64 addr64);

void startIrRecv();


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
