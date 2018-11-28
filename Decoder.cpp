#include "Decoder.h"


enum states  {
    IDLE_ST = 0,
    IDA_ST,
    RTR_SRR_ST,
    IDE_ST,
    EXT_ST,
    RTR_ST,
    READ_RESERVED_BITS_ST,
    STD_ST,
    DATA_LEN_ST,
    DATA_ST,
    CRC_ST,
    CRC_DELIM_ST,
    ACK_SLOT_ST,
    ACK_DELIM_ST,
    EOF_ST,
    INTERMISSION1_ST,
    INTERMISSION2_ST
};


void Decoder::execute(int8_t rx) 
{ 
    int8_t data_count_aux;
    this->rx = rx;
    switch (this->state) {
        case IDLE_ST:
            //Init variables
            this->ida = 0;
            this->count_ida = 0;
            this->count_idb = 0;
            this->data = 0;
            this->data_len = 4;
            this->crc_count = 15;

            if (this->rx == 0)
                this->next_state = IDA_ST;
            break;

        case IDA_ST:
            this->count_ida += 1;
            this->ida << 1;
            this->ida += this->rx;
            
            //transitions
            if (this->count_ida == 11) 
            {
                this->next_state = RTR_SRR_ST;
            }
            //verify if count_ida  does not exceed 11 bits 

            break;

        case RTR_SRR_ST:
            this->rtr_srr = this->rx;

            break;

        case IDE_ST:
            this->ide = this->rx;

            if(this->ide == 1)
            {
                this->next_state = EXT_ST;

            } else {
                this->next_state = STD_ST;
            }
           
            break;

        case EXT_ST:
            this->count_idb += 1;
            this->idb << 1;
            this->idb += this->rx;

             //transitions
            if (this->count_ida == 18) 
            {
                this->next_state = RTR_ST;
            }
            //verify if count_ida  does not exceed 18 bits 

            break;

        case RTR_ST:
            this->srr = this->rtr_srr; //frame read previously was srr
            this->rtr = this->rx;

            //transitions
            this->next_state = READ_RESERVED_BITS_ST;

            break;

        case  READ_RESERVED_BITS_ST:

            this->count_reserved -= 1;
            
            //transitions
            if (this->data_len == 0)
            {
                this->next_state = DATA_LEN_ST;
            }
            
            break;
        
        case DATA_LEN_ST:
            this->data_len -= 1;
            this->data_count << 1;
            this->data_count += this->rx;
            //transitions
            if (this->rtr == 0 && this->data_len == 0) {
                this->next_state = DATA_ST;
                data_count_aux = this->data_count;
            } else if (this->rtr && this-> data_len == 0) {
                this->next_state = CRC_ST;
            }

            break;

        case DATA_ST:
            data_count_aux -= 1;
            this->data << 1;
            this->data += this->rx;
            //transitions
            if ( data_count_aux == 0 ) {
                this->next_state = CRC_ST;
            }

            break;

        case CRC_ST:

            break;

        case CRC_DELIM_ST:

            break;
        
        case ACK_SLOT_ST:

            break;

        case ACK_DELIM_ST:

            break;
        
        case EOF_ST:

            break;
        
        

        

    }

    this->state = this->next_state;
    this->last_rx = rx;
}
