///GND - GND                   ctrl + shift + p para abrir comandos
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


#define RAD_A_DEG = 57.295779
#define PIN_VRx A0
#define PIN_VRy A1
#define PIN_SW A2
#define PIN_VRx2 A7
#define PIN_VRy2 A6
#define PIN_SW2 A3

const int mpuAddress = 0x68;  // Puede ser 0x68 o 0x69
MPU6050 mpu(mpuAddress);

const uint64_t pipes[2] = {0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL};
RF24 radio(9, 10);

const int rs = 3, en = 2, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


typedef struct displayDef{
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

displayDef displaydata;

int ax, ay, az;
int gx, gy, gz;
const int button = 8;
int ESTADObut;
int ESTADOultbut = 0;
int correc = 0;
int x, y;
int x2, y2;
int x_ang, y_ang;
int x_ang2, y_ang2;

int _success = 0;
int _fail = 0;
unsigned long _startTime = 0;

long tiempo_prev;
float dt;
float ang_x, ang_y, ang_z;
float mapeo_ang_x, mapeo_ang_y;
float ang_x_prev, ang_y_prev, ang_z_prev;

void setup()
{
   Serial.begin(115200);
   Wire.begin();
   mpu.initialize();
   pinMode(button, INPUT);
   pinMode( PIN_SW, INPUT_PULLUP );
   pinMode( PIN_SW2, INPUT_PULLUP );
 
   lcd.begin(16, 2);
   lcd.print("RC controller");

   //radio
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

   //valores iniciales data
   displaydata.ESTADObut = 0;
   displaydata.PIN_SW = 0;
   displaydata.PIN_SW2 = 0;
   displaydata.x_ang = 127;
   displaydata.y_ang = 127;
   displaydata.x_ang2 = 127;
   displaydata.y_ang2 = 127;
   displaydata.ang_x = 127;
   displaydata.ang_y = 127;
}

void serial(){
   Serial.print(F("Rotacion en X:  "));
   Serial.print(ang_x);
   Serial.print(F("\t Rotacion en Y: "));
   Serial.print(ang_y);
   //Serial.print(F("\t Rotacion en Z: "));
   //Serial.println(ang_z);
   
   Serial.print("\t X_ang:" );
   Serial.print(x_ang);
   Serial.print("  ");
   Serial.print( "Y_ang:" );
   Serial.print(y_ang);
   Serial.print( " Botjoy:");
   Serial.print( !digitalRead(PIN_SW) );
   Serial.print( "   X_ang2:" );
   Serial.print(x_ang2);
   Serial.print("  ");
   Serial.print( "Y_ang2:" );
   Serial.print(y_ang2);
   Serial.print( " Botjoy2:");
   Serial.print( !digitalRead(PIN_SW2) );
 
   
   Serial.print(F("  Botón switch: "));
   
  
}


void loop() {
   // Leer las aceleraciones y velocidades angulares
   mpu.getAcceleration(&ax, &ay, &az);
   mpu.getRotation(&gx, &gy, &gz);
   
   
   updateFiltered();
   joy();
   serial();
   
   ESTADObut = digitalRead(button);
   if (ESTADObut == HIGH){
    Serial.print("Encendido");
    ESTADObut = 1;
    
    // delay(100);
    }

   if (ESTADObut == LOW){
    Serial.print("apagado");
    ESTADObut = 0;
    delay(10);
   }
 

   

   // comunicación radio
   unsigned long loop_start = millis();
    // printf("Now sen ding %i...", controlPak.steering);

    // First, stop listening so we can talk.
    //radio.stopListening();
    //if (!radio.write( &controldata, sizeof(controldata) )) {
      // Serial.println(F("failed."));
    //}
    //radio.startListening();
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
      Serial.print(" but ");
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
 // if ( radio.available())
    {
     // radio.read(&controldata, sizeof(controldata));

      

      // Keep listening in case of transmitter is retransmitting the message
      // The delay should be enough to cover all the possible retries
      unsigned long wait_for_ready_start = millis();
      while ((millis() - wait_for_ready_start) < 100)
      {
        if (radio.available())
        {
          //controlDef dataDummy;
          // Get (and discard) the answer data
          //radio.read(&dataDummy, sizeof(dataDummy));
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

   displaydata.ESTADObut = ESTADObut;
   displaydata.PIN_SW = PIN_SW;
   displaydata.PIN_SW2 = PIN_SW2;
   displaydata.x_ang = x_ang;
   displaydata.y_ang = y_ang;
   displaydata.x_ang2 = x_ang2;
   displaydata.y_ang2 = y_ang2;
   displaydata.ang_x = mapeo_ang_x;
   displaydata.ang_y = mapeo_ang_y;



   
}

void joy(){
  x = analogRead(PIN_VRx);
  y = analogRead(PIN_VRy);
 
  // mapear los valores a grados
  x_ang = map( x, 0, 1023, 0, 255 );
  y_ang = map( y, 0, 1023, 0, 255 );

  
  x2 = analogRead(PIN_VRx2);
  y2= analogRead(PIN_VRy2);
 
  // mapear los valores a grados
  x_ang2 = map( x2, 0, 1023, 0, 255 );
  y_ang2 = map( y2, 0, 1023, 0, 255 );
 
}

void updateFiltered()
{
   dt = (millis() - tiempo_prev) / 1000.0;
   tiempo_prev = millis();

   //Calcular los ángulos con acelerometro
   float accel_ang_x = atan(ay / sqrt(pow(ax, 2) + pow(az, 2)))*(180.0 / 3.14);
   float accel_ang_y = atan(-ax / sqrt(pow(ay, 2) + pow(az, 2)))*(180.0 / 3.14);

   //Calcular angulo de rotación con giroscopio y filtro complementario
   ang_x = 0.98*(ang_x_prev + (gx / 131)*dt) + 0.02*accel_ang_x;
   ang_y = 0.98*(ang_y_prev + (gy / 131)*dt) + 0.02*accel_ang_y;
   //ang_z = ang_z_prev + ((gy / 131)*dt);

   ang_x_prev = ang_x;
   ang_y_prev = ang_y;
   //ang_z_prev = ang_z; 

   mapeo_ang_x = map(ang_x, -90, +90, 255, 0);
   mapeo_ang_y = map(ang_y, -90, +90, 0, 255);
}
