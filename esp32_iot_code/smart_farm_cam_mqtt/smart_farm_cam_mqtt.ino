/***************************************
 * JD Edu  
 * ESP32 - image and data MQTT 
 * using MQTT with Tworunsoft 
 * hardware: Lily-go V162 camera 
 * Date: 2022/8/18
 **************************************/
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "select_pins.h"
#include "esp_camera.h"

#define SDA 15
#define SCL 14

#define RXD2 5 //5
#define TXD2 12 //12

String macAddress = "";
String ipAddress = "";

// Replace the next variables with your SSID/Password combination
const char* ssid = "GBSA0001";
const char* password = "GBSA0001";

// Depend OLED library ,See  https://github.com/ThingPulse/esp8266-oled-ssd1306
#include "SSD1306.h"
#include "OLEDDisplayUi.h"
#define SSD1306_ADDRESS 0x3c
SSD1306 oled(SSD1306_ADDRESS, I2C_SDA, I2C_SCL, (OLEDDISPLAY_GEOMETRY)SSD130_MODLE_TYPE);
OLEDDisplayUi ui(&oled);

//When using timed sleep, set the sleep time here
#define uS_TO_S_FACTOR 1000000              /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5                    /* Time ESP32 will go to sleep (in seconds) */

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

bool deviceProbe(uint8_t addr)
{
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}

bool setupDisplay()
{
    static FrameCallback frames[] = {
        [](OLEDDisplay * display, OLEDDisplayUiState * state, int16_t x, int16_t y)
        {
            display->setTextAlignment(TEXT_ALIGN_CENTER);
            display->setFont(ArialMT_Plain_10);
            display->drawString(64 + x, 9 + y, macAddress);
            display->drawString(64 + x, 25 + y, ipAddress);
            if (digitalRead(AS312_PIN)) {
                display->drawString(64 + x, 40 + y, "AS312 Trigger");
            }
        },
        [](OLEDDisplay * display, OLEDDisplayUiState * state, int16_t x, int16_t y)
        {
            display->setTextAlignment(TEXT_ALIGN_CENTER);
            display->setFont(ArialMT_Plain_10);
            display->drawString( 64 + x, 5 + y, "Camera Ready! Use");
            display->drawString(64 + x, 25 + y, "http://" + ipAddress );
            display->drawString(64 + x, 45 + y, "to connect");
        }
    };

    if (!deviceProbe(SSD1306_ADDRESS))return false;
    oled.init();
    Wire.setClock(100000);  //! Reduce the speed and prevent the speed from being too high, causing the screen
    oled.setFont(ArialMT_Plain_16);
    oled.setTextAlignment(TEXT_ALIGN_CENTER);
    delay(50);
    oled.drawString( oled.getWidth() / 2, oled.getHeight() / 2 - 10, "LilyGo CAM");
    oled.display();
    ui.setTargetFPS(30);
    ui.setIndicatorPosition(BOTTOM);
    ui.setIndicatorDirection(LEFT_RIGHT);
    ui.setFrameAnimation(SLIDE_LEFT);
    ui.setFrames(frames, sizeof(frames) / sizeof(frames[0]));
    ui.setTimePerFrame(6000);
    ui.disableIndicator();
    return true;
}

void loopDisplay()
{
    if (ui.update()) {
        //button.tick();
    }
}

bool setupCamera()
{
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sscb_sda = SIOD_GPIO_NUM;
    config.pin_sscb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    //init with high specs to pre-allocate larger buffers
    if (psramFound()) {
        config.frame_size = FRAMESIZE_UXGA;
        config.jpeg_quality = 10;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.jpeg_quality = 12;
        config.fb_count = 1;
    }
    pinMode(13, INPUT_PULLUP);
    pinMode(14, INPUT_PULLUP);

    // camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }

    sensor_t *s = esp_camera_sensor_get();
    //initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1);//flip it back
        s->set_brightness(s, 1);//up the blightness just a bit
        s->set_saturation(s, -2);//lower the saturation
    }
    //drop down frame size for higher initial frame rate
    s->set_framesize(s, FRAMESIZE_QVGA);
    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);
    return true;
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

void setup() {
    bool status;
    Serial.begin(115200);
    
    SecondSerial.begin(38400, SERIAL_8N1, RXD2, TXD2);
    //Delay for SecondSerial 
    delay(2000);

    setup_wifi();
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    status = setupCamera();
    Serial.print("setupCamera status "); Serial.println(status);
    if (!status) {
        delay(10000); esp_restart();
    }

  
}

const int MAX_PAYLOAD = 60000;
void sendMQTT(const uint8_t * buf, uint32_t len){
  Serial.println("Sending picture...");
  if(len>MAX_PAYLOAD){
    Serial.print("Picture too large, increase the MAX_PAYLOAD value");
  }else{
    Serial.print("Picture sent ? : ");
    Serial.println(client.publish("/politek/signal_gather_topic/uuid_9807", buf, len, false));
  } 
}


int cool = 0;

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get(); // used to get a single picture.
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }
    Serial.println(fb->len);
    Serial.println(" Picture taken");
    sendMQTT(fb->buf, fb->len);
    esp_camera_fb_return(fb); // must be used to free the memory allocated by esp_camera_fb_get().
    
    delay(2000);

  // Arduino Software Serial: rx 3 tx 8
  // Arduino protocol 
  // 'a' + temperature + 'b' + humid + 'c' + CDS A0 + 'd' + Soil A0 + 'e'
  /*String inString = "";
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
  */
  // Send MQTT data 
  // Send value to MQTT server 
  cool++;
  if(cool > 10 ){    
    Serial.println("Send data to MQTT server");
    char tempString[8];
    char humString[8];
    char soilString[8];
    char cdsString[8];
    dtostrf(humidity, 1, 2, humString);
    dtostrf(temperature, 1, 2, tempString);
    dtostrf(soil_a0, 1, 2, soilString);
    dtostrf(cds_a0, 1, 2, cdsString);
    sprintf(msg, "{'uuid':'9807', 'temperature': '%s', 'humidity': '%s'%}", tempString, humString);
    client.publish("/politek/signal_gather_topic/uuid_9807", msg);
    // Convert the value to a char array
    cool = 0;
  }   
  delay(1000);
}