#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

RF24 radio(9, 10); // CE, CSN
const byte addresses[][6] = {"00001", "00002"};

unsigned long currentMillis;
long previousMillis = 0;    // set up timers
long interval = 10;        // time constant for timer
unsigned long count;
unsigned long remoteMillis;

long previousInterpMillis = 0;    // set up timers
int interpFlag = 0;

//**************remote control****************
struct RECEIVE_DATA_STRUCTURE {
  //put your variable definitions here for the data you want to send
  //THIS MUST BE EXACTLY THE SAME ON THE OTHER ARDUINO
  int16_t menuDown;
  int16_t Select;
  int16_t menuUp;
  int16_t toggleBottom;
  int16_t toggleTop;
  int16_t toggle1;
  int16_t toggle2;
  int16_t mode;
  int16_t RLR;
  int16_t RFB;
  int16_t RT;
  int16_t LLR;
  int16_t LFB;
  int16_t LT;
};

RECEIVE_DATA_STRUCTURE mydata_remote;

int toggleTopOld;
int remoteState;
int remoteStateOld;

void setup() {
  Serial.begin(115200);
  
  radio.begin();
  radio.openWritingPipe(addresses[0]); // 00002
  radio.openReadingPipe(1, addresses[1]); // 00001
  radio.setPALevel(RF24_PA_MIN);

  radio.startListening();
}

void loop() {
  
  
  currentMillis = millis();
  if (currentMillis - previousMillis >= 10) {  // start timed event

    previousMillis = currentMillis;


    // check for radio data
    if (radio.available()) {
      radio.read(&mydata_remote, sizeof(RECEIVE_DATA_STRUCTURE));
      remoteMillis = currentMillis;
      
    }

    // is the remote disconnected for too long ?
    if (currentMillis - remoteMillis > 500) {
      remoteState = 0;
    }
    else {
      remoteState = 1;
      Serial.println("ok");
    }  
    // display remote connection state
    if (remoteState != remoteStateOld) {
      if (remoteState == 0) {
        lcd.setCursor(13, 0);
        lcd.print("N");
      }
      else if (remoteState == 1) {
        lcd.setCursor(13, 0);
        lcd.print("C");
      }
    }
    remoteStateOld = remoteState;

  }

}
