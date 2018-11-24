#include "BitTiming.h"

#define PIN_TIME_QUANTA 13
#define PIN_RX 11 //Input
#define PIN_STATE_2 10
#define PIN_STATE_1 9
#define PIN_IDLE 8 //Input

#define PIN_SOFT_RESET 6
#define PIN_HARD_RESET 5

#define TSEG1 8
#define TSEG2 7
#define SJW 1

#define TQ  1000000 // in microseconds
#define TEST

bool waitingTimeQuanta = true;

BitTiming bit_timing = BitTiming(PIN_RX,
                        PIN_TIME_QUANTA,
                        PIN_STATE_1,
                        PIN_STATE_2,
                        PIN_HARD_RESET,
                        PIN_SOFT_RESET,
                        SJW,
                        TSEG1,
                        TSEG2);

#ifdef TEST
  #define BITS_QUANTITY 3
  int tq_counter=-1; //time quanta counter
  int8_t bus_value[BITS_QUANTITY][16] = {
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
  };
#endif

void callback()
{
    // SYNTETHIC BUS READING
    #ifdef TEST
        tq_counter += 1;
        if((tq_counter / 16) > (BITS_QUANTITY - 1)){
            tq_counter = 0;
        }
        Serial.print("TQ: ");
        Serial.println(tq_counter);
    #endif

    digitalWrite(PIN_TIME_QUANTA, digitalRead(PIN_TIME_QUANTA) ^ 1);
    waitingTimeQuanta = false;

}

void setup()
{
    Serial.begin(57600);        // connect to the serial port
    //OUTPUTS
    pinMode(PIN_TIME_QUANTA, OUTPUT);
    pinMode(PIN_HARD_RESET, OUTPUT);
    pinMode(PIN_SOFT_RESET, OUTPUT);
    pinMode(PIN_STATE_1, OUTPUT);
    pinMode(PIN_STATE_2, OUTPUT);
    //INPUT
    pinMode(PIN_RX, INPUT);
    pinMode(PIN_IDLE, INPUT);

    Timer1.initialize(TQ);
    Timer1.attachInterrupt(callback);
}

void loop() {
    if(!waitingTimeQuanta){
        bit_timing.execute();
        //decoder();
        waitingTimeQuanta = true;
    }
}
