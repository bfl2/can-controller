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
    ERROR_DELIMITER_ST
};

Decoder::Decoder (){
    this->idle = 1;
    this->state = IDLE_ST;
}

void Decoder::initInterestBits() {
    this->ida = 0;
    this->count_ida = 0;
    this->count_idb = 0;
    this->data = 0;
    this->data_len = 4;
    this->crc_count = 15;
    this->eof_count = 0;
    this->bit_stuffing_enable = 1;
    this->bit_destuffing_error = 0;
    this->crc_error = 0;
    this->ack_error = 0;
}

int64_t Decoder::build_standard_frame () {
    // frame example: SOF0 IDA00000010100 RTR0 IDE0 RB0 DTLEN0001 DATA00000001 CRC01000011000000 CRC_D1 ACK0 ACK_DEL1 EOF1111111 111
    

    int64_t frame = 0;
    int8_t bit_count;
    int8_t sof = 0;
    int16_t ida = 20;
    int8_t rtr = 0;
    int8_t ide = 0;
    int8_t dtlen = 1;
    int64_t data = 1;
    int16_t crc = 4288;
    int8_t eof = 127;
    frame <<= 1;
    frame += sof;
    frame <<= 11;
    frame += ida;
    frame <<= 1;
    frame += rtr;
    frame <<= 1;
    frame += ide;
    frame <<= 1;
    frame += 0; // reserved bit
    frame <<= 4;
    frame += dtlen;
    frame <<= 8;
    frame += 1;
    frame <<= 15;
    frame += crc;
    frame <<= 1;
    frame += 1;//crc del
    frame <<= 1;
    frame += 0; //ack
    frame <<= 1;
    frame += 1;//ack del
    frame <<= 7;
    frame += eof;
    frame <<= 1;
    frame += ide;
    frame <<= 3;
    frame += 7;//intermission
    

    return frame;
}

int64_t Decoder::build_standard_frame_reversed () {
    // frame example: SOF0 IDA00000010100 RTR0 IDE0 RB0 DTLEN0001 DATA00000001 CRC01000011000000 CRC_D1 ACK0 ACK_DEL1 EOF1111111 111

    int64_t frame = 0;
    int8_t bit_count;
    int8_t sof = 0;
    int16_t ida = 20;
    int8_t rtr = 0;
    int8_t ide = 0;
    int8_t dtlen = 1;
    int64_t data = 1;
    int16_t crc = 4288;
    int8_t eof = 127;

    frame <<= 3;
    frame += 7;//intermission
    frame <<= 1;
    frame += ide;
    frame <<= 7;
    frame += eof;
    frame <<= 1;
    frame += 1;//ack del
    frame <<= 1;
    frame += 0; //ack
    frame <<= 1;
    frame += 1;//crc del
    frame <<= 15;
    frame += crc;
    frame <<= 8;
    frame += 1;
    frame <<= 4;
    frame += dtlen;
    frame <<= 1;
    frame += 0; // reserved bit
    frame <<= 1;
    frame += rtr;
    frame <<= 1;
    frame += ide;
    frame <<= 11;
    frame += ida;
    frame <<= 1;
    frame += sof;
    
    return frame;
}

void Decoder::runTest(int64_t frame, int8_t frame_len) {
    int8_t synth_rx;
    for (int i=0; i<frame_len; i++)
    {
        synth_rx = frame % 2;
        frame = frame >> 1;
        execute(synth_rx);
        displayStateInfo();
    }

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
    #else
    printf("Frame Read: %d %d %d %d %ld", this->ida, this->rtr, this->ide, this->dlc, this->data);
    #endif

}

void Decoder::displayVariables(){

}

void Decoder::execute(int8_t rx, int8_t sample_point, int8_t bit_destuffing_error) {
    if (sample_point == 1) {
        this->bit_destuffing_error = bit_destuffing_error;
        execute(rx);
    }
}

