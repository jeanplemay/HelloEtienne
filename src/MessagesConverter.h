// Juan Manuel Gallego – galj1704 et Jean-Philippe Lemay – lemj0601

// MessagesConverter.h
#ifndef MessagesConverter_h
#define MessagesConverter_h

using namespace std;

class MessagesConverter {
    public:
        const static int nbBytesMax = 76;
        
        uint8_t* getBytes(char* message, uint8_t size) {
            static uint8_t buffer[nbBytesMax] = {};
            for(int i = 0; i < size; i++) {
                buffer[i] = message[i];
            }
            return buffer;
        }

        char* getChars(uint8_t* message, uint8_t size) {
            static char buffer[nbBytesMax] = {};
            for(int i = 0; i < size; i++) {
                buffer[i] = message[i];
            }
            return buffer;
        }
};

#endif