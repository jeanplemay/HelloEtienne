// Juan Manuel Gallego – galj1704 et Jean-Philippe Lemay – lemj0601

// TrameManager.h
#ifndef TrameManager_h
#define TrameManager_h

#include <math.h>

using namespace std;

#define CRC16 0x8005


class TrameManager {
    public:
        const static int nbBytesMax = 80;

        int* getTrame(uint8_t* message, uint8_t size) {
            Serial.println("TEST 2 **************");
            for (int i = 0; i < size; i++) {
                Serial.print(message[i]);
            }

            uint8_t flag = 0b00000000;
            uint8_t buffer[nbBytesMax] = {};
            static int bitsBuffer[nbBytesMax*8] = {};

            if(size > nbBytesMax-4) {
                return 0;
            }
            buffer[0] = flag;
            buffer[1] = size;
            
            for(int i = 0; i < size; i++) {
                buffer[i+2] = message[i];
            }

            uint16_t crc16 = gen_crc16(message, size);
            buffer[size + 2] = (crc16 & 0xFF00) >> 8;
            buffer[size + 3] = crc16 & 0x00FF;

            for(int i = 0; i < size + 4; i++) {
                int mask = 0b10000000;
                for(int j = 0; j < 8; j++) {
                    bitsBuffer[j + 8*i] = buffer[i] & mask;
                    mask = mask >> 1;
                }
            }

            return bitsBuffer;
        }

        uint8_t* getMessageBytes(int* trame, int size) {
            uint8_t buffer[nbBytesMax] = {};
            static uint8_t message[nbBytesMax-4] = {};

            for(int i = 0; i < size/8; i++) {
                buffer[i] = 0;
                for(int j = 0; j < 8; j++) {
                    if (trame[j + 8*i] == 1) {
                        buffer[i] += pow(2, (7-j));
                    }
                }
            }

            for(int i = 0; i < (size/8)-4; i++) {
                message[i] = buffer[i+2];
                Serial.println(message[i]);
            }

            uint16_t crc16Calculated = gen_crc16(message, (size/8)-4);
            uint16_t crc16Received =  (buffer[(size/8)-2] << 8) | (buffer[(size/8)-1]);

            Serial.println(crc16Calculated);
            //Serial.println(crc16Received);

            if(crc16Calculated != crc16Received) {
                Serial.println("CRC16 ERROR");
                return message;
            } else {
                return message;
            }
        }
        
    private:
        uint16_t gen_crc16(const uint8_t *data, uint8_t size)
        {
            uint16_t out = 0;
            int bits_read = 0, bit_flag;

            /* Sanity check: */
            if(data == NULL)
                return 0;

            while(size > 0)
            {
                bit_flag = out >> 15;

                /* Get next bit: */
                out <<= 1;
                out |= (*data >> bits_read) & 1; // item a) work from the least significant bits

                /* Increment bit counter: */
                bits_read++;
                if(bits_read > 7)
                {
                    bits_read = 0;
                    data++;
                    size--;
                }

                /* Cycle check: */
                if(bit_flag)
                    out ^= CRC16;

            }

            // item b) "push out" the last 16 bits
            int i;
            for (i = 0; i < 16; ++i) {
                bit_flag = out >> 15;
                out <<= 1;
                if(bit_flag)
                    out ^= CRC16;
            }

            // item c) reverse the bits
            uint16_t crc = 0;
            i = 0x8000;
            int j = 0x0001;
            for (; i != 0; i >>=1, j <<= 1) {
                if (i & out) crc |= j;
            }

            return crc;
        }
};

#endif