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
    INTERMISSION2_ST,
    ERROR_FLAG_ST,
    ERROR_SUPERPOSITION_ST,
    ERROR_DELIMITER_ST,
    OVERLOAD_FLAG_ST,
    OVERLOAD_SUPER_ST,
    OVERLOAD_DELIM_ST
};

Decoder::Decoder (){
    this->idle = 1;
    this->next_state = IDLE_ST;
}

void Decoder::initInterestBits() {
    this->ida = 0;
    this->idb = 0;
    this->count_ida = 0;
    this->count_idb = 0;
    this->data = 0;
    this->data_len = 4;
    this->crc_count = 15;
    this->eof_count = 0;
    this->bit_stuffing_enable = 0;
    this->bit_destuffing_error = 0;
    this->crc_error = 0;
    this->ack_error = 0;
    this->stuff_error = 0;
    this->crc_delim_error = 0;
}

void Decoder::displayStateInfo() {
    #ifdef ARDUINO
    Serial.print("State: ");
    Serial.println(this->state);
    Serial.print(" RX: ");
    Serial.println(this->rx);
    #else
    printf("State: \n");
    #endif
}

void Decoder::displayFrameRead() {
    #ifdef ARDUINO
    Serial.print("Frame Read: ");
    Serial.print("");
    #endif
}

void Decoder::execute(int8_t rx, int8_t sample_point, int8_t bit_destuffing_error) {
    if (sample_point == 1) {
        this->bit_destuffing_error = bit_destuffing_error;
        execute(rx);
    }
}

void Decoder::computeCRC(){
    if(this->dlc > 0){
        #ifndef ARDUINO
        printf("frame:");
        #endif
    }
    
    if(this->ide == 0x0){    //standard
        this->standard_payload.p.h.BLANK=0;
        this->standard_payload.p.h.SOF = 0;
        this->standard_payload.p.h.ID_A = ReverseBits(this->ida, 11);
        this->standard_payload.p.h.RESERVED = this->reserved;
        this->standard_payload.p.h.DLC = ReverseBits(this->dlc, 4);
        this->standard_payload.p.h.IDE = this->ide;
        this->standard_payload.p.h.RTR = this->rtr;
    }
    else if(this->ide == 0x1){ //extendedth
        this->extended_payload.p.h.BLANK = 0;
        this->extended_payload.p.h.SOF = 0;
        this->extended_payload.p.h.ID_A = ReverseBits(this->ida, 11);
        this->extended_payload.p.h.ID_B = ReverseBits(this->idb, 18);
        this->extended_payload.p.h.SRR = this->rtr_srr;
        this->extended_payload.p.h.RESERVED = ReverseBits(this->reserved, 2);
        this->extended_payload.p.h.DLC = ReverseBits(this->dlc, 4);
        this->extended_payload.p.h.IDE = this->ide;
        this->extended_payload.p.h.RTR = this->rtr;
    }
    
    if(this->dlc > 8)
        this->dlc = 8;

    int64_t data_aux = this->data;
    this->computed_crc = 0;
    uint16_t data_byte;
    int8_t max_bytes;
    int8_t skip;

    for(int i=0; i < this->dlc; i++){
        data_byte = (data_aux & 0xFF);
        if(this->ide == 0x0)
            this->standard_payload.b[3+(this->dlc - i - 1)] = ReverseBits(data_byte, 8);
        else if(this->ide == 0x1)
            this->extended_payload.b[5+(this->dlc - i - 1)] = ReverseBits(data_byte, 8);
        data_aux >>= 8;
    }

    if(this->ide == 0x0){
        max_bytes = 3 + this->dlc;
        for(int i=0; i < max_bytes; i++)
        {
            data_byte = ReverseBits(this->standard_payload.b[i]&0xff, 8);
            this->computed_crc = CrcNext(this->computed_crc, data_byte);

            #ifndef ARDUINO
            printf(" %02X", data_byte);
            #endif
        }
    }
    else if(this->ide == 0x1){
        max_bytes = 5 + this->dlc;
        for(int i=0; i < max_bytes; i++)
        {
            data_byte = ReverseBits(this->extended_payload.b[i]&0xff, 8);
            this->computed_crc = CrcNext(this->computed_crc, data_byte);

            #ifndef ARDUINO
            printf(" %02X", data_byte);
            #endif
        }
    }
    #ifndef ARDUINO
    printf("\n");
    #endif
}

