#define LED_ON      8

void setup(){
    Serial.begin(115200);
    Serial.println("JD Edu Smart Farm Start ... ");
    pinMode(LED_ON, OUTPUT);
}

void loop(){
    if(Serial.available()>0){
        char c = Serial.read();
        if (c == 'c'){
            Serial.println("LED ON");
            digitalWrite(LED_ON, HIGH);
        }else if(c == 'f'){
            Serial.println("LED OFF");
            digitalWrite(LED_ON, LOW);
        }
    }
}