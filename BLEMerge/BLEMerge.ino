#include <Arduino.h>
// #include <BLEDevice.h>
// #include <BLEUtils.h>
// #include <BLEServer.h>
#include <ArduinoBLE.h>
// #include <SparkFun_Bio_Sensor_Hub_Library.h>
#include <Wire.h>
#include <Adafruit_SSD1327.h>
#include <stdio.h> 
#include "driver/rtc_io.h"
#include "string.h"






//****************************** Hardware Declarations ****************************
TwoWire I2C2 = TwoWire(1);

Adafruit_SSD1327 display(128, 128, &Wire, OLED_RESET_PIN, 400000);

static unsigned long dispUpdate = 1;


//**************************** Global Variables *********************************
bool deviceConnected = false;


//********************************* Initialization *******************************
void setup() {
  Serial.begin(15200);
  delay(1000);
  Serial.println("Starting Examin Breathalyzer Merge Code"); 

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(ENABLE_12V_PIN, OUTPUT);
  pinMode(10, OUTPUT);
  digitalWrite(10, 1);
  digitalWrite(GREEN_LED_PIN, 0);
  digitalWrite(RED_LED_PIN, 0);
  digitalWrite(ENABLE_12V_PIN, 0); 

  Wire.begin(I2C1_SDA_PIN, I2C1_SCL_PIN, 400000);
  Wire.setTimeout(100);

  I2C2.begin(I2C2_SDA_PIN,I2C2_SCL_PIN, 100000);
  I2C2.setTimeout(100); 

  displayInit();

  // capInit();

  // //Initialize the pulse oximeter
  // bodyInit();

  // bleInit();


}


//******************************** Main Controll Loop ***************************
void loop() {
        digitalWrite(ENABLE_12V_PIN, 1);
        // digitalWrite(GREEN_LED_PIN, 1);

  }  






//*********************************** Display *********************************
void displayInit(){
  display.begin(OLED_ADDR);
  display.clearDisplay();
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(0xF);
  display.setCursor(16, 50);
  display.print("DogsEars");
  display.setCursor(45, 70);
  display.print("...");
  // drawUI();
  display.display();
  dispUpdate = micros();  
  
  
  //display.display();
}

