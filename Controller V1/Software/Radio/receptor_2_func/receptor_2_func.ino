#include <SPI.h>
#include <RF24.h>

#include "printf.h"

#define DATARATE RF24_2MBPS
//  #define DATARATE RF24_1MBPS
// #define DATARATE RF24_250KBPS

const uint64_t pipes[2] = {0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL};

RF24 radio(9, 10);



// The debug-friendly names of those roles
const char* role_friendly_name[] = {"Potentiometer", "Neopixel"};

// The role of the current running sketch

int _success = 0;
int _fail = 0;
unsigned long _startTime = 0;




typedef struct
{
  int A0;
  int A1;
  int A2;
}
controlDef;
controlDef controlPak;


typedef struct
{
  float red;
  float green;
  float blue;
}
displayDef;
displayDef displayPak;





void setup(void)
{
  Serial.begin(115200);

  printf_begin();
  printf("\n\rRF24/examples/pingpair/\n\r");

  
  //
  // Setup and configure RF radio
  radio.begin();

  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1, pipes[1]);
 

  radio.setDataRate( DATARATE ) ;
  radio.setPALevel( RF24_PA_MAX ) ;
  radio.setChannel(0x34);
  radio.enableDynamicPayloads() ;
  radio.enableAckPayload();               // not used here
  radio.setRetries(0, 15);                // Smallest time between retries, max no. of retries
  radio.setAutoAck( true ) ;

  radio.printDetails();                   // Dump the configuration of the rf unit for debugging

  radio.powerUp();
  radio.startListening();
} 

void loop(void)
{

  // read potentiometer values and send it out. Receive telemetry data and send it to serial monitor
    unsigned long loop_start = millis();
    // printf("Now sen ding %i...", controlPak.steering);

    // First, stop listening so we can talk.
    radio.stopListening();
    if (!radio.write( &controlPak, sizeof(controlPak) )) {
      // Serial.println(F("failed."));
    }
    radio.startListening();
    // Serial.println(F("delivery success."));
    // printf("Time: %i ", millis() - loop_start);

    while ( !radio.available() && (millis() - loop_start) < 200) {
      // Serial.println(F("waiting."));
    }
    if (millis() - loop_start >= 200) {
      // printf("Failed. Timeout: %i...", millis() - loop_start);
      _fail++;
    } else {
      // get the telemetry data
      radio.read( &displayPak, sizeof(displayPak) );
      // Serial.print("Got response ");
      _success++;
    }

    
      int _ratio = 100 * _fail / (_fail + _success);
      Serial.print("Time ");
      _startTime = (millis() - _startTime);
      Serial.print(_startTime);
      Serial.print(" success ");
      Serial.print(_success);
      Serial.print(" timeout ");
      Serial.print(_fail);
      Serial.print(" Failed ");
      Serial.print(_ratio);
      Serial.print("% ");
      for (int _i = 0; _i < _ratio; _i++) Serial.print("*");
      Serial.print(" red ");
      Serial.print(displayPak.red);
      Serial.print(" green ");
      Serial.print(displayPak.green);
      Serial.print(" blue ");
      Serial.print(displayPak.blue);
      Serial.println();
      _success = 0;
      _fail = 0;
      _startTime = millis();


  //
  // Receive potentiometer values, display them on neopixels, read TMC3200 and send values back
 
    // if there is data ready
  if ( radio.available())
    {
      radio.read(&controlPak, sizeof(controlPak));

      

      // Keep listening in case of transmitter is retransmitting the message
      // The delay should be enough to cover all the possible retries
      unsigned long wait_for_ready_start = millis();
      while ((millis() - wait_for_ready_start) < 100)
      {
        if (radio.available())
        {
          controlDef dataDummy;
          // Get (and discard) the answer data
          radio.read(&dataDummy, sizeof(dataDummy));
        }
      }

      // Send the final one back. This way, we don't delay
      // the reply while we wait on serial i/o.
      radio.stopListening();
      radio.write( &displayPak, sizeof(displayPak) );
      // Serial.print("Sent response ");

      // Now, resume listening so we catch the next packets.
      radio.startListening();
      
      }
     
    
  
}
