///GND - GND
//VCC - VCC
//SDA - Pin A4
//SCL - Pin A5

#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <LiquidCrystal.h>
#include <SPI.h>


#define RAD_A_DEG = 57.295779
#define PIN_VRx A0
#define PIN_VRy A1
#define PIN_SW A2
#define PIN_VRx2 A7
#define PIN_VRy2 A6
#define PIN_SW2 A3

const int mpuAddress = 0x68;  // Puede ser 0x68 o 0x69
MPU6050 mpu(mpuAddress);

const int rs = 3, en = 2, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

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

long tiempo_prev;
float dt;
float ang_x, ang_y, ang_z;
float ang_x_prev, ang_y_prev, ang_z_prev;


void setup(){

   Serial.begin(115200);
   Wire.begin();
   mpu.initialize();
   pinMode(button, INPUT);
   pinMode( PIN_SW, INPUT_PULLUP );
   pinMode( PIN_SW2, INPUT_PULLUP );
 
   lcd.begin(16, 2);
   lcd.print("RC controller");


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
   Serial.print("  but_palanca: ");
   Serial.println(ESTADObut);
   

}

void loop() 
{
   // Leer las aceleraciones y velocidades angulares
   mpu.getAcceleration(&ax, &ay, &az);
   mpu.getRotation(&gx, &gy, &gz);
   
   
   updateFiltered();
   joy();
   serial();
   
   ESTADObut = digitalRead(button);
   if (ESTADObut == HIGH){
    //Serial.print("Encendido");
    ESTADObut = 1;
    
    // delay(100);
    }

   if (ESTADObut == LOW){
    //Serial.print("apagado");
    ESTADObut = 0;
    delay(10);
   }
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

   ang_x = map(ang_x, -90, +90, 255, 0);
   ang_y = map(ang_y, -90, +90, 0, 255);
}
