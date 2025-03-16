#include "RF24.h"
#include <SPI.h> 
#include "nRF24L01.h"
RF24 myRadio (9, 10); // in Mega can use> (48, 49); 


struct package
{
  int id=1;
  float temperature = 18.3;
  char  text[100] = "Text to be transmitted";
};
typedef struct package Package;
Package data;
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

void setup()
{
  Serial.begin(57600);
  myRadio.begin();  
  myRadio.setRetries(15,15);
  myRadio.setAutoAck(false);
  myRadio.setDataRate(RF24_250KBPS);
  myRadio.setPALevel(RF24_PA_HIGH);
  myRadio.openReadingPipe(1,pipes[1]);
  myRadio.startListening();
}

void loop()
{
  myRadio.write(&data, sizeof(data)); 
  Serial.print("\nPackage:");
  Serial.print(data.id);
  Serial.print("\n");
  Serial.println(data.temperature);
  Serial.println(data.text);
  data.id = data.id + 1;
  data.temperature = data.temperature+0.1;
  delay(50);
}
