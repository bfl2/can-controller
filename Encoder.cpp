#include "Encoder.h"

enum states {
    SOF_STATE,
    ARBITRATION_STATE,
    SRR_STATE,
    RTR_STATE,
    IDE_STATE,
    RESERVED_STATE,
    RESERVED_STATE_2,
    DLC_STATE,
    DATA_STATE,
    CRC_STATE,
    CRC_DELIMITER_STATE,
    ACK_STATE,
    ACK_DELIMITER_STATE,
    EOF_STATE,
    LOST_ARBITRATION_STATE,
    STUFFING_HANDLER,
    ACTIVE_ERROR_STATE,
    PASSIVE_ERROR_STATE,
    ERROR_DELIMITER_STATE,
    END_STATE
};

Encoder::Encoder(int8_t pin_tx, int8_t pin_rx)
{
    this->state = SOF_STATE;
    this->stuffing_counter = 0;
    this->frame_header = 0x0;
    this->error_status = 0x0;
    this->write_buffer = 0x0;
    this->frame_type = frame_type;  //0 - standard, 1 - extended
    this->id_a = id_a;
    this->id_b = id_b;
    this->error_flag = NON_ERROR_FLAG;

    this->i_wrote = false;

    //pins
    this->pin_tx = pin_tx;
    this->pin_rx = pin_rx;
}

int8_t Encoder::__writeBit(int8_t bit){
    #ifdef ARDUINO
    if(bit == 1){
        digitalWrite(this->pin_tx, ONE);
        return ONE;
    }
        
    else if(bit == 0){
        digitalWrite(this->pin_tx, ZERO);
        return ZERO;
    }
    #endif

    #ifdef SW
    if(bit == 1){
        printf("%d", ONE);
        return ONE;
    }
    else if(bit == 0){
        printf("%d", ZERO);
        return ZERO;
    }
    #endif
}

int8_t Encoder::StuffingState(){
    #ifdef SW
    printf("(STUFFING ");
    #endif
    this->state = STUFFING_HANDLER;

    int8_t bit = !(this->previous_bit)&1;
    this->previous_bit = bit;
    this->stuffing_counter = 1;

    return this->__writeBit(bit);
}

int8_t Encoder::WriteBit(int8_t new_bit){

    if(this->stuffing_control){
        if(this->stuffing_counter == 5){
            this->StuffingState();
            this->stuffed_bit = new_bit;

            #ifdef SW
            printf(") ");
            #endif

            return -1;
        }
        else{
            if(this->previous_bit == new_bit)
                this->stuffing_counter += 1;
            else if(this->previous_bit != new_bit){
                this->stuffing_counter = 1;
                this->previous_bit = new_bit;
            }
        }
    }

    return this->__writeBit(new_bit);
}

void Encoder::BitErrorCheck () {
     #ifdef SW
    int bus_status = this->stuff_wrote;
    if(bus_status != this->stuff_wrote && this->state > ARBITRATION_STATE) {
        this->error_flag = BIT_ERROR_FLAG;
    }
    #endif
    #ifdef ARDUINO
    int bus_status = digitalRead(this->pin_rx);
    if(bus_status != digitalRead(this->pin_tx) && this->state > ARBITRATION_STATE) {
        this->error_flag = BIT_ERROR_FLAG;
    }
    #endif
}

int8_t Encoder::NextBitFromBuffer(){
    int8_t bit = (this->write_buffer) & 0x1;
    this->write_buffer = this->write_buffer >> 1;
    return bit;
}

void Encoder::AddToWrite(int32_t num, int8_t bits_size){
    this->write_buffer = 0;
    this->write_buffer = ReverseBits(num, bits_size);
}

void Encoder::ErrorFlaging(uint8_t flag){
    this->error_flag = flag;
    this->bit_counter = 0;

    if(this->error_flag == ACTIVE_ERROR_FLAG)
        this->next_state = ACTIVE_ERROR_STATE;
    else if(this->error_flag == PASSIVE_ERROR_FLAG)
        this->next_state = PASSIVE_ERROR_STATE;
    else if(this->error_flag == NON_ERROR_FLAG)
        this->next_state = SOF_STATE;
    else if(this->error_flag == BIT_ERROR_FLAG)
        this->next_state = ACTIVE_ERROR_STATE;
        
}

