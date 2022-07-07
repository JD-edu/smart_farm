#define CDS_A0      A0
#define CDS_D0      4  

void setup(){
    Serial.begin(115200);
    Serial.println("JD Edu Smart Farm Start ... ");

    pinMode(CDS_A0, INPUT);
    pinMode(CDS_D0, INPUT);
}

void loop(){
    int cds_a0 = analogRead(CDS_A0);
    int cds_d0 = digitalRead(CDS_D0);

    Serial.print("Light sensor(digital): ");
    Serial.println(cds_d0);
    Serial.print("Light sensor(analog): ");
    Serial.println(cds_a0);

    delay(100);

}