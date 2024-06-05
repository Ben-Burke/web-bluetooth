// A complete rewrite of the Examin Handset code, for board version 1.6

// Standard includes
#include "StandardIncludes.h"

// The following libraries are definitely required
#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Wire.h>
#include <Adafruit_SSD1327.h>
// ToDo - check if this is required
// #include <Adafruit_GFX.h>

//  Local Includes
#include "HardwareParameters.h"
#include "DebugLogger.h"
#include "ScreenCode.h"

TwoWire I2C2 = TwoWire(1);
#define BUTTON_PIN          6
#define BUTTON_PIN_BITMASK  0x40
// ToDo This needs fixing
// #define verbose   1

#include "Pressure.h"
#include "Capacitance.h"


#define CURRENT_LOG_LEVEL LOG_LEVEL_DEBUG 

// Constants for breath measurement
#define BREATH_LENGTH         5000000 //How long the breath timer should be
#define BREATH_SAMPLES        5  //How many samples are taken for each measurement
#define BREATH_PRESSURE       60 //Minimum pressure for the breath to be accepted
#define BREATH_SENSITIVITY    100
#define BREATH_END_THRESHOLD  4090 //What the capacitance has to drop to for the breath to be considered done (in pF)
#define BREATH_DELAY_1        5000000 //Time until first measurement after breath (in microseconds)
#define BREATH_DELAY_2        10000000    //Time after first measurement until second
#define BREATH_DELAY_3        30000000
// #define WAIT_AFTER_BREATH_1 4
// #define WAIT_AFTER_BREATH_2 6  
// #define WAIT_AFTER_BREATH_3 8

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
int debounce = 0;
double pressure;
double max_pressure;
double init_pressure = 1;
static unsigned long breathStart = 0;
int capReadings = 0;

unsigned long capInitial = 0;
static unsigned long endOfSpike = 0;
unsigned long cap;
uint16_t voltage = 9999;
#define SHUTDOWN_VOLTAGE      3000 //The battery voltage when the system will shutdown in mV





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
          LOG_INFO("Transition: IDLE -> INITIALISE");
        } else {
          currentState = LOW_VOLTAGE;
          LOG_INFO("Transition: IDLE -> LOW_VOLTAGE");
        }
        break;
      case LOW_VOLTAGE:
        // Handle low voltage (e.g., display warning, retry)
        if (checkVoltage()) {
          currentState = IDLE;
          LOG_INFO("Transition: LOW_VOLTAGE -> IDLE");
        } else {
          // Timeout logic for shutdown
          currentState = SHUTDOWN;
          LOG_INFO("Transition: LOW_VOLTAGE -> SHUTDOWN");
        }
        break;
      case SHUTDOWN:
        turnOffComponents();
        enterDeepSleep();
        LOG_INFO("Transition: SHUTDOWN");
        break;
      case INITIALISE:
        // Initialization logic
        displayBaseMessage("Initialising", ".");
        delay(1000);
        sensorsRead();
        displayBaseMessage("Initialising", "..");
        delay(1000);
        displayBaseMessage("Initialising", "...");
        // ... 
        // Transitions to WAIT_FOR_BREATH, MOUTHPIECE_MISSING, or SHUTDOWN
        if (!digitalRead(BUTTON_PIN)) { // Check for button press
          currentState = SHUTDOWN;
          LOG_INFO("Transition: INITIALISE -> SHUTDOWN");
        } else if (!detectMouthpiece()) { // Check for mouthpiece
          currentState = MOUTHPIECE_MISSING;
          LOG_INFO("Transition: INITIALISE -> MOUTHPIECE_MISSING");
          displayBaseMessage("Mouthpiece", "Missing");
          delay(2000);
        } else {
          // Perform other initialization tasks
          // ...
          currentState = WAIT_FOR_BREATH; // Transition to waiting for breath
          LOG_INFO("Transition: INITIALISE -> WAIT_FOR_BREATH");
        }
        break;
      case WAIT_FOR_BREATH:
        if (waitForBreath()) {
          currentState = BREATH;
          LOG_INFO("Transition: WAIT_FOR_BREATH -> BREATH");
        }
        break;
      // ... (add cases for other states)
    }
        // ... (add cases for other states)
    }
    // I want to test the display... can you call something like this to let me test
    // the display?



bool checkVoltage() {
    // TODO: Implement this function
    return true;
}

bool detectMouthpiece() {
    // TODO: Implement this function
   
  LOG_DEBUG("Checking for mouthpiece");
  Wire.beginTransmission(PRESS_ADDR);
  Wire.write(byte(0x0F));
  Wire.endTransmission();
  Wire.requestFrom(PRESS_ADDR,1);
  if(Wire.read() == 0xb3){
    LOG_DEBUG("Mouthpiece detected");
    return 1;
  }else{
    LOG_DEBUG("Mouthpiece not detected");
    return 0;
  }
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
  // displayBaseMessage("waiting for", "breath");
  acquireBreath();
  for (;;); // Don't proceed, loop forever
}






void sensorsRead(){
   
  cap = capRead(); 
  
  pressureInit();
  
  pressure = pressureRead();

  
}

void acquireBreath() {
    

    // Update Display with a countdown 
    if (dispUpdate + refreshRate < micros()) {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(0xF);

        if (breathStart + BREATH_LENGTH > micros()) {
            display.setCursor(25, 50);
            display.print("Breathe");
            // ..(rest of the display update code from the BREATH case)
            display.display();
        } else if(max_pressure < BREATH_PRESSURE) {
          // ... (rest of the display update code from the BREATH case)
        }
        else {
            display.setCursor(25, 50);
            display.print("Waiting");
            display.setCursor(45, 70);
            display.print("For");
            display.setCursor(15, 90);
            display.print("Breath...");
            drawUI();
            display.display();
        }
        
        dispUpdate = micros();
    }

    double capVal = (((double)(cap)) / 8388608) * 4096;
    double valInit = (((double)(capInitial)) / 8388608) * 4096;

    if (capVal >= valInit + BREATH_SENSITIVITY) {
        debounce += 1;
    }
    if (debounce >= 3) {
        currentState = BREATH;
        debounce = 0;
        breathStart = micros();
    }
}

void measure() {
    if (voltage < SHUTDOWN_VOLTAGE) {
        currentState = LOW_VOLTAGE;
        lv_time = micros();
        return; 
    }

    if (!digitalRead(BUTTON_PIN)) {
        currentState = SHUTDOWN;
        return; 
    }

    if (!detectMouthpiece()) {
        currentState = MOUTHPIECE_MISSING;
        inactive_time = micros();
        return; 
    }

    if (pressure - init_pressure > max_pressure) {
        max_pressure = pressure - init_pressure;
    }
   
    double capVal = (((double)(cap)) / 8388608) * 4096;

    if ((capVal <= BREATH_END_THRESHOLD) && (breathStart + BREATH_LENGTH < micros()) && (max_pressure >= BREATH_PRESSURE)) {
        debounce += 1;
    }
    if (debounce >= 3) {
        currentState = WAIT_AFTER_BREATH_1; 
        endOfSpike = micros();
        capReadings = 0;
    }
}




