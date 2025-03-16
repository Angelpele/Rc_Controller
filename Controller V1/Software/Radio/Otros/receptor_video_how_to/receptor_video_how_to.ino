/*
   Arduino RC Receiver - Car Example 
   by Dejan, www.HowToMechatronics.com
   Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
*/
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

int _success = 0;
int _fail = 0;
unsigned long _startTime = 0;


RF24 radio(9, 10);   // nRF24L01 (CE, CSN)
const uint64_t pipes[2] = {0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL};
unsigned long lastReceiveTime = 0;
unsigned long currentTime = 0;

// Max size of this struct is 32 bytes
struct Data_Package {
   byte ESTADObut;
   byte PIN_SW;
   byte PIN_SW2;
   byte x_ang;
   byte y_ang;
   byte x_ang2;
   byte y_ang2;
   byte ang_x;
   byte ang_y;
};

Data_Package data; //Create a variable with the above structure


void setup() {
  
  Serial.begin(115200);  
   radio.begin();
   radio.openWritingPipe(pipes[0]);
   radio.openReadingPipe(1, pipes[1]);
   radio.setDataRate(RF24_2MBPS);
   radio.setPALevel( RF24_PA_MAX ) ;
   radio.setChannel(0x34);
   radio.enableDynamicPayloads() ;
   radio.enableAckPayload();               
   radio.setRetries(0, 20);                // Smallest time between retries, max no. of retries
   radio.setAutoAck( true ) ;
   radio.printDetails();                   // Dump the configuration of the rf unit for debugging
   radio.powerUp();
   radio.startListening();
}
void loop() {
   Serial.print(F("Rotacion en X:  "));
   Serial.print(data.ang_x);
   Serial.print(F("\t Rotacion en Y: "));
   Serial.print(data.ang_y);
   //Serial.print(F("\t Rotacion en Z: "));
   //Serial.println(ang_z);
   
   Serial.print("\t X_ang:" );
   Serial.print(data.x_ang);
   Serial.print("  ");
   Serial.print( "Y_ang:" );
   Serial.print(data.y_ang);
   Serial.print( " Botjoy:");
   Serial.print( !digitalRead(data.PIN_SW) );
   Serial.print( "   X_ang2:" );
   Serial.print(data.x_ang2);
   Serial.print("  ");
   Serial.print( "Y_ang2:" );
   Serial.print(data.y_ang2);
   Serial.print( " Botjoy2:");
   Serial.print( !digitalRead(data.PIN_SW2) );
   Serial.print(F("  Botón switch: "));
   Serial.print(data.ESTADObut);
   Serial.println("");
   _success = 0;
   _fail = 0;
   _startTime = millis();


  
   unsigned long loop_start = millis();
   radio.stopListening();
   radio.startListening();
   _startTime = (millis() - _startTime);
   _startTime = millis();

   if ( radio.available())
    {
      radio.read(&data, sizeof(Data_Package));

      // Keep listening in case of transmitter is retransmitting the message
      // The delay should be enough to cover all the possible retries
      radio.stopListening();   
      radio.startListening();
      
      }
   else{
    resetData();
   }
   delay(100);
}  
  
void resetData() {
  // Reset the values when there is no radio connection - Set initial default values
   data.ESTADObut = 0;
   data.PIN_SW = 0;
   data.PIN_SW2 = 0;
   data.x_ang = 127;
   data.y_ang = 127;
   data.x_ang2 = 127;
   data.y_ang2 = 127;
   data.ang_x = 127;
   data.ang_y = 127;
}
