#ifndef BIT_DE_STUFFING_H
#define BIT_DE_STUFFING_H

//#include <Arduino.h>
#include<stdint.h>
#include <stdio.h>


class BitDeStuffing {

    public:

    BitDeStuffing();
    BitDeStuffing(int8_t rx, int8_t bit_stuffing_enable, int8_t sample_point_in);
    int8_t getRx();
    int8_t getSamplePoint();
    int8_t getStuffingErrorFlag();
    void printStatus();
    void runTest(); 
    void execute(int8_t rx, int8_t bit_stuffing_enable, int8_t sample_point_in);

    int8_t error_fixed;
    int8_t count;

   
    private:

    int8_t rx;
    int8_t sample_point_in;
    int8_t sample_point_out;
    int8_t bit_stuffing_enable;

    int8_t state;
    int8_t next_state;
    int8_t last_rx;
    int8_t stuffing_error;

};


#endif
