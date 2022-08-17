#include "DHT.h"

#define DHTPIN      2
#define DHTTYPE     DHT11

DHT dht(DHTPIN, DHTTYPE);

float humid;
float temp;

void setup(){
    Serial.begin(115200);
    Serial.println("JD Edu Smart Farm Start ... ");
    dht.begin();
}

void loop(){

    humid = dht.readHumidity();
    temp = dht.readTemperature();

    if(isnan(humid) || isnan (temp)){
        Serial.println("Failed to read DHT");
        humid = 0;
        temp = 0;
    }else{
        Serial.print("Humidity: ");
        Serial.println(humid);
        Serial.print("Temperature: ");
        Serial.println(temp);
    }
    delay(500);
}

