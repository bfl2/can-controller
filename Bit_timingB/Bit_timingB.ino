#include "TimerOne.h"

#define pin_time_quanta 13
#define pin_RX 11 //Input
#define pin_state_2 10
#define pin_state_1 9
#define pin_idle 8 //Input

#define pin_soft_reset 6
#define pin_hard_reset 5

#define TSEG1 8
#define TSEG2 7
#define SJW 1

#define TQ  1000000 // in microseconds

#define BITS_QUANTITY 3
#define TEST


enum states {
  START_STATE,
  COUNT_TSEG1_STATE,
  COMPENSATE1_STATE,
  COUNT_TSEG2_STATE,
  COMPENSATE2_STATE
};

int8_t state;
int8_t next_state;
int8_t count_tseg1;
int8_t count_tseg2;
int8_t sample_point;
int8_t write_point;
int8_t e;
bool waitingTimeQuanta = true;

int8_t previous_rx = HIGH; //for edge detection
int8_t rx;
bool negedge_rx = LOW;
bool idle;

int8_t hard_reset = 0;
int8_t soft_reset = 0;

int8_t count = 0;

#ifdef TEST
  int tq_counter=-1; //time quanta counter
  int8_t bus_value[BITS_QUANTITY][16] = {
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
  };
#endif


void edgeDetector() {
  if (previous_rx == HIGH && rx == LOW) {
    negedge_rx = HIGH;
    Serial.println("edge detected");
  }
  else
    negedge_rx = LOW;
}

void callback()
{
  // SYNTETHIC BUS READING
  #ifdef TEST
  tq_counter += 1;
  if((tq_counter / 16) > (BITS_QUANTITY - 1)){
    tq_counter = 0;
  }
  #endif

  
  digitalWrite(pin_time_quanta, digitalRead(pin_time_quanta) ^ 1);
  waitingTimeQuanta = false;
  Serial.print("TQ: ");
  Serial.println(tq_counter);
}
void setup()
{
  Serial.begin(57600);        // connect to the serial port
  //OUTPUTS
  pinMode(pin_time_quanta, OUTPUT);
  pinMode(pin_hard_reset, OUTPUT);
  pinMode(pin_soft_reset, OUTPUT);
  pinMode(pin_state_1, OUTPUT);
  pinMode(pin_state_2, OUTPUT);
  //INPUT
  pinMode(pin_RX, INPUT);
  pinMode(pin_idle, INPUT);

  Timer1.initialize(TQ);
  Timer1.attachInterrupt(callback);
  state = START_STATE;
  next_state = START_STATE;
}

void printStateInfo (bool debugFlag, bool pinStatus) {

  if (debugFlag) {
    switch (state) {
      case START_STATE:
        Serial.println("START_STATE ");
        break;

      case COUNT_TSEG1_STATE:
        Serial.print("COUNT_TSEG1_STATE || ");
        Serial.print("count_tseg1 ");
        Serial.println(count_tseg1, DEC);
        break;

      case COMPENSATE1_STATE:
        Serial.println("COMPENSATE1_STATE ");
        Serial.print("e: ");
        Serial.println(e);

        break;

      case COUNT_TSEG2_STATE:
        Serial.print("COUNT_TSEG2_STATE || ");
        Serial.print("count_tseg2 ");
        Serial.println(count_tseg2, DEC);
        break;

      case COMPENSATE2_STATE:
        Serial.println("COMPENSATE2_STATE ");
        Serial.print("e: ");
        Serial.println(e);
        break;
    }
    if (write_point) {
      Serial.println(" write point = 1");
    }
    if (sample_point)
      Serial.println(" sample_point = 1");
    if(soft_reset)
      Serial.println(" soft_reset = 1");
    if(hard_reset)
      Serial.println(" hard_reset = 1");

  }

  if (pinStatus) {
    Serial.print(" Previous RX: ");
    Serial.println(previous_rx);
    Serial.print(" RX: ");
    Serial.println(rx);
    Serial.print(" idle: ");
    Serial.println(idle);

    Serial.print("\n\n");

  }
}

