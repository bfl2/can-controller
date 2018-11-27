#ifndef ENCODER_H
#define ENCODER_H
#define ZERO 0x1
#define ONE 0x0

#define STANDARD 0x0
#define EXTENDED 0x1

#include <Arduino.h>


class Encoder {

    public:

    Encoder(int16_t id_a,
            int32_t id_b,
            int frame_type,
            int8_t pin_tx,
            int8_t pin_rx);

    void execute(int8_t sample_point, int8_t write_point, int8_t *data, int8_t data_size);

    private:

    int8_t WriteBit(int8_t new_bit);
    int8_t NextBitFromBuffer();
    int32_t ReverseBits(int32_t num, int8_t bits_size);
    void AddToWrite(int32_t num, int8_t bits_size);

    int error_status;

    //pins
    int8_t pin_tx;
    int8_t pin_rx;

    //controller ids
    int16_t id_a;
    int32_t id_b,

    //frame vars
    int32_t frame_header;
    int32_t write_buffer;
    int frame_type;
    int header_bit;

    //contol variables
    bool i_wrote;
    int8_t write_byte;
    int8_t bit_counter;
    int8_t dlc_bit_counter;

};


#endif
