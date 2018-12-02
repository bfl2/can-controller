#ifndef __CRC_H__
#define __CRC_H__

#include<stdint.h>

typedef struct {
    uint64_t BLANK      : 5;
    uint64_t SOF        : 1;
    uint64_t ID_A       : 11;
    uint64_t RTR        : 1;
    uint64_t IDE        : 1;
    uint64_t RESERVED   : 1;
    uint64_t DLC        : 4;
} Header_standard;

typedef struct {
    uint64_t BLANK      : 1;
    uint64_t SOF        : 1;
    uint64_t ID_A       : 11;
    uint64_t SRR        : 1;
    uint64_t IDE        : 1;
    uint64_t ID_B       : 18;
    uint64_t RTR        : 1;
    uint64_t RESERVED   : 2;
    uint64_t DLC        : 4;
} Header_extended;

typedef struct {

    Header_standard h;
    uint8_t d;

} Payload_standard;

typedef struct {

    Header_extended h;
    uint8_t d;

} Payload_extended;

union seed_standard{
    Payload_standard p;
    char b[11];
};

union seed_extended{
    Payload_extended p;
    uint8_t b[13];
};

uint16_t CrcNext(uint16_t crc, uint8_t data);
uint32_t ReverseBits(int32_t num, int8_t bits_size);

#endif