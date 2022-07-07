#define FAN_OUT      9

void setup(){
    Serial.begin(115200);
    Serial.println("JD Edu Smart Farm Start ... ");
    pinMode(FAN_OUT, OUTPUT);
}

void loop(){
    if(Serial.available()>0){
        char c = Serial.read();
        if (c == 't'){
            Serial.println("fan on");
            digitalWrite(FAN_OUT, HIGH);
        }else if(c == 's'){
            Serial.println("fan off");
            digitalWrite(FAN_OUT, LOW);
        }
    }
}