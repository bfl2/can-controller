#ifndef __ENCODER_H__
#define __ENCODER_H__
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
#define BIT_ERROR_FLAG 2

#define SW

#include <stdint.h>
#include <stdio.h>

#include "Crc.h"
//#include <Arduino.h>


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

    private:

        int8_t StuffingState();
        int8_t __writeBit(int8_t bit);
        int8_t WriteBit(int8_t new_bit);
        int8_t NextBitFromBuffer();
        void BitErrorCheck();
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
        seed_standard standard_payload;
        seed_extended extended_payload;

};


#endif