int8_t Encoder::Execute(
        int8_t sample_point, 
        int8_t write_point,
        int16_t id_a,
        int32_t id_b,
        int8_t data[8], 
        int8_t data_size,
        int8_t data_frame_flag,
        int frame_type){

    uint8_t is_this_stuffed, next_bit;

    if(this->state != STUFFING_HANDLER)
        this->state = this->next_state;
    
    switch(this->state)
    {
        case SOF_STATE:
            #ifdef SW
            printf("SOF ");
            #endif
            this->error_flag = NON_ERROR_FLAG;
            this->stuffing_control = true;

            if(id_a > 0x7FF) //check if greater than 11 bits
                this->error_status = 0x2;
            if(id_b > 0x3FFFF) //check if greater than 18 bits
                this->error_status = 0x3;

            this->id_a = id_a;
            this->id_b = id_b;
            this->frame_type = frame_type;

            //building frame header
            if(this->frame_type == EXTENDED){
                this->frame_header += id_a;
                this->frame_header <<= 1;
                this->frame_header += 0x1;  //SRR
                this->frame_header <<= 1;
                this->frame_header += 0x1;  //IDE
                this->frame_header <<= 18;
                this->frame_header += id_b;

                this->standard_payload.p.BLANK = 0;
                this->extended_payload.p.SOF = 0;
                this->extended_payload.p.ID_A = ReverseBits(this->id_a, 11);
                this->extended_payload.p.ID_B = ReverseBits(this->id_b, 18);
                this->extended_payload.p.SRR = 1;
            }
            else if(this->frame_type == STANDARD){
                this->frame_header += id_a;

                this->standard_payload.p.BLANK = 0;
                this->standard_payload.p.SOF = 0;
                this->standard_payload.p.ID_A = ReverseBits(this->id_a, 11);
            }
            else{
                this->error_status = 0x1;
            }

            if(this->frame_type == STANDARD)
                this->AddToWrite(this->frame_header, 11);
            else if(this->frame_type == EXTENDED)
                this->AddToWrite(this->frame_header, 31);

            this->bit_counter = 1;
            this->WriteBit(0);

            this->next_state = ARBITRATION_STATE;

        break;

        case ARBITRATION_STATE: //arbitration next_state
            #ifdef SW
            printf("ARBITRATION ");
            #endif

            if( ((this->bit_counter > 0) && (this->bit_counter <= 12))
                || ((this->bit_counter >= 13) && (this->frame_type == EXTENDED)))
            {
                if((this->i_wrote) && (sample_point == 1)){
                    #ifdef SW
                    printf("CHECK ");
                    #endif

                    #ifdef ARDUINO
                    int bus_status = digitalRead(this->pin_rx);
                    #endif

                    #ifdef SW
                    int bus_status = this->stuff_wrote;
                    printf("%d", bus_status);
                    #endif

                    if(bus_status != this->stuff_wrote)
                        this->next_state = LOST_ARBITRATION_STATE;
                    else{
                        if((this->bit_counter == 12) && (this->frame_type == STANDARD))
                            this->next_state = RTR_STATE;
                        else if((this->bit_counter == 12) && (this->frame_type == EXTENDED))
                            this->next_state = SRR_STATE;
                        else if((this->bit_counter == 32))
                            this->next_state = RTR_STATE;
                    }

                    this->i_wrote = false;
                }
                else if(!(this->i_wrote) && (write_point == 1)){
                    #ifdef SW
                    printf("WRITING ");
                    #endif

                    next_bit = this->NextBitFromBuffer();

                    this->i_wrote = true;
                    this->bit_counter += 1;

                    is_this_stuffed = this->WriteBit(next_bit);
                    if(is_this_stuffed != -1)   //not stuffed
                        this->stuff_wrote = is_this_stuffed;
                    else
                        goto end_automata;
                }
            }

        break;
        
        case SRR_STATE:
            #ifdef SW
            printf("SRR ");
            #endif

            if((this->i_wrote) && (sample_point == 1)){
                #ifdef SW
                printf("CHECK ");
                #endif
                #ifdef ARDUINO
                int bus_status = digitalRead(this->pin_rx);
                #endif

                #ifdef SW
                int bus_status = this->stuff_wrote;
                printf("%d", bus_status);
                #endif

                this->i_wrote = false;

                if(bus_status != this->stuff_wrote)
                    this->next_state = LOST_ARBITRATION_STATE;
                else
                    this->next_state = IDE_STATE;
            }
            else if(!(this->i_wrote) && write_point == 1){
                #ifdef SW
                printf("WRITING ");
                #endif

                this->extended_payload.p.SRR = 1;

                this->bit_counter += 1;
                this->i_wrote = true;
        
                is_this_stuffed = this->WriteBit(1);
                if(is_this_stuffed != -1)   //not stuffed
                    this->stuff_wrote = is_this_stuffed;
                else
                    goto end_automata;
            }
            BitErrorCheck();

        break;

        case RTR_STATE:
            #ifdef SW
            printf("RTR ");
            #endif

            if((this->i_wrote) && (sample_point == 1)){
                #ifdef SW
                printf("CHECK ");
                #endif

                #ifdef ARDUINO
                int bus_status = digitalRead(this->pin_rx);
                #endif

                #ifdef SW
                int bus_status = this->stuff_wrote;
                printf("%d", bus_status);
                #endif

                this->i_wrote = false;

                if(bus_status != this->stuff_wrote)
                    this->next_state = LOST_ARBITRATION_STATE;
                else{
                    if(this->frame_type == STANDARD){
                        this->standard_payload.p.RTR = data_frame_flag;
                        this->next_state = IDE_STATE;
                    }
                        
                    else if(this->frame_type == EXTENDED){
                        this->extended_payload.p.RTR = data_frame_flag;
                        this->next_state = RESERVED_STATE;
                    }
                }
            }
            else if(!(this->i_wrote) && write_point == 1){
                #ifdef SW
                printf("WRITING ");
                #endif

                this->i_wrote = true;

                is_this_stuffed = this->WriteBit(data_frame_flag);
                if(is_this_stuffed != -1)   //not stuffed
                    this->stuff_wrote = is_this_stuffed;
                else
                    goto end_automata;
            }
            BitErrorCheck();
        break;
        
        case IDE_STATE:
            #ifdef SW
            printf("IDE ");
            #endif

            if(this->frame_type == STANDARD){
                this->standard_payload.p.IDE = 0;
                this->next_state = RESERVED_STATE;

                is_this_stuffed = this->WriteBit(0);
                if(is_this_stuffed != -1)   //not stuffed
                    this->stuff_wrote = is_this_stuffed;
                else
                    goto end_automata;
            }
            else if(this->frame_type == EXTENDED){
                if((this->i_wrote) && (sample_point == 1)){
                    #ifdef SW
                    printf("CHECK ");
                    #endif

                    #ifdef ARDUINO
                    int bus_status = digitalRead(this->pin_rx);
                    #endif

                    #ifdef SW
                    int bus_status = this->stuff_wrote;
                    printf("%d", bus_status);
                    #endif

                    this->i_wrote = false;

                    if(bus_status != this->stuff_wrote)
                        this->next_state = LOST_ARBITRATION_STATE;
                    else{
                        this->AddToWrite(this->id_b, 18);
                        this->next_state = ARBITRATION_STATE;
                    }
                }
                else if(!(this->i_wrote) && write_point == 1){
                    #ifdef SW
                    printf("WRITING ");
                    #endif

                    this->standard_payload.p.IDE = 1;

                    this->bit_counter += 1;
                    this->i_wrote = true;

                    is_this_stuffed = this->WriteBit(1);
                    if(is_this_stuffed != -1)   //not stuffed
                        this->stuff_wrote = is_this_stuffed;
                    else
                        goto end_automata;
                }
            }
            BitErrorCheck();
        break;

        case RESERVED_STATE:
            #ifdef SW
            printf("RESERVED ");
            #endif

            this->write_byte = 0;
            this->bit_counter = 0;
            this->data_counter = 0;
            this->AddToWrite(data_size, 4);

            if(this->frame_type == EXTENDED){
                this->extended_payload.p.RESERVED = 3;
                this->extended_payload.p.DLC = ReverseBits(data_size, 4);
                this->next_state = RESERVED_STATE_2;
            }
            else if(this->frame_type == STANDARD){
                this->standard_payload.p.RESERVED = 1;
                this->standard_payload.p.DLC = ReverseBits(data_size, 4);
                this->next_state = DLC_STATE;
            }

            is_this_stuffed = this->WriteBit(0);
            if(is_this_stuffed != -1)   //not stuffed
                this->stuff_wrote = is_this_stuffed;
            else
                goto end_automata;

        break;

        case RESERVED_STATE_2:
            #ifdef SW
            printf("RESERVED2 ");
            #endif

            this->next_state = DLC_STATE;

            is_this_stuffed = this->WriteBit(0);
            if(is_this_stuffed != -1)   //not stuffed
                this->stuff_wrote = is_this_stuffed;
            else
                goto end_automata;
            BitErrorCheck();
            
        break;

        case DLC_STATE:
            #ifdef SW
            printf("DLC ");
            #endif

            this->bit_counter += 1;

            next_bit = this->NextBitFromBuffer();
        
            if(this->bit_counter == 4){
                this->bit_counter = 0;
                this->AddToWrite(data[0], 8);
                
                if(data_frame_flag)
                    this->next_state = DATA_STATE;
                else
                    this->next_state = CRC_STATE;
            }

            is_this_stuffed = this->WriteBit(next_bit);
            if(is_this_stuffed != -1)   //not stuffed
                this->stuff_wrote = is_this_stuffed;
            else
                goto end_automata;
            BitErrorCheck();

        break;

        case DATA_STATE:
            #ifdef SW
            printf("DATA ");
            #endif

            this->bit_counter += 1;

            next_bit = this->NextBitFromBuffer();

            if(this->bit_counter == 8){
                if(this->frame_type == STANDARD){
                    this->standard_payload.b[3+this->data_counter] = 
                            ReverseBits(data[(data_size - this->data_counter - 1)], 8);
                }
                else if(this->frame_type ==  EXTENDED){
                    this->extended_payload.b[5+this->data_counter] = 
                            ReverseBits(data[(data_size - this->data_counter - 1)], 8);
                }

                this->bit_counter = 0;
                this->data_counter += 1;
                this->AddToWrite(data[this->data_counter], 8);
            }
            if(this->data_counter == data_size){
                int max_bytes;
                int skip;
                this->frame_crc = 0;
                uint8_t data_aux;

                if(this->frame_type == STANDARD){
                    max_bytes = 3 + data_size;
                    for(int i=0; i < max_bytes; i++){

                        data_aux = ReverseBits(this->standard_payload.b[i]&0xff, 8);
                        this->frame_crc = CrcNext(this->frame_crc, data_aux);
                    }
                }
                else if(this->frame_type == EXTENDED){
                    max_bytes = 5 + data_size;
                    for(int i=0; i < max_bytes; i++){

                        data_aux = ReverseBits(this->extended_payload.b[i]&0xff, 8);
                        this->frame_crc = CrcNext(this->frame_crc, data_aux);
                    }
                }

                this->AddToWrite(this->frame_crc, 15);
                this->bit_counter = 0;

                this->next_state = CRC_STATE;
            }

            is_this_stuffed = this->WriteBit(next_bit);
            if(is_this_stuffed != -1)   //not stuffed
                this->stuff_wrote = is_this_stuffed;
            else
                goto end_automata;
            BitErrorCheck();
        break;

        case CRC_STATE:
            #ifdef SW
            printf("CRC ");
            #endif

            this->bit_counter += 1;
            next_bit = this->NextBitFromBuffer();

            if(this->bit_counter == 15)
                this->next_state = CRC_DELIMITER_STATE;

            is_this_stuffed = this->WriteBit(next_bit);
            if(is_this_stuffed != -1)   //not stuffed
                this->stuff_wrote = is_this_stuffed;
            else
                goto end_automata;
        
            BitErrorCheck();
        break;

        case CRC_DELIMITER_STATE:
            #ifdef SW
            printf("CRC DELIMITER ");
            #endif

            this->WriteBit(1);
            this->stuffing_control = false;

            this->next_state = ACK_STATE;

        break;

        case ACK_STATE:
            #ifdef SW
            printf("ACK ");
            #endif

            if((this->i_wrote) && (sample_point == 1)){
                #ifdef SW
                printf("CHECK ");
                #endif

                #ifdef ARDUINO
                int bus_status = digitalRead(this->pin_rx);
                #endif

                #ifdef SW
                int bus_status = this->stuff_wrote;
                printf("%d", bus_status);
                #endif

                if(this->error_flag){
                    this->bit_counter = 0;

                    this->ErrorFlaging(ACTIVE_ERROR_FLAG);
                }
                    
                else
                    this->next_state = ACK_DELIMITER_STATE;

                this->i_wrote = false;
            }
            else if(!(this->i_wrote) && (write_point == 1)){
                #ifdef SW
                printf("WRITING ");
                #endif

                next_bit = this->NextBitFromBuffer();

                this->i_wrote = true;
                this->bit_counter += 1;

                this->WriteBit(1);
            }
            BitErrorCheck();

        break;
        
        case ACK_DELIMITER_STATE:
            #ifdef SW
            printf("ACK DELIMITER ");
            #endif

            this->WriteBit(1);
            this->bit_counter = 0;

            this->next_state = EOF_STATE;
            BitErrorCheck();

        break;

        case EOF_STATE:
            #ifdef SW
            printf("EOF ");
            #endif

            this->WriteBit(1);
            this->bit_counter += 1;

            if(this->bit_counter == 7){
                #ifdef SW
                if(this->next_state != END_STATE)
                    printf("\n");
                #endif

                this->next_state = SOF_STATE;
                return 10;  //finished status
            }
            BitErrorCheck();
        break;

        case LOST_ARBITRATION_STATE:
            #ifdef SW
            printf("LOST ARBITRATION ");
            #endif

            return 1;   //lost arbitration status
            BitErrorCheck();
        break;

        case STUFFING_HANDLER:
            this->stuff_wrote = this->WriteBit(this->stuffed_bit);

            this->state = this->next_state;
            BitErrorCheck();
        break;

        default:
        #ifdef SW
        printf("Restarting automata\n");
        #endif

        this->next_state = SOF_STATE;
        
        break;
    }


    end_automata:
    if(this->state == STUFFING_HANDLER)
        return 2;
    else{
        #ifdef SW
        if(this->state != END_STATE)
            printf("\n");
        #endif
        return 0;
    }
}

