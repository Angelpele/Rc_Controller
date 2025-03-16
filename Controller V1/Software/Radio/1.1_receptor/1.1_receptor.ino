///GND - GND
//VCC - VCC
//SDA - Pin A4
//SCL - Pin A5

#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <LiquidCrystal.h>
#include <SPI.h>
#include <RF24.h>

#include "printf.h"

#define DATARATE RF24_2MBPS
//  #define DATARATE RF24_1MBPS
// #define DATARATE RF24_250KBPS



const uint64_t pipes[2] = {0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL};
RF24 radio(9, 10);

typedef struct
{
  int A0;
  int A1;
  int A2;
  int A3;
  int A4;
  int A5;
  int A6;
  int A7;
  int A8;
}
controlDef;
controlDef controldata;



typedef struct {
   byte ESTADObut;
   byte PIN_SW;
   byte PIN_SW2;
   byte x_ang;
   byte y_ang;
   byte x_ang2;
   byte y_ang2;
   byte ang_x;
   byte ang_y;
}
displayDef;
displayDef displaydata;

int ax, ay, az;
int gx, gy, gz;
const int button = 8;
int ESTADObut = 0;
int ESTADOultbut = 0;
int correc = 0;
int x, y;
int x2, y2;
int x_ang, y_ang;
int x_ang2, y_ang2;


int _success = 0;
int _fail = 0;
unsigned long _startTime = 0;


void setup()
{
   Serial.begin(115200);
   printf_begin();
   printf("\n\rRF24/examples/pingpair/\n\r");
  
   radio.begin();
   radio.openWritingPipe(pipes[0]);
   radio.openReadingPipe(1, pipes[1]);
   radio.setDataRate(RF24_2MBPS);
   radio.setPALevel( RF24_PA_MAX ) ;
   radio.setChannel(0x34);
   radio.enableDynamicPayloads() ;
   radio.enableAckPayload();               
   radio.setRetries(0, 15);                // Smallest time between retries, max no. of retries
   radio.setAutoAck( true ) ;
   radio.printDetails();                   // Dump the configuration of the rf unit for debugging
   radio.powerUp();
   radio.startListening();

}

void serial(){
  
}
void loop(){
   // Leer las aceleraciones y velocidades angulares

   // comunicación radio
   unsigned long loop_start = millis();
    // printf("Now sen ding %i...", controlPak.steering);

    // First, stop listening so we can talk.
    radio.stopListening();
    if (!radio.write( &controldata, sizeof(controldata) )) {
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
      radio.read( &displaydata, sizeof(displaydata) );
      // Serial.print("Got response ");
      _success++;
    }
    int _ratio = 100 * _fail / (_fail + _success);
      Serial.print("   Time ");
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
      Serial.print(displaydata.ESTADObut);
      Serial.print(" green ");
      Serial.print(displaydata.ang_x);
      Serial.print(" blue ");
      Serial.print(displaydata.ang_y);
      Serial.println();
      _success = 0;
      _fail = 0;
      _startTime = millis();  
    
   Serial.println("");
  if ( radio.available())
    {
      radio.read(&controldata, sizeof(controldata));

      

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
      radio.write( &displaydata, sizeof(displaydata) );
      // Serial.print("Sent response ");

      // Now, resume listening so we catch the next packets.
      radio.startListening();
      
      }
   
}
