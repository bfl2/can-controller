#ifndef __CRC_H__
#define __CRC_H__

#include<stdint.h>

typedef struct {
    unsigned int BLANK      : 5;
    unsigned int SOF        : 1;
    unsigned int ID_A       : 11;
    unsigned int RTR        : 1;
    unsigned int IDE        : 1;
    unsigned int RESERVED   : 1;
    unsigned int DLC        : 4;
    unsigned int B7         : 8;
    unsigned int B6         : 8;
    unsigned int B5         : 8;
    unsigned int B4         : 8;
    unsigned int B3         : 8;
    unsigned int B2         : 8;
    unsigned int B1         : 8;
    unsigned int B0         : 8;
} Payload_standard;

typedef struct {
    unsigned int BLANK      : 1;
    unsigned int SOF        : 1;
    unsigned int ID_A       : 11;
    unsigned int SRR        : 1;
    unsigned int IDE        : 1;
    unsigned int ID_B       : 18;
    unsigned int RTR        : 1;
    unsigned int RESERVED   : 2;
    unsigned int DLC        : 4;
    unsigned int B7         : 8;
    unsigned int B6         : 8;
    unsigned int B5         : 8;
    unsigned int B4         : 8;
    unsigned int B3         : 8;
    unsigned int B2         : 8;
    unsigned int B1         : 8;
    unsigned int B0         : 8;
} Payload_extended;

union seed_standard{
    Payload_standard p;
    char b[11];
};

union seed_extended{
    Payload_extended p;
    char b[13];
};

uint16_t CrcNext(uint16_t crc, uint8_t data, int8_t skip);
uint16_t calculateCRC(uint16_t crc, uint8_t bit);
int32_t ReverseBits(int32_t num, int8_t bits_size);

#endif