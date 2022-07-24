#include <DHT.h>
#define Type DHT11
int DHTpin = 0;
float humid, tempC, tempF;
DHT HT(DHTpin, Type);
//Coordinated Configuration:
int x0 = 8;

void getTemp_Display()
{
  humid = HT.readHumidity();
  tempC = HT.readTemperature();
  tempF = HT.readTemperature(true);
}
