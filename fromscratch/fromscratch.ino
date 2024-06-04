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
#include "HardwareParameters.h"
#define CURRENT_LOG_LEVEL LOG_LEVEL_DEBUG 
#include "DebugLogger.h"
#include "ScreenCode.h"



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

// set currentState to INITIALISE
State currentState = INITIALISE;

// Function prototypes (placeholders for now)
bool checkVoltage(); 
bool detectMouthpiece();
void turnOffComponents();
void enterDeepSleep();
// ... (add other functions for measurements, UI updates, etc.)



// Global variables for display
// ToDo: see if any of these are really needed
float batteryVoltage = 0.0;
uint8_t date[3] = {0, 0, 0};
uint8_t id[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
unsigned long dispUpdate = 0;
unsigned long refreshRate = 1000000; // 1 second
unsigned long lv_time = 0;
unsigned long inactive_time = 0;



bool checkVoltage() {
    // TODO: Implement this function
    return true;
}

bool detectMouthpiece() {
    // TODO: Implement this function
    return true;
}

void turnOffComponents() {
    // TODO: Implement this function
}

void enterDeepSleep() {
    // TODO: Implement this function
}

bool waitForBreath() {
  // TODO: Implement this function
  displayBaseMessage("waiting for", "breath");
  for (;;); // Don't proceed, loop forever
}


TwoWire I2C2 = TwoWire(1);
#define BUTTON_PIN          6
#define BUTTON_PIN_BITMASK  0x40





void setup() {
    Serial.begin(115200);
    delay(1000);

    LOG_INFO("Starting I2C Number 1");
    Wire.begin(I2C1_SDA_PIN, I2C1_SCL_PIN, 400000);
    Wire.setTimeout(100);

    LOG_INFO("Starting I2C Number 2");
    I2C2.begin(I2C2_SDA_PIN,I2C2_SCL_PIN, 100000);
    I2C2.setTimeout(100); 

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(ENABLE_12V_PIN, OUTPUT);
    pinMode(10, OUTPUT);
    digitalWrite(10, 1);
    digitalWrite(GREEN_LED_PIN, 0);
    digitalWrite(RED_LED_PIN, 0);
    digitalWrite(ENABLE_12V_PIN, 0); 

    pinMode(OLED_RESET_PIN, OUTPUT);
    digitalWrite(OLED_RESET_PIN, LOW); // Reset display
    delay(10);
    digitalWrite(OLED_RESET_PIN, HIGH);
    delay(100); // Allow display to stabilize

  
  // Initialize I2C2 after I2C1 and display reset
    // I2C2.begin(I2C2_SDA_PIN,I2C2_SCL_PIN, 100000);
    // I2C2.setTimeout(100);
    LOG_INFO("Starting setup");

    // and the magic 12V pin
    digitalWrite(ENABLE_12V_PIN, 1);

    LOG_INFO("Initialising Display");
    if (!initDisplay()) {
        LOG_ERROR("Display failed to initialise");
        for (;;); // Don't proceed, loop forever
    }


}



void loop() {
      int sensorValue = analogRead(A0);
      logMessage(LOG_LEVEL_INFO, "Sensor reading: %d", sensorValue);
      drawUI();
      delay(3000);
    
    switch (currentState) {
        case IDLE:
            if (checkVoltage()) {
                currentState = INITIALISE;
            } else {
                currentState = LOW_VOLTAGE;
            }
            break;
        case LOW_VOLTAGE:
            // Handle low voltage (e.g., display warning, retry)
            if (checkVoltage()) {
                currentState = IDLE;
            } else {
                // Timeout logic for shutdown
                currentState = SHUTDOWN;
            }
            break;
        case SHUTDOWN:
            turnOffComponents();
            enterDeepSleep(); 
            break;
        case INITIALISE:
            // Initialization logic
            displayBaseMessage("Initialising", ".");
            delay(1000);
            displayBaseMessage("Initialising", "..");
            delay(1000);
            displayBaseMessage("Initialising", "...");
            // ... 
            // Transitions to WAIT_FOR_BREATH, MOUTHPIECE_MISSING, or SHUTDOWN
            if (!digitalRead(BUTTON_PIN)) { // Check for button press
            currentState = SHUTDOWN;
            } else if (!detectMouthpiece()) { // Check for mouthpiece
                currentState = MOUTHPIECE_MISSING;
            } else {
                // Perform other initialization tasks
                // ...
                currentState = WAIT_FOR_BREATH; // Transition to waiting for breath
            }
        case WAIT_FOR_BREATH:
            if (waitForBreath()) {
                currentState = BREATH;
            }
            break;
        // ... (add cases for other states)
    }
    // I want to test the display... can you call something like this to let me test
    // the display?
}





