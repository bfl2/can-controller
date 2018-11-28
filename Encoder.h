#ifndef ENCODER_H
#define ENCODER_H
#define ZERO 0x1
#define ONE 0x0

#define STANDARD 0x0
#define EXTENDED 0x1

#define DATA_FRAME 0x1
#define REMOTE_FRAME 0x0

#define CRC_GENERATOR 0x4599
#define CRC_GENERATOR_SPEC 0xC599

#define ACTIVE_ERROR_FLAG 1
#define PASSIVE_ERROR_FLAG -1
#define NON_ERROR_FLAG 0

#define SW

#include <stdint.h>
#include <stdio.h>
//#include <Arduino.h>

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
} payload_standard;

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
} payload_extended;

union seed_standard{
    payload_standard p;
    char b[11];
};

union seed_extended{
    payload_extended p;
    char b[13];
};

class Encoder{

    public:

    //error flag
    uint8_t error_flag;

    Encoder(
            int8_t pin_tx,
            int8_t pin_rx);

    int8_t Execute(
            int8_t sample_point, 
            int8_t write_point,
            int16_t id_a,
            int32_t id_b,
            int8_t data[8], 
            int8_t data_size,
            int8_t data_frame_flag,
            int frame_type);

    int8_t ExecuteError(
        int8_t sample_point, 
        int8_t write_point,
        bool idle);

    void ErrorFlaging(uint8_t flag);

    uint16_t CrcNext(uint16_t crc, uint8_t data);

    private:

    int8_t StuffingState();
    int8_t __writeBit(int8_t bit);
    int8_t WriteBit(int8_t new_bit);
    int8_t NextBitFromBuffer();
    int32_t ReverseBits(int32_t num, int8_t bits_size);
    void AddToWrite(int32_t num, int8_t bits_size);
    void CreateCRCSeed();

    int error_status;

    int8_t state;
    int8_t next_state;

    //pins
    int8_t pin_tx;
    int8_t pin_rx;

    //controller id
    int16_t id_a;
    int32_t id_b;

    //frame vars
    int32_t frame_header;
    int32_t write_buffer;
    int16_t frame_crc;
    int frame_type;

    //contol variables
    bool i_wrote;
    int8_t write_byte;
    int8_t bit_counter;
    int8_t data_counter;
    int8_t stuff_wrote;
    
    //bit stuffing control
    bool stuffing_control;
    int8_t previous_bit;
    int8_t stuffed_bit;
    int8_t stuffing_counter;

    //payloads
    seed_extended extended_payload;
    seed_standard standard_payload;

};


#endif
