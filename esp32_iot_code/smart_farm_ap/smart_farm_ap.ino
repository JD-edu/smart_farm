#include <WiFi.h>
#include <Wire.h>
#include <U8x8lib.h>

// I2C bus 
#define SDA 15
#define SCL 14

// Serial port 2 
#define RXD2 5
#define TXD2 12

// WiFi SSDI and password 
const char* ssid     = "ESP32-Access-Point";
const char* password = "123456789";

// OLED initialize 
U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(/* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);   // OLEDs without Reset of the Display

// SEcond seerial intialize
HardwareSerial SecondSerial(2);

// Web server initialize - Set web server port number to 80
WiFiServer server(80);

// Sensor data variables
float temperature = 0;
float humidity = 0;
float cds_a0 = 0;
float soil_a0 = 0; 

// Variable to store the HTTP request
String header;

void OLED_display(){
  String inString = "";
  Serial.println("OLED_dispaly"); 
  if(SecondSerial.available()>0){
      if(SecondSerial.available()){
        Serial.println("Serial !!"); 
        inString = SecondSerial.readStringUntil('\n');
        //Serial.println(inString);
        // temperature
        int first = inString.indexOf('a');
        int second = inString.indexOf('b');
        String tempStr = inString.substring(first+1, second);
        temperature = tempStr.toFloat();
        //Serial.println(temperature);

        // humidity
        first = inString.indexOf('b');
        second = inString.indexOf('c');
        String humidStr = inString.substring(first+1, second);
        humidity = humidStr.toFloat();
        //Serial.println(humidity);

        // CDS analog 
        first = inString.indexOf('c');
        second = inString.indexOf('d');
        String cds_a0Str = inString.substring(first+1, second);
        cds_a0 = cds_a0Str.toFloat();
        //Serial.println(cds_a0);

        // soil analog 
        first = inString.indexOf('d');
        second = inString.indexOf('e');
        String soil_a0Str = inString.substring(first+1, second);
        soil_a0 = soil_a0Str.toFloat();
        //Serial.println(soil_a0);
        
        // Display value on OLED 
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

        delay(100);
    } 
  } 
}

void setup() {
  // Start serial 
  Serial.begin(115200);
  SecondSerial.begin(38400, SERIAL_8N1, RXD2, TXD2);

  // AP and server start 
  Serial.print("Setting AP (Access Point)…");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  server.begin();

  // OLED start 
  u8x8.begin();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_chroma48medium8_r); 
  u8x8.drawString(0,0,"start");
  delay(1000)
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients
  OLED_display();
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      // If we have data from arduino, read it and display it on OLED 
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 26  
            client.println("<p> Temperature: " + String(temperature) + "</p>");
            // If the output26State is off, it displays the ON button       
            //if (output26State=="off") {
            //  client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            //} else {
            //  client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            //} 
               
            // Display current state, and ON/OFF buttons for GPIO 27  
            //client.println("<p>GPIO 27 - State " + output27State + "</p>");
            // If the output27State is off, it displays the ON button       
            //if (output27State=="off") {
            //  client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
            //} else {
            //  client.println("<p><a href=\"/27/off\"><button class=\"button button2\">OFF</button></a></p>");
            //}
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}