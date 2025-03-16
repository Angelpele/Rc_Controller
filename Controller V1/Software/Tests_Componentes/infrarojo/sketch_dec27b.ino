int LED = 13 ;
int IR = 4; //Entrada digital conectada al sensor infrarrojo

void setup()
{
 pinMode( LED, OUTPUT) ; // LED como salida
 pinMode( IR , INPUT) ; //Sensor infrarrojo como entrada
}

void loop()
{
 int valor = digitalRead(IR) ; //leemos el valor del sensor infrarrojo
 digitalWrite( LED, valor) ;
}
