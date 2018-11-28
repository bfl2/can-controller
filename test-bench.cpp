#include <iostream>
#include <string>

#include "Encoder.h"

int main(){

    //int8_t a = 0;

    int8_t status = 0;

    Encoder e = Encoder(0, 0);

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


    while(true){
        if((status != 0) && (status != 2)){
            printf("STATUS: %d", status);
            break;
        }
        else
            status = e.Execute(1, 1, 1, 0, data, 1, DATA_FRAME, STANDARD);
    }

    return 0;
}