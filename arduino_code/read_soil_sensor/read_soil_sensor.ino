#define SOIL_A0     A1
#define SOIL_D0     6

void setup(){
    Serial.begin(115200);
    Serial.println("JD Edu Smart Farm Start ... ");

    pinMode(SOIL_A0, INPUT);
    pinMode(SOIL_D0, INPUT);
}

void loop(){
    int soil_a0 = analogRead(SOIL_A0);
    int soil_d0 = digitalRead(SOIL_D0);

    Serial.print("Soil sensor(digital): ");
    Serial.println(soil_d0);
    Serial.print("Soil sensor(analog): ");
    Serial.println(soil_a0);

    delay(100);

}