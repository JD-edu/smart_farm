#include "DHT.h"
#include<SoftwareSerial.h>

#define DHTPIN      2
#define CDS_A0      A0
#define CDS_D0      4        
#define DHTTYPE     DHT11
#define SOIL_A0     A1
#define SOIL_D0     6
#define PAN_OUT     9
#define PUMP_OUT    5
#define LED_ON      8

const byte rxPin = 3;
const byte txPin = 7;

SoftwareSerial mySerial =  SoftwareSerial(rxPin, txPin);

DHT dht(DHTPIN, DHTTYPE);

void setup(){
    Serial.begin(115200);
    Serial.println("JD Edu Smart Farm Start ... ");

    pinMode(rxPin, INPUT);
    pinMode(txPin, OUTPUT);

    pinMode(CDS_A0, INPUT);
    pinMode(CDS_D0, INPUT);
    pinMode(PAN_OUT, OUTPUT);
    pinMode(PUMP_OUT, OUTPUT);
    pinMode(LED_ON, OUTPUT);

    dht.begin();
    mySerial.begin(38400);
}

int cool = 0;
void loop(){
    if(mySerial.available()>0){
        char c = mySerial.read();
        Serial.println(c);
        if (c == 'c'){
            Serial.println("FAN ON");
            digitalWrite(PAN_OUT, HIGH);
        }else if(c == 'f'){
            Serial.println("FAN OFF");
            digitalWrite(PAN_OUT, LOW);
        }else if(c == 'i'){
            Serial.println("PUMP ON");
            digitalWrite(PUMP_OUT, HIGH);
        }else if(c == 'k'){
            Serial.println("PUMP OFF");
            digitalWrite(PUMP_OUT, LOW);
        }else if(c == 't'){
            Serial.println("LED ON");
            digitalWrite(LED_ON, HIGH);
        }else if(c == 's'){
            Serial.println("LED OFF");
            digitalWrite(LED_ON, LOW);
        }
    }

    if(Serial.available()>0){
        char c = Serial.read();
        if (c == 'c'){
            Serial.println("FAN ON");
            digitalWrite(PAN_OUT, HIGH);
        }else if(c == 'f'){
            Serial.println("FAN OFF");
            digitalWrite(PAN_OUT, LOW);
        }else if(c == 'i'){
            Serial.println("PUMP ON");
            digitalWrite(PUMP_OUT, HIGH);
        }else if(c == 'k'){
            Serial.println("PUMP OFF");
            digitalWrite(PUMP_OUT, LOW);
        }else if(c == 't'){
            Serial.println("LED ON");
            digitalWrite(LED_ON, HIGH);
        }else if(c == 's'){
            Serial.println("LED OFF");
            digitalWrite(LED_ON, LOW);
        }
    }


    float humid = dht.readHumidity();
    float temp = dht.readTemperature();

    if(isnan(humid) || isnan (temp)){
        Serial.println("Failed to read DHT");
    }

    int cds_a0 = analogRead(CDS_A0);
    int cds_d0 = digitalRead(CDS_D0);
    int soil_a0 = analogRead(SOIL_A0);
    int soil_d0 = digitalRead(SOIL_D0);

    String tempStr = String(temp);
    String humidStr = String(humid);
    String cds_a0Str = String(cds_a0);
    String soil_a0Str = String(soil_a0);
    cool++;
    if(cool > 10){
        String packet = 'a'+ tempStr + 'b' + humidStr + 'c' + cds_a0Str + 'd' +soil_a0Str +'e' ;
        Serial.println(packet);
        mySerial.println(packet);
    }
    delay(100);
}