#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define buz 16
#define OLED_RESET    -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#if (defined(_AVR) || defined(ESP8266)) && !defined(AVR_ATmega2560_)
SoftwareSerial mySerial(14, 12); // SoftwareSerial for UNO and similar
#else
#define mySerial Serial1 // HardwareSerial for others
#endif

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(100);
  Serial.println("\n\nDelete Fingerprint");

  finger.begin(57600);
  pinMode(buz,OUTPUT);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1);
  }

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(2000);
  display.clearDisplay();
}

void loop() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println("Place finger to verify");

  uint8_t id = readnumber();

  if (id == 0) {
    display.clearDisplay();
    display.println("Invalid ID, try again");
    display.display();
    delay(2000);
    return;
  }

  display.clearDisplay();
  display.println("Verifying...");
  display.display();

  if (verifyFingerprint(id)) {
    display.clearDisplay();
    display.println("Verification successful");
    display.display();
    delay(1000);
            digitalWrite(buz, HIGH);
        delay(150);
        digitalWrite(buz, LOW);
    display.clearDisplay();
    deleteFingerprint(id);
  } else {
    display.clearDisplay();
    display.println("Finger mismatch error");
    display.display();
    delay(2000);
  }
}

uint8_t readnumber(void) {
  uint8_t num = 0;
  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

bool verifyFingerprint(uint8_t id) {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  {
    return false;
  }

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  {
    return false;
  }

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  {
    return false;
  }

  if (finger.fingerID != id) {
    return false;
  }

  return true;
}

uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = finger.deleteModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
    return FINGERPRINT_OK;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
  }
  return p;
}
