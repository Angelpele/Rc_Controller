#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#define DATARATE RF24_2MBPS
const uint64_t pipes = "0xF0F0F0F0D2LL";

#include "printf.h"


RF24 radio(9, 10);
int x;

struct Received_data {
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

Received_data received_data;

void reset_the_Data() 
{
  // SEGURIDAD cuando no recibe datos del TX
  received_data.x_ang = 124;   
  received_data.y_ang = 124;
  received_data.x_ang2 = 124;
  received_data.y_ang2 = 124;
  received_data.PIN_SW = 0;
  received_data.PIN_SW2 = 0;
  received_data.ang_x = 125;
  received_data.ang_y = 125;
  received_data.ESTADObut = 0;
  
}

void setup()
{
  Serial.begin(115200);
  printf_begin();
  
  //reset_the_Data();
 
  radio.begin();
  radio.openReadingPipe(0, pipes);
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


unsigned long lastRecvTime = 0;

void receive_the_data()
{
  while ( radio.available() ) {
  radio.read(&received_data, sizeof(Received_data));
  lastRecvTime = millis(); 
  }
}

void loop()
{
   Serial.print(F("Rotacion en X:  "));
   Serial.print(received_data.ang_x);
   Serial.print(F("\t Rotacion en Y: "));
   Serial.print(received_data.ang_y);
   //Serial.print(F("\t Rotacion en Z: "));
   //Serial.println(ang_z);
   
   Serial.print("\t X_ang:" );
   Serial.print(received_data.x_ang);
   Serial.print("  ");
   Serial.print( "Y_ang:" );
   Serial.print(received_data.y_ang);
   Serial.print( " Botjoy:");
   Serial.print(received_data.PIN_SW);
   Serial.print( "   X_ang2:" );
   Serial.print(received_data.x_ang2);
   Serial.print("  ");
   Serial.print( "Y_ang2:" );
   Serial.print(received_data.y_ang2);
   Serial.print( " Botjoy2:");
   Serial.print(received_data.PIN_SW2);
 
   
   Serial.print(F("  Botón switch: "));
   Serial.println(received_data.ESTADObut);
   


   
   receive_the_data();

// si se pierde la señal por 1 segundo
  unsigned long now = millis();
  if ( now - lastRecvTime > 2000 ) {
    reset_the_Data();
    Serial.println("Fail...............................................");
    
  } 
}
