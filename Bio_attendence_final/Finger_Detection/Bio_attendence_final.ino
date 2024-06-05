#include <Adafruit_Fingerprint.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Adafruit_SSD1306.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>


#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(14, 12);
#else
#define mySerial Serial1
#endif

#define buz 16

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// WiFi credentials
const char* ssid = "Naveen";
const char* password = "9553287950";

// Google Sheets script URL
String serverName = "https://script.google.com/macros/s/AKfycbxGfQf9NoNtuf6KNynVebPVXOeW1SbLi8F4qtREIjpbAIdIbiglGCbM6RqmrlR6YeCo/exec";

// Define fingerprint ID to name mappings
// Add additional IDs as needed
#define RAMU 9
#define HEMANTH 1
#define NAVEEN 12
#define YUKTHI 11
#define JAGGU 13

// Timing variables
unsigned long lastTime = 0;
unsigned long timerDelay = 1000; 
//  digitalWrite(buz,LOW);

void setup() {
  pinMode(buz,OUTPUT);
  digitalWrite(buz,LOW);
  Serial.begin(9600);
  while (!Serial);
  delay(100);


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(1000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
        display.clearDisplay();
      display.setTextSize(1.5);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0,0);
      display.println("WiFi Connected ");

      display.display();
  Serial.println(WiFi.localIP());

  // Initialize fingerprint sensor
  Serial.println("\n\nAdafruit finger detect test");
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  // Read sensor parameters
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  } else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}

void loop() {
  // Check if 2 seconds have passed
  if ((millis() - lastTime) > timerDelay) {
    int id = getFingerprintID();
    if (id > -1) {
      // Map ID to name
      String name;
      if (id == RAMU) {
        name = "RAMU";
      } else if (id == HEMANTH) {
        name = "Hemanth";
      } else if (id == NAVEEN) {
        name = "Naveen";
      } else if (id == JAGGU) {
        name = "JAGGU";
      } else {
        name = "Unknown";
      }

      if (name != "Unknown") { // Check if the fingerprint is recognized
        // Display name and status on OLED
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(SSD1306_WHITE);
        
        // Name column
        display.setCursor(0, 0);
      
        display.println(" "+name );
        
        // Status column

        display.setCursor(125, 0);
        display.println("\n");
        display.println(" Verified ");

        display.display();
        digitalWrite(buz, HIGH);
        delay(150);
        digitalWrite(buz, LOW);

        // Send data to Google Sheets
        if (WiFi.status() == WL_CONNECTED) {
          WiFiClientSecure client;
          client.setInsecure(); // Bypass SSL verification for testing

          HTTPClient http;

          String serverPath = serverName + "?value1=" + String(id) + "&value2=" + name;

          // Send HTTP GET request
          http.begin(client, serverPath.c_str());
          int httpResponseCode = http.GET();

          if (httpResponseCode > 0) {
            Serial.print("HTTP Response code: ");
            Serial.println(httpResponseCode);
            String payload = http.getString();
            Serial.println(payload);
          } else {
            Serial.print("Error code: ");
            Serial.println(httpResponseCode);
          }
          http.end();
        } else {
          Serial.println("WiFi Disconnected");
        }
      }
    } else {
      // Display status on OLED
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      
      // Status column

      display.setCursor(0, 0);
      display.println(" No Finger");
            display.setCursor(125, 0);
            display.println("\n");
      display.println("  Found  ");

      display.display();
    }
    // Update the last time the GET request was sent
    lastTime = millis();
  }
}


int getFingerprintID() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) {
    if (p == FINGERPRINT_NOTFOUND) {
      Serial.println("No match found");
    } else {
      Serial.println("Unknown error");
    }
    return -1;
  }

  // Found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}
