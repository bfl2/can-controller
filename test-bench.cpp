#include <iostream>
#include <string>

#include "Encoder.h"
#include "Decoder.h"
#include "BitDeStuffing.h"

int main(){

    int8_t status = 0;
    bool error_fixed = true;
    bool no_errors = true;

    Encoder e = Encoder(0, 0);
    Decoder d = Decoder();
    BitDeStuffing bde = BitDeStuffing();

    int8_t data[8] = {
        0x1,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0,
        0x0
    };


    int standard_frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,0,1,0,0,1,1,0,1,0,1,0,1,0,1,1,0,0,1,1,1,0,0,0,0,1,0,1,0,1,0,1,1,1,1,1,1,1,1,-1}; 

    int standard_frame2[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,0,1,1,0,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,1,0,0,1,1,1,0,0,1,0,0,1,1,0,1,0,1,1,1,1,1,1,1,1,-1};

    int standard_frame3[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,0,1,1,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,1,0,0,1,0,0,1,0,1,1,1,0,1,0,1,1,1,1,1,1,1,1,-1};

    int standard_frame4[] = {0,0,0,0,0,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,1,0,0,0,0,1,0,1,1,1,1,1,0,1,1,1,1,1,1,1,1,-1};

    int crc_error_frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,1,0,0,0,0,1,1,1,0,0,0,1,1,0,1,0,0,0,0,0,0,1,1,1,1,1,1,1,1,-1};

    int stuff_error_frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,1,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,-1};

    int8_t sample_point = 0;
    // int standard_frame[] = {0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,1,1,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,1-1};


    int i = 0;
    while(true){
        int can_bit = stuff_error_frame[i];
        if(can_bit == -1)
            break;
        i++;

        bde.execute(can_bit, d.bit_stuffing_enable, 1);
        sample_point = bde.getSamplePoint();

        if(sample_point == 1){
            printf("%d ", can_bit);
            d.execute(can_bit);
        }

        if(
            ((bde.getStuffingErrorFlag() == 1)
            || d.crc_error)
            && error_fixed)
        {
            //printf("here\n");
            e.ErrorFlaging(ACTIVE_ERROR_FLAG);
            error_fixed = false;
            no_errors = false;
            if(bde.getStuffingErrorFlag() == 1)
                d.stuff_error = 1;
                
        }

        if(!error_fixed){
            if(bde.getStuffingErrorFlag() == 1)
                d.stuff_error = 1;

            status = e.ExecuteError(1, 1, 1);
            
            if(status == 10){
                bde.error_fixed = 1;
                error_fixed = true;
                no_errors = true;
            }
        }
            
    }
    return 0;
}