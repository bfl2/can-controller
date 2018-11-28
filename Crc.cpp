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

int32_t ReverseBits(int32_t num, int8_t bits_size){
    int8_t count = 0;
    int32_t reverse_num = 0;
    
    while(num){
        reverse_num <<= 1;
        reverse_num += num & 1;
        num >>= 1;
        count += 1;
    }
    
    reverse_num <<= (bits_size - count);
    return reverse_num;
}