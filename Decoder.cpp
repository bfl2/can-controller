#include "Decoder.h"


enum states = {
    IDLE_ST = 0,
    IDA_ST,
    RTR_SRR_ST,
    IDE_ST,
    EXT_ST,
    READ_RESERVED_BITS_ST,
    STD_ST,
    DATA_LEN_ST,
    DATA_ST,
    CRC_DELIM_ST,
    ACK_SLOT_ST,
    ACK_DELIM_ST,
    EOF_ST,
    INTERMISSION1_ST,
    INTERMISSION2_ST


};


Decoder::execute(int8_t rx) 
{ 
    switch (this->state) {
        case IDLE_ST:

            break;

        case IDA_ST:

            break;
    }

    this->state = this->next_state;
}