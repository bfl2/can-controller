#ifndef DECODER_H
#define DECODER_H

#include <Arduino.h>

class Decoder {

    public:
        Decoder();

        void initInterestBits();
        int64_t build_standard_frame();
        void runTest();
        void displayStateInfo();
        void execute(int8_t rx);

        
        //interest Bits
        int8_t rx;
        int8_t rtr_srr;
        int8_t rtr;
        int8_t srr;
        int8_t ide;
        int16_t ida;
        int32_t idb; //extended identifier
        int64_t data;
        int16_t data_count;
        int16_t crc;
        int8_t ack;
        int8_t idle;
        int8_t bit_stuffing_enable;

        //Error Bits
        int8_t bit_destuffing_error;
        int8_t crc_error;
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


};


#endif
