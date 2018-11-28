#include "Crc.h"

uint16_t CrcNext(uint16_t crc, uint8_t data)
{
    uint16_t eor;
    unsigned int i = 8;

    crc ^= (uint16_t)data << 7;
    do {
        eor = crc & 0x4000 ? 0x4599 : 0;
        crc <<= 1;
        crc ^= eor;
    } while (--i);

    return crc & 0x7fff;
}