#include "Crc.h"
#include <stdio.h>

uint16_t CrcNext(uint16_t crc, uint8_t data, int8_t skip)
{
    uint16_t eor;
    unsigned int i = (8-skip);
    uint8_t bit;

    printf("i:%d\n", i);
    printf("before: %x\n", data);

    data <<= skip;

    printf("after: %x\n", data);

    //crc ^= (uint16_t)data << 7;
    do {
        // eor = crc & 0x4000 ? 0x4599 : 0;
        // crc <<= 1;
        // crc ^= eor;
        bit = data & 0x80;
        bit >>= 7;
        data <<= 1;
        printf("%d\n", bit);
        crc = calculateCRC(crc, bit);
    } while (--i);

    return crc;

    //return crc & 0x7fff;
}

uint16_t calculateCRC(uint16_t crc, uint8_t bit)
{
    crc <<= 1;
    if ((crc >= (1 << 15)) ^ bit) { // um smente no bit mais significativo
        crc ^= 0x4599;
    }
    crc &= 0x7fff; // zero no bit mais significativo e um no resto

    return crc;
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