#include "BitTiming.h"

enum states {
    START_STATE,
    COUNT_TSEG1_STATE,
    COMPENSATE1_STATE,
    COUNT_TSEG2_STATE,
    COMPENSATE2_STATE
};

BitTiming::BitTiming(int8_t pin_rx,
                        int8_t pin_time_quanta,
                        int8_t pin_state_1,
                        int8_t pin_state_2,
                        int8_t pin_hard_reset,
                        int8_t pin_soft_reset,
                        int8_t sjw,
                        int8_t tseg1,
                        int8_t tseg2)
{

    this->pin_time_quanta = pin_time_quanta;
    this->pin_state_1 = pin_state_1;
    this->pin_state_2 = pin_state_2;
    this->pin_hard_reset = pin_hard_reset;
    this->pin_soft_reset = pin_soft_reset;

    this->tq_counter = 0;
    this->state = START_STATE;
    this->next_state = START_STATE;
    this->sjw = sjw;
    this->tseg1 = tseg1;
    this->tseg2 = tseg2;

}

void BitTiming::edgeDetector() {
    if ((this->previous_rx == HIGH) && (this->rx == LOW)) {
        this->negedge_rx = HIGH;
        Serial.println("edge detected");
    }
    else
        this->negedge_rx = LOW;
}

void BitTiming::printStateInfo (bool debugFlag, bool pinStatus) {
    if (debugFlag) {
        switch (this->state) {
            case START_STATE:
            Serial.println("START_STATE ");
            break;

            case COUNT_TSEG1_STATE:
            Serial.print("COUNT_TSEG1_STATE || ");
            Serial.print("count_tseg1 ");
            Serial.println(this->count_tseg1, DEC);
            break;

            case COMPENSATE1_STATE:
            Serial.println("COMPENSATE1_STATE ");
            Serial.print("e: ");
            Serial.println(this->e);

            break;

            case COUNT_TSEG2_STATE:
            Serial.print("COUNT_TSEG2_STATE || ");
            Serial.print("count_tseg2 ");
            Serial.println(this->count_tseg2, DEC);
            break;

            case COMPENSATE2_STATE:
            Serial.println("COMPENSATE2_STATE ");
            Serial.print("e: ");
            Serial.println(this->e);
            break;
        }
        if (write_point) {
            Serial.println(" write point = 1");
        }
        if (sample_point)
            Serial.println(" sample_point = 1");
        if(soft_reset)
            Serial.println(" soft_reset = 1");
        if(hard_reset)
            Serial.println(" hard_reset = 1");

        }

    if (pinStatus) {
        Serial.print(" Previous RX: ");
        Serial.println(this->previous_rx);
        Serial.print(" RX: ");
        Serial.println(this->rx);
        Serial.print(" idle: ");
        Serial.println(this->idle);
        Serial.print("\n\n");
    }
}

bool BitTiming::checkIdle(){

    //bool idle_value =  digitalRead(pin_idle);
    bool idle_value = false;
    return idle_value;
}

void BitTiming::setIOPins() {
    //INPUTS
    #ifdef TEST
        this->rx = this->syntethic_rx;
    #else
        this->rx = digitalRead(this->pin_rx);
    #endif

    BitTiming::edgeDetector();
    this->idle = BitTiming::checkIdle();

    //OUTPUTS

    digitalWrite(this->pin_hard_reset, this->hard_reset);
    digitalWrite(this->pin_soft_reset, this->soft_reset);

    //displaying states
    switch (this->state) {
        case START_STATE: //0x00
        digitalWrite(this->pin_state_1, LOW);
        digitalWrite(this->pin_state_2, LOW);
        break;

        case COUNT_TSEG1_STATE://0x01
        digitalWrite(this->pin_state_1, LOW);
        digitalWrite(this->pin_state_2, HIGH);
        break;

        case COMPENSATE1_STATE:
        break;

        case COUNT_TSEG2_STATE://0x10
        digitalWrite(this->pin_state_1, HIGH);
        digitalWrite(this->pin_state_2, LOW);
        break;

        case COMPENSATE2_STATE:
        break;
    }
}

void BitTiming::execute(){
    bool debugFlag = true;
    bool pinStatusFlag = true;
    BitTiming::setIOPins();

    //hard reset logic
    if(BitTiming::checkIdle() && this->negedge_rx){
        Serial.println("hard reset trigger");
        this->hard_reset = 0x1;
    }
    else
        this->hard_reset = 0x0;
    //soft reset logic
    if(!(BitTiming::checkIdle()) && this->negedge_rx){
        Serial.println("soft reset trigger");
        this->soft_reset = 0x1;
    }
    else
        this->soft_reset = 0x0;

    switch (this->state) {

        case START_STATE:
        this->count_tseg1 = 0;
        this->count_tseg2 = 0;
        this->sample_point = 0;
        this->write_point = 0;
        //state transition
        this->next_state = COUNT_TSEG1_STATE;

        break;

        case COUNT_TSEG1_STATE:
        this->count_tseg2 = 0;
        this->count_tseg1++;
        this->write_point = 1;
        if (this->count_tseg1 > this->sjw)
        {
            this->e = this->sjw - 1;
        } else {
            this->e = this->count_tseg1 - 1;
        }
        //state transitions
        if (this->hard_reset) { /// present in every state
            this->count_tseg1 = 0;
            this->next_state = COUNT_TSEG1_STATE;
        } else if (this->negedge_rx && this->soft_reset) {
            this->next_state = COMPENSATE1_STATE;
        } else if (this->count_tseg1 == this->tseg1) {
            this->next_state = COUNT_TSEG2_STATE;
        }

        break;

        case COMPENSATE1_STATE:
        this->e -= 1;
        this->sample_point = 0;
        //state transition
        if (this->hard_reset) { /// present in every state
            this->count_tseg1 = 0;
            this->next_state = COUNT_TSEG1_STATE;
        } else if (this->e <= 0) {
            this->next_state = COUNT_TSEG2_STATE;
        }

        break;

        case COUNT_TSEG2_STATE:

        this->count_tseg1 = 0;
        this->count_tseg2++;
        this->sample_point = 1;
        if (this->count_tseg2 > this->sjw)
        {
            this->e = this->sjw - 1;
        } else {
            this->e = this->count_tseg2 - 1;
        }
        //state transitions
        if (this->hard_reset) { /// present in every state
            this->count_tseg1 = 0;
            this->next_state = COUNT_TSEG1_STATE;
        } else if (this->negedge_rx && soft_reset) {
            this->next_state = COMPENSATE2_STATE;
        } else if (this->count_tseg2 == this->tseg2) {
            this->next_state = START_STATE;
        }
        break;

        case COMPENSATE2_STATE:
        this->e -= 1;
        this->sample_point = 0;
        //state transition
        if (this->hard_reset) { /// present in every state
            this->count_tseg1 = 0;
            this->next_state = COUNT_TSEG1_STATE;
        } else if (this->e <= 0) {
            this->next_state = COUNT_TSEG2_STATE;
        }
        break;
    }
    BitTiming::printStateInfo(debugFlag, pinStatusFlag);
    this->state = this->next_state;
    this->previous_rx = this->rx;

}



