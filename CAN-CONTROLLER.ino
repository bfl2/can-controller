#include "BitTiming.h"
#include "BitDeStuffing.h"

#define PIN_TIME_QUANTA 13
#define PIN_TX 12 //Output
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

#define ID_A 13
#define ID_B 13

BitTiming bit_timing = BitTiming(PIN_RX,
                        PIN_TIME_QUANTA,
                        PIN_STATE_1,
                        PIN_STATE_2,
                        PIN_HARD_RESET,
                        PIN_SOFT_RESET,
                        SJW,
                        TSEG1,
                        TSEG2);
BitDeStuffing bit_de_stuffing = BitDeStuffing();

Encoder encoder = Encoder(ID_A,
                    ID_B,
                    EXTENDED,
                    PIN_TX,
                    PIN_RX);


#ifdef TEST
  #define BITS_QUANTITY 3
  int tq_counter=-1; //time quanta counter
  int8_t bus_value[BITS_QUANTITY][16] = {
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
  };
#endif

bool waiting_time_quanta = true;
bool want_write = false;
bool data_buffered = false;
bool bus_idle = true;
bool data_recv = false;
unsigned long time_delay = 0;

unsigned long long TX_DATA = 0x0;
unsigned long long RX_DATA = 0x0;

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
    waiting_time_quanta = false;

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

    //check if can put the roll
    if(digitalRead(PIN_RX) == HIGH)     //recessive
        bus_idle = true;
    else if(digitalRead(PIN_RX) == LOW) //dominant
        bus_idle = false;
}

void application(){
    if(data_recv){

    }

    if((!data_buffered) && ((time_delay-micros()) == 1000)){
        DATA[0] = 199;
        time_delay = micros();
        data_buffered = true;
    }
}

void loop() {
    if(!waiting_time_quanta){
        bit_timing.execute();
        bit_destuffing.execute();

        //check if buss is idle
        if((!bus_idle) && bit_destuffing.output_sample_point){
            decoder.execute();
        }
        else if(bus_idle && bit_timing.write_point){
            encoder(bit_destuffing.output_sample_point)
        }
        waiting_time_quanta = true;
    }

    application();
}
