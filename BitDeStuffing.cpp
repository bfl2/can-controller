#include "BitDeStuffing.h"

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
    Serial.print(" RX: ");
    Serial.print(BitDeStuffing::getRx());
    Serial.print(" Sample Point: ");
    Serial.print(BitDeStuffing::getSamplePoint());
    Serial.print(" Stuffing Error: ");
    Serial.print(BitDeStuffing::getStuffingErrorFlag());
    Serial.print(" Estado: ");
    Serial.println(this->state);
}

void BitDeStuffing::execute(int8_t rx, int8_t bit_stuffing_enable, int8_t sample_point_in) 
{
    bool isStart;
    this->rx = rx;
    this->bit_stuffing_enable = bit_stuffing_enable;
    this->sample_point_in = sample_point_in;

    switch (this->state) {
        case START_STATE:
            isStart = true;
            this->sample_point_out = this->sample_point_in;
            this->count = 0;
            //STATE TRANSITIONS
            if (bit_stuffing_enable == 1)
                this->next_state = RESTART_STATE;
            
            break;

        case RESTART_STATE:

            this->sample_point_out = this->sample_point_in;
            this->count = 1;
            this->stuffing_error = 0;
            //STATE TRANSITIONS
            if (this->bit_stuffing_enable == 1) 
            {
                if (isStart) 
                {
                    this->count += 1;
                    isStart = false;
                }

                if (this->rx == 0) 
                {
                    this->next_state = COUNT_0_STATE;
                }
                else {
                    this->next_state = COUNT_1_STATE;
                }
            } else {
                this->next_state = START_STATE;
            }

            break;

        case COUNT_0_STATE:

            this->sample_point_out = this->sample_point_in;
            this->count += 1;
            //STATE TRANSITIONS
             if (this->bit_stuffing_enable == 1) 
            {
                if (this->rx == 1) 
                {
                    this->next_state = RESTART_STATE;
                }
                else if (this->count == 5) 
                {
                    this->next_state = CHECK_ERROR_0_STATE;
                }   
            } else {
                this->next_state = START_STATE;
            }

            break;

        case COUNT_1_STATE:

            this->sample_point_out = this->sample_point_in;
            this->count += 1;
            //STATE TRANSITIONS
             if (this->bit_stuffing_enable == 1) 
            {
                if (this->rx == 0) 
                {
                    this->next_state = RESTART_STATE;
                }
                else if (this->count == 5) 
                {
                    this->next_state = CHECK_ERROR_1_STATE;
                }   
            } else {
                this->next_state = START_STATE;
            }

            break;

        case CHECK_ERROR_0_STATE:
            this->count += 1;
            //STATE TRANSITIONS
             if (this->bit_stuffing_enable == 1) 
            {
                if (this->rx == 0) 
                {
                    this->next_state = ERROR_STATE;
                }
                else
                {
                    this->sample_point_out = 0;
                    this->next_state = RESTART_STATE;
                }   
            } else {
                this->next_state = START_STATE;
            }

            break;

        case CHECK_ERROR_1_STATE:

            this->count += 1;
            //STATE TRANSITIONS
             if (this->bit_stuffing_enable == 1) 
            {
                if (this->rx == 1) 
                {
                    this->next_state = ERROR_STATE;
                }
                else
                {
                    this->sample_point_out = 0;
                    this->next_state = RESTART_STATE;
                }   
            } else {
                this->next_state = START_STATE;
            }
            break;

        case ERROR_STATE:
            this->stuffing_error = 1;
            //STATE TRANSITIONS
            this->next_state = RESTART_STATE;
           
            break;
    }
    this->state = this->next_state; //update state
    this->last_rx = this->rx;

}

