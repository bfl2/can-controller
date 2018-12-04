#include "BitDeStuffing.h"
#include <Arduino.h>

#ifndef ARDUINO

#define ARDUINO
#endif

enum states {
    START_STATE = 0,
    RESTART_STATE,
    COUNT_0_STATE,
    COUNT_1_STATE,
    CHECK_ERROR_0_STATE,
    CHECK_ERROR_1_STATE,
    ERROR_STATE,
};

BitDeStuffing::BitDeStuffing(int8_t rx, int8_t bit_stuffing_enable, int8_t sample_point_in) 
{

    this->rx;

    this->state = START_STATE;
    this->next_state = START_STATE;
    this->last_rx = 0;
    this->count = 0;
    this->bit_stuffing_enable = bit_stuffing_enable;
    this->stuffing_error = 0;
    this->sample_point_in = sample_point_in;
    this->sample_point_out = 0;

}

BitDeStuffing::BitDeStuffing() 
{
    this->state = START_STATE;
    this->next_state = START_STATE;
    this->bit_stuffing_enable = 1;
    this->last_rx = 1;
    this->count = 0;
    this->stuffing_error = 0;
}

int8_t BitDeStuffing::getRx() {
    return this->rx;
}
int8_t BitDeStuffing::getSamplePoint() {
    return this->sample_point_out;
}
int8_t BitDeStuffing::getStuffingErrorFlag() {
    return this->stuffing_error;
}

void BitDeStuffing::runTest() {
    
    BitDeStuffing::execute(1,1,1);
    BitDeStuffing::printStatus();
    BitDeStuffing::execute(1,1,1);
    BitDeStuffing::printStatus();
    BitDeStuffing::execute(1,1,1);
    BitDeStuffing::printStatus();
    BitDeStuffing::execute(1,1,1);
    BitDeStuffing::printStatus();
    BitDeStuffing::execute(1,1,1);
    BitDeStuffing::printStatus();
    BitDeStuffing::execute(1,1,1);
    BitDeStuffing::printStatus();
    BitDeStuffing::execute(0,1,1);
    BitDeStuffing::printStatus();
    BitDeStuffing::execute(1,1,1);
    BitDeStuffing::printStatus();
}

void BitDeStuffing::printStatus() 
{
    #ifdef ARDUINO
    Serial.print(" RX: ");
    Serial.print(BitDeStuffing::getRx());
    Serial.print(" Sample Point: ");
    Serial.print(BitDeStuffing::getSamplePoint());
    Serial.print(" Stuffing Error: ");
    Serial.print(BitDeStuffing::getStuffingErrorFlag());
    Serial.print(" Estado: ");
    Serial.println(this->state);
    #else
    // printf("\nenabled:%d RX: %d Sample point: %d stuffing_error: %d  Estado: %d|\n", this->bit_stuffing_enable, this->rx, this->sample_point_out, this->stuffing_error, this->state);
    #endif
}

void BitDeStuffing::execute(int8_t rx, int8_t bit_stuffing_enable, int8_t sample_point_in) 
{
    bool isStart;
    this->rx = rx;
    this->bit_stuffing_enable = bit_stuffing_enable;
    this->sample_point_in = sample_point_in;

    if(this->error_fixed){
        this->error_fixed = 0;
        this->state = START_STATE;
        this->next_state = START_STATE;
    }

    switch (this->state) {
        case START_STATE:
            this->stuffing_error = 0;
            this->sample_point_out = 1;

            if ((bit_stuffing_enable == 1) || (rx == 0)) {
                this->count = 1;
                this->last_rx = rx;
                this->next_state = RESTART_STATE;
            }
        break;

        case RESTART_STATE:
            if(this->bit_stuffing_enable == 0){
                this->count = 1;
                this->stuffing_error = 0;
                this->sample_point_out = 1;
                this->last_rx = rx;
            }
            else{
                if(this->sample_point_out == 0)
                    this->sample_point_out = 1;

                if(rx == this->last_rx){
                    this->count += 1;
                    if(this->count == 6){
                        this->stuffing_error = 1;
                        this->next_state = ERROR_STATE;
                    }
                }
                else{
                    if(this->count == 5){
                        printf("STUFFED\n");
                        this->count = 1;
                        this->last_rx = rx;
                        this->sample_point_out = 0;
                        this->next_state = RESTART_STATE;
                    }
                    else{
                        this->last_rx = rx;
                        this->count = 1;
                    }
                }  
            }
        break;

        case ERROR_STATE:
            this->stuffing_error = 1;
            //STATE TRANSITIONS
            if(this->error_fixed == 1){
                this->error_fixed = 0;
                this->next_state = RESTART_STATE;
            }
        break;
    }
    //printStatus();
    this->state = this->next_state; //update state

    if(this->error_fixed == 1)
        this->last_rx = this->rx;

}