bool check_idle(){
  //bool idle_value =  digitalRead(pin_idle);
  bool idle_value = false;
  return idle_value;
}

void setIOPins() {
  //INPUTS
  #ifdef TEST
    int i = tq_counter / 16;
    int j = tq_counter % 16;
    rx = bus_value[i][j];
  #else
    rx = digitalRead(pin_RX);
  #endif

  edgeDetector();
  idle = check_idle();

  //OUTPUTS

  digitalWrite(pin_hard_reset, hard_reset);
  digitalWrite(pin_soft_reset, soft_reset);


  //displaying states
  switch (state) {
    case START_STATE: //00
      digitalWrite(pin_state_1, LOW);
      digitalWrite(pin_state_2, LOW);
      break;

    case COUNT_TSEG1_STATE://01
      digitalWrite(pin_state_1, LOW);
      digitalWrite(pin_state_2, HIGH);
      break;

    case COMPENSATE1_STATE:
      break;

    case COUNT_TSEG2_STATE://10
      digitalWrite(pin_state_1, HIGH);
      digitalWrite(pin_state_2, LOW);
      break;

    case COMPENSATE2_STATE:
      break;
  }


}

void loop() {
  bool debugFlag = true;
  bool pinStatusFlag = true;
  if (!waitingTimeQuanta) {
    setIOPins();

    //hard reset logic
    if(check_idle() && negedge_rx){
      Serial.println("hard reset trigger");
      hard_reset = 0x1;
    }
    else
      hard_reset = 0x0;
    //soft reset logic
    if(!check_idle() && negedge_rx){
      Serial.println("soft reset trigger");
      soft_reset = 0x1;
    }
    else
      soft_reset = 0x0;
    
    switch (state) {

      case START_STATE:
        count_tseg1 = 0;
        count_tseg2 = 0;
        sample_point = 0;
        write_point = 0;
        //state transition
        next_state = COUNT_TSEG1_STATE;

        break;

      case COUNT_TSEG1_STATE:
        count_tseg2 = 0;
        count_tseg1++;
        write_point = 1;
        if (count_tseg1 > SJW)
        {
          e = SJW - 1;
        } else {
          e = count_tseg1 - 1;
        }
        //state transitions
        if (hard_reset) { /// present in every state
          count_tseg1 = 0;
          next_state = COUNT_TSEG1_STATE;
        } else if (negedge_rx && soft_reset) {
          next_state = COMPENSATE1_STATE;
        } else if (count_tseg1 == TSEG1) {
          next_state = COUNT_TSEG2_STATE;
        }

        break;

      case COMPENSATE1_STATE:
        e -= 1;
        sample_point = 0;
        //state transition
        if (hard_reset) { /// present in every state
          count_tseg1 = 0;
          next_state = COUNT_TSEG1_STATE;
        } else if (e <= 0) {
          next_state = COUNT_TSEG2_STATE;
        }


        break;

      case COUNT_TSEG2_STATE:

        count_tseg1 = 0;
        count_tseg2++;
        sample_point = 1;
        if (count_tseg2 > SJW)
        {
          e = SJW - 1;
        } else {
          e = count_tseg2 - 1;
        }
        //state transitions
        if (hard_reset) { /// present in every state
          count_tseg1 = 0;
          next_state = COUNT_TSEG1_STATE;
        } else if (negedge_rx && soft_reset) {
          next_state = COMPENSATE2_STATE;
        } else if (count_tseg2 == TSEG2) {
          next_state = START_STATE;
        }
        break;

      case COMPENSATE2_STATE:
        e -= 1;
        sample_point = 0;
        //state transition
        if (hard_reset) { /// present in every state
          count_tseg1 = 0;
          next_state = COUNT_TSEG1_STATE;
        } else if (e <= 0) {
          next_state = COUNT_TSEG2_STATE;
        }
        break;
    }
    waitingTimeQuanta = true;
    printStateInfo(debugFlag, pinStatusFlag);
    state = next_state;
    previous_rx = rx;

  }

}