void Decoder::execute(int8_t rx) 
{ 
    this->rx = rx;
    if((this->stuff_error == 1)
        && (this->state != ERROR_FLAG_ST) 
        && (this->state != ERROR_SUPERPOSITION_ST) 
        && (this->state != ERROR_DELIMITER_ST))
    {
        this->next_state = ERROR_FLAG_ST;
    }

    if((this->state == ERROR_FLAG_ST) && (this->rx == 1)){
        this->error_count = 8;
        this->next_state = ERROR_DELIMITER_ST;
    }
        

    this->state = this->next_state;

    switch (this->state) {
        case IDLE_ST:
            #ifndef ARDUINO
            printf("IDLE\n");
            #endif
            //Init variables
            initInterestBits();

            if (this->rx == 0){
                this->bit_stuffing_enable = 1;
                this->next_state = IDA_ST;
                this->idle = 0;
            }
                
        break;

        case IDA_ST:
            #ifndef ARDUINO
            printf("IDA %d\n", this->count_ida);
            #endif
            this->count_ida += 1;
            this->ida <<= 1;
            this->ida += this->rx;
            
            //transitions
            if (this->count_ida == 11) 
            {
                #ifndef ARDUINO
                printf("IDA: 0x%04X\n", this->ida);
                #endif
                this->next_state = RTR_SRR_ST;
            }
            //verify if count_ida  does not exceed 11 bits 
        break;

        case RTR_SRR_ST:
            #ifndef ARDUINO
            printf("RTR_SRR\n");
            #endif
            this->rtr_srr = this->rx;

            //transitions
            this->next_state = IDE_ST;
        break;

        case IDE_ST:
            #ifndef ARDUINO
            printf("IDE\n");
            #endif
            this->ide = this->rx;
            this->rtr = this->rtr_srr;

            if(this->ide == 1)
            {
                this->next_state = EXT_ST;

            } else {
                this->next_state = STD_ST;
            } 
        break;

        case EXT_ST:
            #ifndef ARDUINO
            printf("IDB %d\n", this->count_idb);
            #endif
            this->count_idb += 1;
            this->idb <<= 1;
            this->idb += this->rx;

             //transitions
            if (this->count_idb == 18) 
            {
                #ifndef ARDUINO
                printf("IDB: 0x%05X\n", this->idb);
                #endif
                this->next_state = RTR_ST;
            }
            //verify if count_ida  does not exceed 18 bits 
        break;

        case RTR_ST:
            #ifndef ARDUINO
            printf("RTR\n");
            #endif

            this->srr = this->rtr_srr; //frame read previously was srr
            this->rtr = this->rx;
            
            //transitions
            this->next_state = READ_RESERVED_BITS_ST;
            this->count_reserved = 2;
        break;

        case READ_RESERVED_BITS_ST:
            #ifndef ARDUINO
            printf("RESERVED\n");
            #endif

            this->reserved <<= 1;
            this->reserved += this->rx;

            this->count_reserved -= 1;

            
            //transitions
            if (this->count_reserved == 0)
            {
                this->data_len = 4;
                this->data_count = 0;
                this->next_state = DATA_LEN_ST;
            }
        break;
        
        case DATA_LEN_ST:
            #ifndef ARDUINO
            printf("DATA_LEN\n");
            #endif
            this->data_count <<= 1;
            this->data_count += this->rx;
            this->data_len -= 1;
            //transitions
            if ((this->rtr == 0) && (this->data_len == 0)) {
                this->dlc = this->data_count;

                if(this->data_count > 8)
                    this->data_count = 8;

                this->data_count = (this->data_count*8); 
                this->next_state = DATA_ST;
                this->data_count_aux = this->data_count;
            } else if ((this->rtr == 1) && (this->data_len == 0)) {
                this->dlc = 0;
                this->data_count = 0;
                this->next_state = CRC_ST;
            }
        break;

        case STD_ST:
            #ifndef ARDUINO
            printf("STD\n");
            #endif

            this->data_count = 0;
            this->reserved = this->rx;

            //transitions
            this->next_state = DATA_LEN_ST;
        break;

        case DATA_ST:
            #ifndef ARDUINO
            printf("DATA %d\n", this->data_count_aux);
            #endif
            this->data_count_aux -= 1;
            this->data <<= 1;
            this->data += this->rx;

            //transitions
            if ( this->data_count_aux == 0 ) {
                this->crc_count = 15;
                this->crc = 0;
                this->next_state = CRC_ST;
            }

        break;

        case CRC_ST:
            #ifndef ARDUINO
            printf("CRC %d\n", 15-crc_count);
            #endif
            this->crc_count -= 1;
            this->crc <<= 1;
            this->crc += this->rx;

            if (this->crc_count==0) {
                this->computeCRC();
                if((this->crc - this->computed_crc) != 0){
                    this->crc_error = 1;
                    this->error_count = 5;
                }
    
                this->next_state = CRC_DELIM_ST;
            }

        break;

        case CRC_DELIM_ST:
            #ifndef ARDUINO
            printf("CRC DELIM comparing: 0x%04X - 0x%04X\n", this->crc, this->computed_crc);
            #endif
            this->bit_stuffing_enable = 0;
            this->crc_delim_error = 0;
            //transitions
            if (this->rx == 0) {
                this->crc_delim_error = 1;
                this->error_count = 12;
                this->next_state = ERROR_FLAG_ST;
            } else{
                this->next_state = ACK_SLOT_ST;
            }

        break;
        
        case ACK_SLOT_ST:
            #ifndef ARDUINO
            printf("ACK SLOT\n");
            #endif
            this->ack = this->rx;
            //transitions
            this->next_state = ACK_DELIM_ST;

        break;

        case ACK_DELIM_ST:
            #ifndef ARDUINO
            printf("ACK DELIM\n");
            #endif
            //transitions
            if(this->crc_error == 1){
                this->error_count = 12;
                this->next_state = ERROR_FLAG_ST;
            } else {   
                if (this->rx == 1) {
                    this->next_state = EOF_ST;
                } else {
                    this->next_state = ERROR_FLAG_ST;
                    this->error_count = 12;
                    this->ack_error = 1;
                }
            }

        break;
    
        case EOF_ST:
            #ifndef ARDUINO
            printf("EOF %d \n", this->eof_count);
            #endif
            this->eof_count += 1;
            displayFrameRead();
            
            //transitions
            if (this->eof_count == 7) {
                this->next_state = INTERMISSION1_ST;
            }

        break;

        case INTERMISSION1_ST:
            #ifndef ARDUINO
            printf("INTERMISSION1\n");
            #endif

            //transitions
            if (this->rx == 1) {
                this->next_state = INTERMISSION2_ST;
            }

        break;
        
        case INTERMISSION2_ST:
            #ifndef ARDUINO
            printf("INTERMISSION2\n");
            #endif
            //transitions
            if (this->rx == 1) {
                this->next_state = IDLE_ST;
                this->idle = 1;
            }
        break;

        case ERROR_FLAG_ST:
            #ifndef ARDUINO
            printf("ERROR FLAG {ack_error:%d, crc_delim_error:%d, crc_error:%d, stuff_error:%d}\n", 
                        this->ack_error, this->crc_delim_error, this->crc_error, this->stuff_error);
            #endif
            this->error_count -= 1;
            //transitions
            if(this->error_count == 0) {
                this->error_count = 8;
                this->next_state = ERROR_DELIMITER_ST;
            }
        break;

        case ERROR_SUPERPOSITION_ST:    //checando bit a mais no erro
            #ifndef ARDUINO
            printf("ERROR_SUPERPOSITION\n");
            #endif
            if ( this->rx == 1 ) {
                this->next_state = ERROR_DELIMITER_ST;
            }
        break;

         case ERROR_DELIMITER_ST:
            #ifndef ARDUINO
            printf("ERROR_DELIMITER %d\n", this->error_count);
            #endif
            this->idle = 1;
            this->error_count -= 1;
            if(this->error_count == 0){
                this->next_state = ERROR_DELIMITER_ST;
            }
        break;

        case OVERLOAD_FLAG_ST:

            break;

        case OVERLOAD_SUPER_ST:

            break;
        
        case OVERLOAD_DELIM_ST:

            break;
    }
    this->last_rx = rx;
}
