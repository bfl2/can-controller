#include "Encoder.h"

enum states {
    IDLE_STATE,
    ARBITRATION_STATE,
    SRR_STATE,
    RTR_STATE,
    LOST_ARBITRATION_STATE,
    IDE_STATE,
    GAINED_ARBITRATION,
    RESERVED_STATE,
    RESERVED_STATE_2,
    DLC_STATE,
    DATA_STATE
};

Encoder::Encoder(int16_t id_a,
            int32_t id_b,
            int frame_type,
            int8_t pin_tx,
            int8_t pin_rx)
{

    this->state = IDLE_STATE;
    this->stuf_counter = 0;
    this->frame_header = 0x0;
    this->error_status = 0x0;
    this->header_bit = 0;
    this->write_buffer = 0x0;
    this->frame_type = frame_type;  //0 - standard, 1 - extended
    this->id_a = id_a;
    this->id_b = id_b;

    this->i_wrote = false;

    //pins
    this->pin_tx = pin_tx;
    this->pin_rx = pin_rx;

    if(id_a > 0x7FF) //check if greater than 11 bits
        this->error_status = 0x2;
    if(id_b > 0x3FFFF) //check if greater than 18 bits
        this->error_status = 0x3;

    //building frame header
    if(this->frame_type == EXTENDED){
        this->frame_header += 0x1;  //SOF
        this->frame_header << 11;
        this->frame_header += id_a;
        this->frame_header << 1;
        this->frame_header += 0x1;  //SRR
        this->frame_header << 1;
        this->frame_header += 0x1;  //IDE
        this->frame_header << 18;
        this->frame_header += id_b;
    }
    else if(this->frame_type == STANDARD){
        this->frame_header += 1;
        this->frame_header << 11;
        this->frame_header += id_a;
    }
    else{
        this->error_status = 0x1;
    }
}

int8_t Encoder::WriteBit(int8_t new_bit){
    if(new_bit == 1){
        digitalWrite(this->pin_tx, ONE);
        return ONE;
    }
        
    else if(new_bit == 0){
        digitalWrite(this->pin_tx, ZERO);
        return ZERO;
    }
        
}

int8_t Encoder::NextBitFromBuffer(){
    int8_t bit = (this->write_buffer) & 0x01;
    this->write_buffer = this->write_buffer >> 1;
    return bit;
}

int32_t Encoder::ReverseBits(int32_t num, int8_t bits_size){
    
    int8_t count = 0;
    int32_t reverse_num = 0;
    
    while(num){
        reverse_num <<= 1;
        reverse_num += num & 1;
        num >>= 1;
        count += 1;
    }
    
    reverse_num <<= (num_size - count);
    return reverse_num;
}

void Encoder::AddToWrite(int32_t num, int8_t bits_size){
    
    this->write_buffer = this->ReverseBits(num, bits_size);

}

void Encoder::execute(int8_t sample_point, int8_t write_point, int8_t *data, int8_t data_size){
    
    switch (this->state)
    {
        case IDLE_STATE:
            this->state = ARBITRATION_STATE;
            break;

        case ARBITRATION_STATE: //arbitration state
            if(this->header_bit == 0){
                this->AddToWrite(this->frame_header, 32);
                this->WriteBit(this->NextBitFromBuffer());
                this->header_bit += 1;
            }
            else{
                if( ((this->header_bit > 0) && (this->header_bit <= 11) && (this->frame_header == STANDARD))
                    || ((this->header_bit >= 13) && (this->frame_header == EXTENDED)))
                {
                    if((this->i_wrote) && (sample_point == 1)){
                        int bus_status = digitalRead(this->pin_rx);
                        if(bus_status != this->stuff_wrote)
                            this->state = LOST_ARBITRATION_STATE;
                        else{
                            if((this->header_bit == 12) && (this->frame_header == STANDARD))
                                this->state = RTR_STATE;
                            else if((this->header_bit == 12) && (this->frame_header == EXTENDED))
                                this->state = SRR_STATE;
                            else if((this->header_bit == 32))
                                this->state = RTR_STATE;
                        }

                        this->i_wrote = false;
                    }
                    else if(!(this->i_wrote) && (write_point == 1)){
                        this->stuff_wrote = this->WriteBit(this->NextBitFromBuffer());
                        this->i_wrote = true;
                        this->header_bit += 1;
                    }
                }
            }
            break;
        
        case SRR_STATE:
            this->WriteBit(1);
            this->state = IDE_STATE;

        case RTR_STATE:
            this->WriteBit(0);
            if(this->frame_type == STANDARD)
                this->state = IDE_STATE;
            else if(this->frame_type == EXTENDED)
                this->state = RESERVED_STATE;

            break;
        
        case IDE_STATE:
            if(this->frame_header == STANDARD){
                this->WriteBit(0);
                this->state = RESERVED_STATE;
            }
            else if(this->frame_header == EXTENDED){
                this->WriteBit(1);
                this->header_bit += 1;
                this->state = ARBITRATION_STATE;
            }
            break;

        case RESERVED_STATE:
            this->WriteBit(0);
            this->write_byte = 0;
            this->bit_counter = 0;
            this->dlc_bit_counter = 0;
            this->AddToWrite(data_size, 4);

            if(this->frame_type == EXTENDED)
                this->state = RESERVED_STATE_2;
            else if(this->frame_type == STANDARD){

                this->state = DLC_STATE;
            }
                
            break;

        case RESERVED_STATE_2:
            this->WriteBit(0);
            this->state = DLC_STATE;
            break;

        case DLC_STATE:
            this->WriteBit(this->NextBitFromBuffer());
            this->dlc_bit_counter += 1;

            if(this->dlc_bit_counter == 4){
                this->state = DATA_STATE;
            }
            break;

        case DATA_STATE:
            break;

        default:
            break;
    }
}