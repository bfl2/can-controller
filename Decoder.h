#ifndef DECODER_H
#define DECODER_H

#include <Arduino.h>

class Decoder {

    public:
        Decoder();


        void execute(int8_t rx);

    private:

        int8_t state;
        int8_t next_state;


};


#endif
