// A complete rewrite of the Examin Handset code, for board version 1.6

// The following libraries are definitely required
#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Wire.h>
#include <Adafruit_SSD1327.h>
// ToDo - check if these are required
#include <stdio.h> 
#include "driver/rtc_io.h"
#include "string.h"

//  Local Includes
#define CURRENT_LOG_LEVEL LOG_LEVEL_DEBUG 
#include "DebugLogger.h"


// Define states (you'll need to replace these with meaningful values)
enum State {
    IDLE = 0,
    INITIALISE = 1,
    LOW_VOLTAGE = 2,
    SHUTDOWN = 3,
    WAIT_FOR_BREATH = 4,
    BREATH = 5,
    MOUTHPIECE_MISSING = 6,
    WAIT_AFTER_BREATH_1 = 7,
    MEASURE_1 = 8,
    WAIT_AFTER_BREATH_2 = 9,
    MEASURE_2 = 10,
    WAIT_AFTER_BREATH_3 = 11,
    MEASURE_3 = 12,
    COMPLETE = 13
};

// Function prototypes (placeholders for now)
bool checkVoltage(); 
bool detectMouthpiece();
void turnOffComponents();
void enterDeepSleep();
// ... (add other functions for measurements, UI updates, etc.)

// Global variables for display
float batteryVoltage = 0.0;
uint8_t date[3] = {0, 0, 0};
uint8_t id[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long dispUpdate = 0;
unsigned long refreshRate = 1000000; // 1 second
unsigned long lv_time = 0;
unsigned long inactive_time = 0;


TwoWire I2C2 = TwoWire(1);
#define BUTTON_PIN          6
#define BUTTON_PIN_BITMASK  0x40





// Declaration for an SSD1327 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // I2C display address for this specific display, check datasheet
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 128 // OLED display height, in pixels


// it looks like SSD1327_SWITCHCAPVCC is no longer defined in the header file
//  assign  it here
#define  SDD1327_SWITCHCAPVCC 0x3C
Adafruit_SSD1327 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


void setup() {
    Serial.begin(115200);
    delay(1000);

    LOG_INFO("Starting setup");
    Serial.println("Good morning, Dave");

}

// Example usage
void loop() {
//   Serial.println('Do I know you?');
  int sensorValue = analogRead(A0);
  logMessage(LOG_LEVEL_INFO, "Sensor reading: %d", sensorValue);
  delay(10000);
  // ...
}



// Function to initialize the display
void initDisplay() {
    if (!display.begin(SDD1327_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1327 allocation failed"));
        for (;;); // Don't proceed, loop forever
    }

    // Clear display buffer and set the background color to black
    display.clearDisplay();
    display.fillScreen(SSD1327_BLACK);
}

// Function to draw the UI elements
void drawUI() {
    // Set text color
    display.setTextColor(SSD1327_WHITE);
    display.setCursor(5, 15);
    display.setTextSize(1);
    display.print("Batt: ");
    display.print(batteryVoltage/1000.0);
    display.print("V");
    // Display battery voltage

    display.setCursor(75, 15);
    // Display current time
    display.print(date[0]);
    display.print("-");
    display.print(date[1]);
    display.print("-");
    display.print(date[2]);

    display.setCursor(0, 120);
    display.print("ID:");

    for(int i = 0; i < 9; i++){
        display.print(id[i]);
    }

    display.drawRoundRect(5, 25, 117, 90, 10, SSD1327_WHITE);
}
