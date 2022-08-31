#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "arduino_secrets.h" //Network SSID/Password safe storage.

char ssid[] = SECRET_SSID;  // your network SSID (name) 
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP) 
int keyIndex = 0;           // your network key index number (needed only for WEP)

int status = WL_IDLE_STATUS;
WiFiServer server(80);

int buttonState = 0;         // variable for reading the pushbutton status

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, INPUT);

  Serial.begin(9600);      // initialize serial communication
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(2000);
  display.clearDisplay();
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }



  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // print the network name (SSID);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }


  server.begin();                           // start the web server on port 80
  printWifiStatus();                        // you're connected now, so print out the status

  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  // Display static text
  display.println("SSID: " + (String) ssid);
  display.display();

}



void loop() {
  buttonState = digitalRead(5);
  if (buttonState == HIGH) {
    // turn LED on:
    digitalWrite(2, LOW);
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
  }
  WiFiClient client = server.available();   // listen for incoming clients
  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    String ledState = "";
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        // Check to see if the client request was /X
        if (currentLine.endsWith("GET /RH")) {
          digitalWrite(2, HIGH);
          ledState = "Red LED turned on.";
        }
        if (currentLine.endsWith("GET /RL")) {
          digitalWrite(2, LOW);
          ledState = "Red LED turned off";
        }
        if (currentLine.endsWith("GET /BH")) {
          digitalWrite(3, HIGH);
          ledState = "Blue LED turned on";
        }
        if (currentLine.endsWith("GET /BL")) {
          digitalWrite(3, LOW);
          ledState = "Blue LED turned off";
        }
        if (currentLine.endsWith("GET /OH")) {
          digitalWrite(4, HIGH);
          ledState = "Orange LED turned on";
        }
        if (currentLine.endsWith("GET /OL")) {
          digitalWrite(4, LOW);
          ledState = "Orange LED turned off";
        }
        if (currentLine.indexOf("?ip") > 0) {
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(WHITE);
          display.setCursor(0, 0);
          // Display static text
          display.println("IP: " + currentLine.substring(currentLine.indexOf("=") + 1, currentLine.indexOf(" H")));
          display.println("\n" + ledState);
          display.display();
        }

        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:application/json");
            client.println();
            DynamicJsonDocument jBuffer(1024);
            jBuffer["LED_UPDATE"] = ledState;
            jBuffer["Red"] = digitalRead(2) == HIGH ? "On" : "Off";
            jBuffer["Blue"] = digitalRead(3) == HIGH ? "On" : "Off";
            jBuffer["Orange"] = digitalRead(4) == HIGH ? "On" : "Off";
            serializeJson(jBuffer , client);

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}
