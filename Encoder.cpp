#include "Encoder.h"

enum states {
    IDLE_STATE,
    ARBITRATION_STATE,
    SRR_STATE,
    RTR_STATE,
    LOST_ARBITRATION_STATE,
    IDE_STATE,
    RESERVED_STATE,
    RESERVED_STATE_2,
    DLC_STATE,
    DATA_STATE,
    CRC_ACK_STATE
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

        this->standard_payload.BLANK = 0;
        this->extended_payload.SOF = 1;
        this->extended_payload.ID_A = this->id_a;
        this->extended_payload.ID_B = this->id_b;
        this->extended_payload.SRR = 1;
    }
    else if(this->frame_type == STANDARD){
        this->frame_header += 1;
        this->frame_header << 11;
        this->frame_header += id_a;

        this->standard_payload.BLANK = 0;
        this->standard_payload.SOF = 1;
        this->standard_payload.ID_A = this->id_a;
    }
    else{
        this->error_status = 0x1;
    }
}

int8_t Encoder::WriteBit(int8_t new_bit){
    #ifdef ARDUINO
    if(new_bit == 1){
        digitalWrite(this->pin_tx, ONE);
        return ONE;
    }
        
    else if(new_bit == 0){
        digitalWrite(this->pin_tx, ZERO);
        return ZERO;
    }
    #endif



    #ifdef SW
    if(new_bit == 1){
        printf("1");
        return ONE;
    }
        
    else if(new_bit == 0){
        printf("0");
        return ZERO;
    }
    #endif
        
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

uint16_t Encoder::CrcNext(uint16_t crc, uint8_t data)
{
    uint16_t eor;
    unsigned int i = 8;

    crc ^= (uint16_t)data << 7;
    do {
        eor = crc & 0x4000 ? 0x4599 : 0;
        crc <<= 1;
        crc ^= eor;
    } while (--i);

    return crc & 0x7fff;
}

void Encoder::Execute(int8_t sample_point, int8_t write_point, int8_t [8]data, int8_t data_size){
    
    switch (this->state)
    {
        case IDLE_STATE:
            this->state = ARBITRATION_STATE;
            this->bit_counter = 0;
            break;

        case ARBITRATION_STATE: //arbitration state
            if(this->bit_counter == 0){
                this->AddToWrite(this->frame_header, 32);
                this->WriteBit(this->NextBitFromBuffer());
                this->bit_counter += 1;
            }
            else{
                if( ((this->bit_counter > 0) && (this->bit_counter <= 11) && (this->frame_type == STANDARD))
                    || ((this->bit_counter >= 13) && (this->frame_type == EXTENDED)))
                {
                    if((this->i_wrote) && (sample_point == 1)){
                        #ifdef ARDUINO
                        int bus_status = digitalRead(this->pin_rx);
                        #endif

                        #ifdef SW
                        int bus_status = this->stuff_wrote;
                        #endif

                        if(bus_status != this->stuff_wrote)
                            this->state = LOST_ARBITRATION_STATE;
                        else{
                            if((this->bit_counter == 12) && (this->frame_type == STANDARD))
                                this->state = RTR_STATE;
                            else if((this->bit_counter == 12) && (this->frame_type == EXTENDED))
                                this->state = SRR_STATE;
                            else if((this->bit_counter == 32))
                                this->state = RTR_STATE;
                        }

                        this->i_wrote = false;
                    }
                    else if(!(this->i_wrote) && (write_point == 1)){
                        int8_t next_bit = this->NextBitFromBuffer();

                        this->stuff_wrote = this->WriteBit(next_bit);
                        this->i_wrote = true;
                        this->bit_counter += 1;
                    }
                }
            }
            break;
        
        case SRR_STATE:
            this->extended_payload.SRR = 1;
            this->WriteBit(1);
            this->state = IDE_STATE;

        case RTR_STATE:
            this->WriteBit(0);
            if(this->frame_type == STANDARD){
                this->standard_payload.RTR = 0;
                this->state = IDE_STATE;
            }
                
            else if(this->frame_type == EXTENDED){
                this->extended_payload.RTR = 0;
                this->state = RESERVED_STATE;
            }

            break;
        
        case IDE_STATE:
            if(this->frame_type == STANDARD){
                this->WriteBit(0);
                this->standard_payload.IDE = 0;
                this->state = RESERVED_STATE;
            }
            else if(this->frame_type == EXTENDED){
                this->WriteBit(1);
                this->standard_payload.IDE = 1;
                this->bit_counter += 1;
                this->state = ARBITRATION_STATE;
            }
            break;

        case RESERVED_STATE:
            this->WriteBit(0);
            this->write_byte = 0;
            this->bit_counter = 0;
            this->data_counter = 0;
            this->AddToWrite(data_size, 4);

            if(this->frame_type == EXTENDED){
                this->extended_payload.RESERVED = 3;
                this->extended_payload.DLC = data_size;
                this->state = RESERVED_STATE_2;
            }
                
            else if(this->frame_type == STANDARD){
                this->standard_payload.RESERVED = 1;
                this->standard_payload.DLC = data_size;
                this->state = DLC_STATE;
            }
                
            break;

        case RESERVED_STATE_2:
            this->WriteBit(0);
            this->state = DLC_STATE;
            break;

        case DLC_STATE:
            this->WriteBit(this->NextBitFromBuffer());
            this->bit_counter += 1;

            if(this->bit_counter == 4){
                this->bit_counter = 0;
                this->AddToWrite(data[0]);
                this->state = DATA_STATE;
            }
            break;

        case DATA_STATE:
            this->WriteBit(this->NextBitFromBuffer());
            this->bit_counter += 1;

            if(this->bit_counter == 8){
                if(this->frame_type == STANDARD){
                    this->standard_payload.b[this->data_counter] = data[this->data_counter];
                }
                else if(this->frame_type ==  EXTENDED){
                    this->extended_payload.b[this->data_counter] = data[this->data_counter];
                }

                this->bit_counter = 0;
                this->data_counter += 1;
                this->AddToWrite(data[this->data_counter]);
            }
            if(this->data_counter == data_size){
                int max_bytes;
                this->frame_crc = 0;

                if(this->frame_type == STANDARD)
                    max_bytes = 3 + data_size;
                else if(this->frame_type == EXTENDED)
                    max_bytes = 5 + data_size;
                
                for(int i=0; i < max_bytes; i++)
                    this->frame_crc = this->CrcNext(this->frame_crc, this->ReverseBits(data[i], 8));

                this->state = CRC_STATE;
            }
            break;
        
        case CRC_ACK_STATE:

            printf("Starting ack");

            break;

        default:
            break;
    }
}