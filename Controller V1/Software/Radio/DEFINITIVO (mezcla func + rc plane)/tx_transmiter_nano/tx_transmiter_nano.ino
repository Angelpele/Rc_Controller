#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <LiquidCrystal.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


const uint64_t pipes = "0xF0F0F0F0D2LL";
#define DATARATE RF24_2MBPS
RF24 radio(9, 10);

const int mpuAddress = 0x68;  // Puede ser 0x68 o 0x69
MPU6050 mpu(mpuAddress);
#define RAD_A_DEG = 57.295779

const int rs = 3, en = 2, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

int Pin1 = A2;
int Pin2 = A3;
int button = 8;
int ESTADObut = 0;

int ax, ay, az;
int gx, gy, gz;
unsigned long tiempo_prev, prevtime;
float dt;
float ang_x, ang_y, ang_z;
float mapeo_ang_x, mapeo_ang_y;
float ang_x_prev, ang_y_prev, ang_z_prev;
int timer_off = 0;

// estructura de los 9 canales del radio
struct canales_radio {
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

canales_radio enviar;

void setup()
{
  Serial.begin(57600);
  pinMode(button, INPUT);
  pinMode(Pin1, INPUT_PULLUP );
  pinMode(Pin2, INPUT_PULLUP );
  Wire.begin();
  mpu.initialize();
  lcd.begin(16,2);
  //lcd.setCursor(4,2);
  lcd.print("  Mando rc ");
  //lcd.setCursor(1,1);
  //lcd.print("full distance");

  radio.begin();
  radio.openWritingPipe(pipes);
  radio.setDataRate( DATARATE ) ;
  radio.setPALevel( RF24_PA_MAX ) ;
  radio.setChannel(0x34);
  radio.enableDynamicPayloads() ;
  radio.enableAckPayload();               // not used here
  radio.setRetries(0, 15);                // Smallest time between retries, max no. of retries
  radio.setAutoAck( true ) ;
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging
  radio.powerUp();
  radio.stopListening();
} 

void loop(){ 
  
  if (millis() - prevtime >= 25){  //It will only execute if a min delay has passed
    /*Ejemplo de canal normal:
    enviar.aleron1 = map( analogRead(A2), 0, 1024, 0, 255);
    Ejemplo canal invertido:
    enviar.aleron1 = map( analogRead(A2), 0, 1024, 255, 0);  */
  
    mpu.getAcceleration(&ax, &ay, &az);
    mpu.getRotation(&gx, &gy, &gz);
    
    updateFiltered();
    serial();

    ESTADObut = digitalRead(button);
    if (ESTADObut == HIGH){
      Serial.println("Encendido");
      ESTADObut = 1;
      
      // delay(100);
      }

    if (ESTADObut == LOW){
      Serial.println("apagado");
      ESTADObut = 0;
      //delay(10);
    }
    if(!digitalRead(Pin1) == 1 && !digitalRead(Pin2)==1){
      timer_off++;
    }
    if(timer_off > 50){
      lcd.noDisplay();
    }

    enviar.x_ang = map( analogRead(A0), 0, 1023, 255, 0 );
    enviar.y_ang = map( analogRead(A1), 0, 1023, 0, 255 );
    enviar.x_ang2 = map( analogRead(A7), 0, 1023, 255, 0 );
    enviar.y_ang2 = map( analogRead(A6), 0, 1023, 0, 255 );
    enviar.PIN_SW = !digitalRead(Pin1);
    enviar.PIN_SW2 = !digitalRead(Pin2);
    enviar.ang_x = mapeo_ang_x;
    enviar.ang_y = mapeo_ang_y;
    enviar.ESTADObut = ESTADObut;
    
    radio.write(&enviar, sizeof(canales_radio));
    prevtime = millis();
  }
}


void serial(){
   Serial.print(F("Rotacion en X:  "));
   Serial.print(enviar.ang_x);
   Serial.print(F("\t Rotacion en Y: "));
   Serial.print(enviar.ang_y);
   //Serial.print(F("\t Rotacion en Z: "));
   //Serial.println(ang_z);
   
   Serial.print("\t X_ang:" );
   Serial.print(enviar.x_ang);
   Serial.print("  ");
   Serial.print( "Y_ang:" );
   Serial.print(enviar.y_ang);
   Serial.print( " Botjoy:");
   Serial.print(enviar.PIN_SW);
   Serial.print( "   X_ang2:" );
   Serial.print(enviar.x_ang2);
   Serial.print("  ");
   Serial.print( "Y_ang2:" );
   Serial.print(enviar.y_ang2);
   Serial.print( " Botjoy2:");
   Serial.print(enviar.PIN_SW2);
 
   
   Serial.print(F("  Botón switch: "));

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