void Decoder::execute(int8_t rx) 
{ 
    this->rx = rx;
    switch (this->state) {
        case IDLE_ST:
            #ifndef ARDUINO
            printf("IDLE\n");
            #endif
            //Init variables
            initInterestBits();

            if (this->rx == 0)
                this->next_state = IDA_ST;
                this->idle = 0;
            break;

        case IDA_ST:
            #ifndef ARDUINO
            printf("IDA\n");
            #endif
            this->count_ida += 1;
            this->ida <<= 1;
            this->ida += this->rx;
            
            //transitions
            if (this->count_ida == 11) 
            {
                printf("IDA: 0x%04X\n", this->ida);
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
            printf("EXT\n");
            #endif
            this->count_idb += 1;
            this->idb <<= 1;
            this->idb += this->rx;

             //transitions
            if (this->count_ida == 18) 
            {
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
                this->data_count = (this->data_count*8); 
                this->next_state = DATA_ST;
                this->data_count_aux = this->data_count;
            } else if ((this->rtr == 1) && (this->data_len == 0)) {
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
                if(this->ide == 0x0){    //standard
                    this->standard_payload.p.BLANK=0;
                    this->standard_payload.p.SOF = 0;
                    this->standard_payload.p.ID_A = ReverseBits(this->ida, 11);
                    this->standard_payload.p.RESERVED = this->reserved;
                    this->standard_payload.p.DLC = ReverseBits(this->dlc, 4);
                    this->standard_payload.p.IDE = this->ide;
                    this->standard_payload.p.RTR = this->rtr;
                }
                else if(this->ide == 0x1){ //extended
                    this->extended_payload.p.BLANK = 0;
                    this->extended_payload.p.SOF = 0;
                    this->extended_payload.p.ID_A = ReverseBits(this->ida, 11);
                    this->extended_payload.p.ID_B = ReverseBits(this->ida, 18);
                    this->extended_payload.p.SRR = 1;
                    this->extended_payload.p.RESERVED = ReverseBits(this->reserved, 2);
                    this->extended_payload.p.DLC = ReverseBits(this->dlc, 4);
                    this->extended_payload.p.IDE = this->ide;
                    this->extended_payload.p.RTR = this->rtr;
                }

                int64_t data_aux = this->data;
                this->computed_crc = 0;
                int16_t data_byte;
                int8_t max_bytes;
                int8_t skip;

                for(int i=0; i < this->dlc; i++){
                    data_byte = (data_aux & 0xFF);
                    if(this->ide == 0x0)
                        standard_payload.b[3+(this->dlc - i - 1)] = ReverseBits(data_byte, 8);
                    else if(this->ide == 0x1)
                        extended_payload.b[5+(this->dlc - i - 1)] = data_byte;
                    data_aux >>= 8;
                }

                if(this->ide == 0x0){
                    skip = 5;
                    max_bytes = 3 + this->dlc;

                    printf("DATA(%d): ", max_bytes);

                    for(int i=0; i < max_bytes; i++)
                    {
                        data_aux = ReverseBits(this->standard_payload.b[i]&0xff, 8);
                        printf("%02X ", data_aux);                        

                        if(i != 0)
                            skip = 0;

                        this->computed_crc = CrcNext(this->computed_crc, data_aux, skip);
                    }
                    printf("\n");
                }
                else if(this->ide == 0x1){
                    skip = 1;
                    max_bytes = 5 + this->dlc;
                    for(int i=max_bytes; i > 0; i--)
                    {
                        if(i != 0)
                            skip = 0;

                        data_aux = ReverseBits(this->extended_payload.b[i]&0xff, 8);
                        this->computed_crc = CrcNext(this->computed_crc, data_aux, skip);
                    }
                }

                printf("CRC: ");
                uint8_t crc_aux = this->computed_crc;
                int n;

                for(int i=0; i < 15; i++){
                    n = crc_aux&0x1;
                    crc_aux >>= 1;
                    
                    printf("%d", n);
                }

                printf("\n");

                this->crc_count = 15;
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
                if(this->crc != this->computed_crc){
                    printf("TESTE\n");
                    this->crc_error = 1;
                    this->error_count = 5;
                    this->next_state = ERROR_FLAG_ST;
                }
                else
                    this->next_state = CRC_DELIM_ST;
            }

            break;

        case CRC_DELIM_ST:
            #ifndef ARDUINO
            printf("CRC DELIM\n");
            #endif
            this->bit_stuffing_enable = 0;
            //transitions
            if (this->rx == 1) {
                this->crc_delim_error = 1;
                this->error_count = 5;
                this->next_state = ERROR_FLAG_ST;

            } else {
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
            printf("ACL DELIM\n");
            #endif
            //transitions
            if (this->rx == 1) {
                this->next_state = EOF_ST;
            } else {
                this->next_state = ERROR_FLAG_ST;
                this->error_count = 5;
                this->ack_error = 1;
            }

            break;
        
        case EOF_ST:
            #ifndef ARDUINO
            printf("EOF\n");
            #endif
            this->eof_count += 1;
            
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
            displayFrameRead();
            //transitions
            if (this->rx == 1) {
                this->next_state = IDLE_ST;
                this->idle = 1;
            }
            break;

        case ERROR_FLAG_ST:
            #ifndef ARDUINO
            printf("ERROR FLAG\n");
            #endif
            this->error_count -= 1;
            //transitions
            if( this->error_count ==0 ) {
                this->next_state = ERROR_SUPERPOSITION_ST;
            }

            break;

        case ERROR_SUPERPOSITION_ST:
            #ifndef ARDUINO
            printf("ERROR_SUPERPOSITION\n");
            #endif
            if ( this->rx == 1 ) {
                this->next_state = ERROR_DELIMITER_ST;
            }

            break;
         case ERROR_DELIMITER_ST:
            #ifndef ARDUINO
            printf("ERROR_DELIMITER\n");
            #endif
            this->idle = 1;
            this->next_state = IDLE_ST;
            break;


    }

    this->state = this->next_state;
    this->last_rx = rx;
}