int8_t Encoder::ExecuteError(
    int8_t sample_point, 
    int8_t write_point,
    bool idle){

    int8_t status = 0;
    this->state = this->next_state;
    this->stuffing_control = false;

    switch(this->state){

        case ACTIVE_ERROR_STATE:
            #ifdef SW
            printf("ACTIVE ERROR %d ", bit_counter);
            #endif

            this->WriteBit(1);
            this->bit_counter += 1;

            if(this->bit_counter == 6){
                this->bit_counter = 0;
                this->next_state = ERROR_DELIMITER_STATE;
            }
        break;

        case PASSIVE_ERROR_STATE:
            #ifdef SW
            printf("PASSIVE ERROR %d ", bit_counter);
            #endif
            
            this->WriteBit(0);
            this->bit_counter += 1;

            if(this->bit_counter == 6){
                this->bit_counter = 0;
                this->next_state = ERROR_DELIMITER_STATE;
            }
        break;

        case ERROR_DELIMITER_STATE:
            #ifdef SW
            printf("ERROR DELIMITER %d ", bit_counter);
            #endif

            this->WriteBit(0);
            this->bit_counter += 1;

            if(this->bit_counter == 8){
                this->bit_counter = 0;
                this->ErrorFlaging(NON_ERROR_FLAG);
                status = 10;
            }
            
        break;
    }

    #ifdef SW
    if(this->state != END_STATE)
        printf("\n");
    #endif

    return status;
}
