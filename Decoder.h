#ifndef DECODER_H
#define DECODER_H

//#include <Arduino.h>
#include <stdio.h>
#include <stdint.h>
#include "Crc.h"

class Decoder {

    public:
        Decoder();

        void initInterestBits();
        int64_t build_standard_frame();
        int64_t build_standard_frame_reversed();
        void runTest(int64_t frame, int8_t len);
        void displayStateInfo();
        void displayFrameRead();
        void execute(int8_t rx, int8_t sample_point, int8_t bit_destuffing_error);
        void execute(int8_t rx);
        void displayVariables();

        
        //interest Bits
        int8_t rx;
        int8_t rtr_srr;
        int8_t rtr;
        int8_t srr;
        int8_t ide;
        int8_t dlc;
        int8_t reserved;
        int8_t data_count_aux;
        int16_t ida;
        int32_t idb; //extended identifier
        int64_t data;
        int16_t data_count;
        int16_t crc;
        int16_t computed_crc;
        int8_t ack;
        int8_t idle;
        int8_t bit_stuffing_enable;

        //Error Bits
        int8_t bit_destuffing_error;
        int8_t crc_error;
        int8_t crc_delim_error;
        int8_t ack_error;

    private:

        int8_t state;
        int8_t next_state;
        int8_t last_rx;
        int8_t count_ida;
        int8_t count_idb;
        int8_t count_reserved;
        int8_t data_len;
        int8_t crc_count;
        int8_t eof_count;
        int8_t error_count;

        //payloads
        seed_standard standard_payload;
        seed_extended extended_payload;
};


#endif
