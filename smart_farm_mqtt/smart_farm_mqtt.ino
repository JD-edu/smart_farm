#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <U8x8lib.h>

#define SDA 15
#define SCL 14

#define RXD2 5 //5
#define TXD2 12 //12

U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display
// Replace the next variables with your SSID/Password combination
const char* ssid = "GBSA0001";
const char* password = "GBSA0001";

// Add your MQTT Broker IP address, example:
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "3.34.187.90";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[100];
int value = 0;

HardwareSerial SecondSerial(2);

float temperature = 0;
float humidity = 0;
float cds_a0 = 0;
float soil_a0 = 0;

void setup() {
  Serial.begin(115200);
  
  SecondSerial.begin(38400, SERIAL_8N1, RXD2, TXD2);
  //Delay for SecondSerial 
  delay(2000);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  u8x8.begin();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_chroma48medium8_r); 
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("JD_edu", "scott", "test@1234!")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("/politek/signal_gather_topic/uuid_9807");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
int cool = 0;
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Arduino Software Serial: rx 3 tx 8
  // Arduino protocol 
  // 'a' + temperature + 'b' + humid + 'c' + CDS A0 + 'd' + Soil A0 + 'e'
  String inString = "";
  String tempStr = "";
  String humidStr = "";
  String cds_a0Str = "";
  String soil_a0Str = "";
  if(SecondSerial.available()>0){
    if(SecondSerial.available()){

      inString = SecondSerial.readStringUntil('\n');
      Serial.println(inString);
      // temperature
      int first = inString.indexOf('a');
      int second = inString.indexOf('b');
      tempStr = inString.substring(first+1, second);
      temperature = tempStr.toFloat();
      //Serial.println(temperature);

      // humidity
      first = inString.indexOf('b');
      second = inString.indexOf('c');
      humidStr = inString.substring(first+1, second);
      humidity = humidStr.toFloat();
      //Serial.println(humidity);

      // CDS analog 
      first = inString.indexOf('c');
      second = inString.indexOf('d');
      cds_a0Str = inString.substring(first+1, second);
      cds_a0 = cds_a0Str.toFloat();
      //Serial.println(cds_a0);

      // soil analog 
      first = inString.indexOf('d');
      second = inString.indexOf('e');
      soil_a0Str = inString.substring(first+1, second);
      soil_a0 = soil_a0Str.toFloat();
      //Serial.println(soil_a0);
    } 

  }

  if(Serial.available()>0){
    if(Serial.available()){
      inString = Serial.readStringUntil('\n');
      if(inString[0] == 'c'){
        SecondSerial.print("c");
      }else if(inString[0] == 'f'){
        SecondSerial.print("f");
      }else if(inString[0] == 'i'){
        SecondSerial.print("i");
      }else if(inString[0] == 'k'){
        SecondSerial.print("k");
      }else if(inString[0] == 't'){
        SecondSerial.print("t");
      }else if(inString[0] == 's'){
        SecondSerial.print("s");
      }
    }
  }

  // Send MQTT data 
  // Send value to MQTT server 
  cool++;
  if(cool > 10 ){    
    char tempString[8];
    char humString[8];
    char soilString[8];
    char cdsString[8];
    dtostrf(humidity, 1, 2, humString);
    dtostrf(temperature, 1, 2, tempString);
    dtostrf(soil_a0, 1, 2, soilString);
    dtostrf(cds_a0, 1, 2, cdsString);
    u8x8.drawString(0,0,humString);
    u8x8.drawString(0,1,tempString); 
    u8x8.drawString(0,2,soilString);
    u8x8.drawString(0,3,cdsString); 
    sprintf(msg, "{'uuid':'9807', 'temperature': '%s', 'humidity': '%s'%}", tempString, humString);
    client.publish("/politek/signal_gather_topic/uuid_9807", msg);
    //client.publish("esp32/humidity", humidStr);
    // Convert the value to a char array
    cool = 0;
  }    
  delay(100);
}