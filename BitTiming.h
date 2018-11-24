#ifndef BIT_TIMING_H
#define BIT_TIMING_H

#include <Arduino.h>
#include "TimerOne.h"

#define TEST

class BitTiming{
public:

    BitTiming(int8_t pin_rx,
                int8_t pin_time_quanta,
                int8_t pin_state_1,
                int8_t pin_state_2,
                int8_t pin_hard_reset,
                int8_t pin_soft_reset,
                int8_t sjw,
                int8_t tseg1,
                int8_t tseg2);

    int8_t bus_value;

    //BIT TIMING METHODS
    void edgeDetector();
    void printStateInfo (bool debugFlag, bool pinStatus);
    bool checkIdle();
    void setIOPins();
    void execute();
    void setTimeQuanta();

private:

    bool idle;

    int tq_counter;

    int8_t state;
    int8_t next_state;
    int8_t count_tseg1;
    int8_t count_tseg2;
    int8_t sample_point;
    int8_t write_point;
    int8_t e;
    int8_t rx;
    int8_t sjw;
    int8_t tseg1;
    int8_t tseg2;

    //pins
    int8_t pin_time_quanta;
    int8_t pin_rx;
    int8_t pin_state_1;
    int8_t pin_state_2;
    int8_t pin_hard_reset;
    int8_t pin_soft_reset;
    //
    
    /////////////////////////

    bool waiting_time_quanta = true;

    int8_t previous_rx = HIGH; //for edge detection
    int8_t negedge_rx = LOW;
    int8_t hard_reset = 0;
    int8_t soft_reset = 0;
    int8_t count = 0;
};


#endif