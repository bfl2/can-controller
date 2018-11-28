#include <iostream>
#include <string>

#include "Encoder.h"
#include "Decoder.h"
#include "BitDeStuffing.h"

int main(){

    int8_t status = 0;
    bool error_fixed = false;

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

    int standard_frame[] = {0,1,1,0,0,1,1,1,0,0,1,0,0,0,0,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1,1,0,1,1,0,1,1,1,1,0,0,1,1,1,0,1,1,1,1,1,1,1,1};

    int8_t sample_point = 0;
    // int standard_frame[] = {0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,1,0,0,0,0,1,1,0,0,0,0,0,0,1,0,1,1,1,1,1,1,1,1,1,1,1,-1};

    for(int i=0; ;i++){
        int can_bit = standard_frame[i];

        bde.execute(can_bit, 1, 1);
        sample_point = bde.getSamplePoint();

        if(sample_point == 1){
            if(can_bit == -1)
                break;

            else{
                printf("%d ", can_bit);
                d.execute(can_bit);
            }
        }
    }


    // while(true){
    //     if(status == 10){
    //         printf(" STATUS: %d\n", status);
    //         break;
    //     }
    //     else if((status == 2) && (error_fixed == false)){
    //         printf(" STATUS: %d\n", status);
    //         e.ErrorFlaging(ACTIVE_ERROR_FLAG);
    //     }

    //     if(e.error_flag == NON_ERROR_FLAG)
    //         status = e.Execute(1, 1, 1, 0, data, 1, REMOTE_FRAME, EXTENDED);
    //     else{
    //         status = e.ExecuteError(1, 1, 1);
    //         if(e.error_flag == NON_ERROR_FLAG)
    //             error_fixed = true;
    //     }

    // }

    return 0;
}