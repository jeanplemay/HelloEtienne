#include "Particle.h"

#include "TrameManager.h"
#include "MessagesConverter.h"

SYSTEM_THREAD(ENABLED);

void threadFunction(void *param);

Thread thread("testThread", threadFunction);

TrameManager trameManager;
MessagesConverter messagesConverter;

volatile int counter = 0;
unsigned long lastTime = 0;
system_tick_t lastThreadTime = 0;

int PIN_LED = D7;
int PIN_IN = D2;
int PIN_OUT = D3;

float CLOCK_PERIOD = 100;

enum States {
    init,
    start,
    output0,
    output1,
    ignore_low,
    ignore_high,
    end_state,
    error
};

States currentState = init;
States nextState = init;

enum TimePeriod {
    shortPeriod, 
    mediumPeriod,
    longPeriod,
    veryLongPeriod
};

int startEndBits[] = {0, 1, 1, 1, 1, 1, 1, 0};

int buffer[640] = {};
int bufferSize = 0;

void setup() {
	Serial.begin(9600);
    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_IN, INPUT);
    pinMode(PIN_OUT, OUTPUT);
    waitFor(Serial.isConnected, 30000);

    attachInterrupt(PIN_IN, manchesterInput, CHANGE);
    lastTime = millis();
}

void loop() {
}


void threadFunction(void *param) {
    int bitsSize;

    char message[] = "Hello world";
    int charSize = 11;
    uint8_t* messageBytes = messagesConverter.getBytes(message, charSize);
    int *bits = trameManager.getTrame(messageBytes, charSize);
    bitsSize = (charSize+4)*8;

    int i = 0;
	while(true) {
    
        digitalWrite(PIN_LED, bits[i]);

        if(i < 8) {
            digitalWrite(PIN_OUT, startEndBits[i++]);
            os_thread_delay_until(&lastThreadTime, CLOCK_PERIOD);
        } else if (i >= 8 + bitsSize) {
            digitalWrite(PIN_OUT, startEndBits[i++ - 8 - bitsSize]);
            os_thread_delay_until(&lastThreadTime, CLOCK_PERIOD);
        }
        else if(bits[i++ - 8] == 0) {
            digitalWrite(PIN_OUT, LOW);
            os_thread_delay_until(&lastThreadTime, CLOCK_PERIOD / 2);
            digitalWrite(PIN_OUT, HIGH);
            os_thread_delay_until(&lastThreadTime, CLOCK_PERIOD / 2);
        } else {
            digitalWrite(PIN_OUT, HIGH);
            os_thread_delay_until(&lastThreadTime, CLOCK_PERIOD / 2);
            digitalWrite(PIN_OUT, LOW);
            os_thread_delay_until(&lastThreadTime, CLOCK_PERIOD / 2);
        }

        if(i > 15 + bitsSize) i = 0;
	}
	// You must not return from the thread function
}

void decodeInput() {
    uint8_t* messageBytesReceived = trameManager.getMessageBytes(buffer, bufferSize);
    char* messageReceived = messagesConverter.getChars(messageBytesReceived, bufferSize/8);
    Serial.printlnf("Message : %s", messageReceived);
}

void setNextState(TimePeriod timePeriod) {
    switch (currentState) {
        case init:
            Serial.println("CURRENT STATE : init");
            if (timePeriod == veryLongPeriod) {
                nextState = start;
            }
            break;
        case start:
            Serial.println("CURRENT STATE : start");
            if (timePeriod == mediumPeriod) {
                nextState = ignore_high;
            } else if (timePeriod == longPeriod) {
                nextState = output0;
            }
            break;
        case ignore_high:
            Serial.println("CURRENT STATE : ignore_high");
            if (timePeriod == shortPeriod) {
                nextState = output1;
            } else {
                nextState = error;
            }
            break;
        case ignore_low:
            Serial.println("CURRENT STATE : ignore_low");
            if (timePeriod == shortPeriod) {
                nextState = output0;
            } else if (timePeriod == mediumPeriod) {
                nextState = end_state;
            } else {
                nextState = error;
            }
            break;
        case output0:
            Serial.println("CURRENT STATE : output0:");
            if (timePeriod == shortPeriod) {
                nextState = ignore_low;
            } else if (timePeriod == mediumPeriod) {
                nextState = output1;
            } else {
                nextState = error;
            }
            break;
        case output1:
            Serial.println("CURRENT STATE : output1");
            if (timePeriod == shortPeriod) {
                nextState = ignore_high;
            } else if (timePeriod == mediumPeriod) {
                nextState = output0;
            } else if (timePeriod == longPeriod) {
                nextState = end_state;
            } else {
                nextState = error;
            }
            break;
        case end_state:
            Serial.println("CURRENT STATE : end_state");
            if(timePeriod == veryLongPeriod) {
                nextState = init;
            } else {
                nextState = error;
            }
            break;
        case error:
            Serial.println("CURRENT STATE : error");
            nextState = init;
            break;
    }
}

void setOutput() {
    switch (currentState) {
        case start:
            bufferSize = 0; 
            break;
        case output0:
            Serial.println("0");
            buffer[bufferSize++] = 0;
            break;
        case output1:
            Serial.println("1");
            buffer[bufferSize++] = 1;
            break;
        case end_state:
            for (int i = 0; i < bufferSize; i++) {
                Serial.print(buffer[i]);
            }
            decodeInput();
            break;
        case error:
            Serial.println("ERROR");
            break;
        default:
            break;
    }
}

void manchesterInput() {
    int time = millis() - lastTime;
    Serial.printlnf("time=%d",time);
    lastTime = millis();

    TimePeriod timePeriod;
    
    if(time > 1.9 * CLOCK_PERIOD) {
        timePeriod = veryLongPeriod;
    } else if( time <= 1.9 * CLOCK_PERIOD && time > 1.3 * CLOCK_PERIOD) {
        timePeriod = longPeriod;
    } else if( time <= 1.3 * CLOCK_PERIOD && time > 0.8 * CLOCK_PERIOD) {
        timePeriod = mediumPeriod;
    } else {
        timePeriod = shortPeriod;
    }
    setNextState(timePeriod);
    currentState = nextState;
    setOutput();
}