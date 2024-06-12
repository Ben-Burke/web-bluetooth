#include <Arduino.h>
#include <ArduinoBLE.h>
#include <Wire.h>
#include <Adafruit_SSD1327.h>
#include <stdio.h> 
#include "driver/rtc_io.h"
#include "string.h"
#include "Logger.h"

//**************************** System Settings **********************************
#define DEVICE_NAME         "Examin_Breathalyzer_1"
#define verbose             1

//****************************** Flow Settings **********************************
#define THRESHOLD             400
#define BREATH_LENGTH         5000000 //How long the breath timer should be
#define PERIOD                20000
#define BREATH_SAMPLES        5  //How many samples are taken for each measurement
#define BREATH_PRESSURE       60 //Minimum pressure for the breath to be accepted
#define BREATH_SENSITIVITY    100
#define BREATH_END_THRESHOLD  4090 //What the capacitance has to drop to for the breath to be considered done (in pF)
#define BREATH_DELAY_1        5000000 //Time until first measurement after breath (in microseconds)
#define BREATH_DELAY_2        10000000    //Time after first measurement until second
#define BREATH_DELAY_3        30000000
#define SLEEP_TIME            600000000 //Time until the device falls asleep
#define SHUTDOWN_VOLTAGE      3000 //The battery voltage when the system will shutdown in mV

//**************************** Hardware Parameters ******************************
#define BIO_RESET_PIN       17   
#define BIO_MFIO_PIN        18

#define OLED_ADDR             0x3C
#define OLED_RESET_PIN      9
#define ENABLE_12V_PIN      12 

#define CAP_ADDR              0x48


#define PRESS_ADDR          0x5D

#define BUTTON_PIN          6
#define BUTTON_PIN_BITMASK  0x40

#define I2C1_SCL_PIN        5
#define I2C1_SDA_PIN        4

#define I2C2_SCL_PIN        38                
#define I2C2_SDA_PIN        21  

#define GREEN_LED_PIN       2
#define RED_LED_PIN         1

#define EEPROM_ADDR         0x50


//****************************** System States **********************************
#define IDLE                0
#define INITIALISE           1
#define WAIT_FOR_BREATH     2
#define BREATH              3
#define WAIT_AFTER_BREATH_1 4
#define MEASURE_1           5
#define WAIT_AFTER_BREATH_2 6  
#define MEASURE_2           7
#define WAIT_AFTER_BREATH_3 8
#define MEASURE_3           9
#define COMPLETE            10
#define MOUTHPIECE_MISSING  11
#define SHUTDOWN            12
#define LOW_VOLTAGE         13


// The ArduinoBLE method begins here.
#define MY_UUID(val) ("555a0002-" val "-467a-9538-01f0652c74ef")

// fill in your name here. It will be combined with
// the Arduino's MAC address to set the peripheral BLE name:
const char myName[] = "SCRBLE_1";

// set up the service and the characteristics:
BLEService                     examin_service                 (MY_UUID("0000"));

BLEIntCharacteristic           VirusSensorCharacteristic  (MY_UUID("0005"), BLERead | BLEWrite);
BLEIntCharacteristic           VirusInitialCharacteristic  (MY_UUID("0006"), BLERead | BLEWrite);
BLEIntCharacteristic           VirusMeasurement1Characteristic  (MY_UUID("0007"), BLERead | BLEWrite);
BLEIntCharacteristic           VirusMeasurement2Characteristic  (MY_UUID("0008"), BLERead | BLEWrite);
BLEIntCharacteristic           VirusMeasurement3Characteristic  (MY_UUID("0009"), BLERead | BLEWrite);


//****************************** Screen Bitmaps *********************************
static const unsigned char PROGMEM ble_bitmap[] = {
  0b00011000,  0b00010100,
  0b00010010,  0b10010001,
  0b01010010,  0b00110100,
  0b00011000,  0b00011000,
  0b00110100,  0b01010010,
  0b10010001,  0b00010010,
  0b00010100,  0b00011000,
  0b00000000,  0b00000000
  };

static const unsigned char PROGMEM u_bitmap[] = {
  0b11111110, 0b01111111,
  0b11111110, 0b01111111,
  0b11111110, 0b01111111,
  0b11111110, 0b01111111,
  0b00111100, 0b00111100,
  0b00111100, 0b00111100,
  0b00111100, 0b00111100,
  0b00111100, 0b00111100,
  0b00111100, 0b00111100,
  0b00111100, 0b00111100,
  0b00111111, 0b11111100,
  0b00111111, 0b11111100,
  0b00011111, 0b11111000,
  0b00001111, 0b11110000,
  0b00000111, 0b11100000,
  0b00000000, 0b00000000,
  };  

//****************************** Hardware Declarations ****************************
TwoWire I2C2 = TwoWire(1);

Adafruit_SSD1327 display(128, 128, &Wire, OLED_RESET_PIN, 400000);

// snuck this in here as a shortcut
void drawUI();
#include "ScreenFunctions.h"

//**************************** Global Variables *********************************
bool deviceConnected = false;

char BLEbuf[50];

uint32_t temp;
double pressure;
double max_pressure;
double init_pressure = 1;
double pressTotal;

int power_debounce = 0;

int debounce = 0;
int capReadings = 0;
unsigned long cap;
unsigned long capTotal = 0;
unsigned long capInitial = 0;
unsigned long capFinal_1 = 0;
unsigned long capFinal_2 = 0;
unsigned long capFinal_3 = 0;
static unsigned long dispUpdate = 1;
long refreshRate = 2000;

int state = IDLE;
static unsigned long breathStart = 0;
static unsigned long lastRead = 0;
static unsigned long endOfSpike = 0;
static unsigned long inactive_time = 0;
static unsigned long lv_time = 0;
bool connection = 0;

uint8_t date [3] = {0, 0, 0}; 
char id [10];

uint16_t voltage = 9999;

uint8_t charge = 0;

//********************************* Initialization *******************************
void setup() {
  Serial.begin(15200);
  delay(5000);
  LOG_INFO("Starting Examin Breathalyzer June2024 Merge Code");
  // Serial.println("Starting Examin Breathalyzer Merge Code"); 

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(ENABLE_12V_PIN, OUTPUT);
  //ToDo - what the hell is pin 10 and where is OUTPUT and INPUT_PULLUP apparently NOT defined?
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

  capInit();

  //Initialize the pulse oximeter

  bleInit();


}

void InitialStatesCheck(int &retFlag);


void prepareScreen(){
          display.clearDisplay();
          display.setTextSize(2);
          display.setTextColor(0xF);
}


void drawAndDisplayUI(){
      drawUI();
      display.display();
      dispUpdate = micros();
      
}


void castVirusVariables(){
  double capVal = (((double)(cap))/8388608)*4096;
  double valInit = (((double)(capInitial))/8388608)*4096;
  double valFinal_1 = (((double)(capFinal_1))/8388608)*4096;
  double valFinal_2 = (((double)(capFinal_2))/8388608)*4096;
  double valFinal_3 = (((double)(capFinal_3))/8388608)*4096;
}

void setVirusCharacteristics(){
  // This was blowing up variable definitions, so I commented it out
  // this was randomly generated by copilot, but, it's a good idea to have a function that sets the characteristics
  // VirusSensorCharacteristic.writeValue(capVal);
  // VirusInitialCharacteristic.writeValue(valInit);
  // VirusMeasurement1Characteristic.writeValue(valFinal_1);
  // VirusMeasurement2Characteristic.writeValue(valFinal_2);
  // VirusMeasurement3Characteristic.writeValue(valFinal_3);

}
//******************************** Main Controll Loop ***************************
void loop() {
  //ToDo remove this is isn't used 
  if(micros() >= lastRead + PERIOD){
    lastRead = micros();

    int retFlag; // for the InitialStatesCheck function
    
    if((state != IDLE)|(state != MOUTHPIECE_MISSING)|(state != SHUTDOWN)){
      pressureInit();
      sensorsRead();
      dataLog();
      bleUpdate();
    }else{
      // who needs temp when you have beurer and a pulse oximeter?
      temp = 0;
      cap = 0; 
      pressure = 0;
    }

    
    // castVirusVariables();
    double capVal = (((double)(cap))/8388608)*4096;
    double valInit = (((double)(capInitial))/8388608)*4096;
    double valFinal_1 = (((double)(capFinal_1))/8388608)*4096;
    double valFinal_2 = (((double)(capFinal_2))/8388608)*4096;
    double valFinal_3 = (((double)(capFinal_3))/8388608)*4096;


    switch(state){
      case SHUTDOWN:
        digitalWrite(ENABLE_12V_PIN, 0);
        digitalWrite(GREEN_LED_PIN, 0);
        digitalWrite(RED_LED_PIN, 0);

        delay(500);
        //why pin 6 Owen?
        rtc_gpio_pullup_en(GPIO_NUM_6);
        rtc_gpio_pulldown_dis(GPIO_NUM_6);
        esp_sleep_enable_ext0_wakeup((gpio_num_t)GPIO_NUM_6, 0);
        esp_deep_sleep_start();
        break;

      case IDLE:
        //Not required for June24 Release
        if(voltage < SHUTDOWN_VOLTAGE){
          state = LOW_VOLTAGE;
          lv_time = micros();
        }


        state = INITIALISE;

        // Turn on the screen
        digitalWrite(ENABLE_12V_PIN, 1);
        digitalWrite(GREEN_LED_PIN, 1);



        break;

      //Initialize the device, take the initial capacitance measurements before the breath 
      case INITIALISE:
        if(voltage < SHUTDOWN_VOLTAGE){
          state = LOW_VOLTAGE;
          lv_time = micros();
        }
        //Check if the button has been pressed
        if(!digitalRead(BUTTON_PIN)){
          state = SHUTDOWN;
        }
        //Check if the mouthpiece is connected
        if(!detectMouthpiece()){
          state = MOUTHPIECE_MISSING;
          inactive_time = micros();
          break;
        }

        if(date[0] == 0){
          date[0] = readEEPROM(1);
          date[1] = readEEPROM(2);
          date[2] = readEEPROM(3);

          for(int i = 0; i < 9; i++){
            id[i] = readEEPROM(i + 4);
          }

        }


        //Update the display
        if(dispUpdate + refreshRate < micros() ){
          display.clearDisplay();
          display.setTextSize(2);
          display.setTextColor(0xF);
          display.setCursor(16, 50);
          display.print("INITIALISE");
          display.setCursor(45, 70);
          display.print("...");
          // drawUI();
          // display.display();
          // ToDo - if dispUpdate = micros() then this can be a call to drawAndDisplayUI
          drawAndDisplayUI();
          // Having trouble getting the appearance of the display to work here... stick with old code for now
          // displayBaseMessage("INITIALISE", "Please Wait");
          // dispUpdate = micros();
        }
        //Set the LEDs
        digitalWrite(GREEN_LED_PIN, 1);
        digitalWrite(RED_LED_PIN, 0);

        //Update initial capacitance measurements
        capTotal += cap;
        capReadings += 1;

        pressTotal += pressure;

        //Once enough capacitance samples have been taken move onto wait for breath
        if(capReadings >= BREATH_SAMPLES){
          capInitial = capTotal/capReadings;
          init_pressure = pressTotal/capReadings;
          state = WAIT_FOR_BREATH;
          capTotal = 0;
          capReadings = 0;
          inactive_time = micros();
        }
        break;

      //Wait until the mouthpiece is breathed into
      case WAIT_FOR_BREATH:
        InitialStatesCheck( retFlag );
        // if(voltage < SHUTDOWN_VOLTAGE){
        //   state = LOW_VOLTAGE;
        //   lv_time = micros();
        // }
        // //Check if the button has been pressed
        // if((!digitalRead(BUTTON_PIN))|(inactive_time + SLEEP_TIME < micros())){
        //   state = SHUTDOWN;
        // }
        // //Check if the mouthpiece is connected
        // if(!detectMouthpiece()){
        //   state = MOUTHPIECE_MISSING;
        //   inactive_time = micros();
        //   break;
        // }




        //Update the display
        if(dispUpdate + refreshRate < micros() ){
          prepareScreen();
          display.setCursor(25, 50);
          display.print("Waiting");
          display.setCursor(45, 70);
          display.print("For");
          display.setCursor(15, 90);
          display.print("Breath...");
          drawAndDisplayUI();
        }
        
        //If the capacitance of the sensor increases move onto next state (after debouncing)
        if(capVal >= valInit + BREATH_SENSITIVITY){
          debounce += 1;
        }
        if(debounce >= 3){
          state = BREATH;
          debounce = 0;
          breathStart = micros();
        }         
        break;

      //Waits until the breath has been finished
      case BREATH:
        InitialStatesCheck(retFlag);
        if (retFlag == 2)
          break;

        if(pressure - init_pressure > max_pressure){
          max_pressure = pressure - init_pressure;
        }

        //Updates display
        if(dispUpdate + refreshRate < micros() ){
          prepareScreen();

          if(breathStart + BREATH_LENGTH > micros()){
            display.setCursor(25, 50);
            display.print("Breathe");
            display.setCursor(40, 70);
            display.setTextSize(3);
            itoa((breathStart + BREATH_LENGTH - micros())/1000000, BLEbuf, 10);
            display.print(BLEbuf);
            display.print(".");
            itoa(((breathStart + BREATH_LENGTH - micros())/100000)%10, BLEbuf, 10);
            display.print(BLEbuf);  
          }else if(max_pressure < BREATH_PRESSURE){
            display.setTextSize(2);
            display.setCursor(10, 50);
            display.print("Breath");
            display.setCursor(10, 70);
            display.print("Again");
          } else {
            display.setTextSize(2);
            display.setCursor(10, 50);
            display.print("Waiting");
            display.setCursor(45, 70);
            display.print("...");
          }
  
          drawAndDisplayUI();
        }

        //Checks if the breath has ended
        if((capVal <= BREATH_END_THRESHOLD)&(breathStart + BREATH_LENGTH < micros())&(max_pressure >= BREATH_PRESSURE)){
          debounce += 1;
        }

        //Debounce for detecting when the breath has ended
        if(debounce >= 3){
          state = WAIT_AFTER_BREATH_1;
          endOfSpike = micros();
          capReadings = 0;
        }
        break;

      //Waiting for the moisture to evaporate after the breath
      case WAIT_AFTER_BREATH_1:
        // ToDo Duplicate? int retFlag;
        InitialStatesCheck(retFlag);
        if (retFlag == 2)
          break;

        //Update the diplay
        if(dispUpdate + refreshRate < micros() ){
          prepareScreen();
          display.setCursor(0, 50);
          display.print("Processing");
          display.setCursor(45, 70);
          display.print("...");
          drawAndDisplayUI();
        }

        //Check if enough time has passed
        if(micros() >= endOfSpike + BREATH_DELAY_1){
          capTotal = 0;
          capReadings = 0;
          state = MEASURE_1;
        }
        break;

      //Meaure the final capacitance value of the sensor
      case MEASURE_1:
        InitialStatesCheck(retFlag);
        if (retFlag == 2)
          break;

        //Update the display
        if(dispUpdate + refreshRate < micros() ){
          prepareScreen();
          display.setCursor(0, 50);
          display.print("Processing");
          display.setCursor(45, 70);
          display.print("...");
          drawAndDisplayUI();
        }

        //Update the capacitance readings
        capTotal += cap;
        capReadings += 1;
        if(capReadings >= BREATH_SAMPLES){
          //Find the average of the readings
          capFinal_1 = capTotal/capReadings;
          state = WAIT_AFTER_BREATH_2;
        }
        break;

      case WAIT_AFTER_BREATH_2:
        if(voltage < SHUTDOWN_VOLTAGE){
          state = LOW_VOLTAGE;
          lv_time = micros();
        }
        //Check if the button has been pressed
        if(!digitalRead(BUTTON_PIN)){
          state = SHUTDOWN;
        }

        //Check if the mouthpiece is there
        if(!detectMouthpiece()){
          state = MOUTHPIECE_MISSING;
          inactive_time = micros();
          break;
        }

        //Update the diplay
        if(dispUpdate + refreshRate < micros() ){
          prepareScreen();
          display.setCursor(0, 50);
          display.print("Processing");
          display.setCursor(45, 70);
          display.print("...");
          drawAndDisplayUI();
        }

        //Check if enough time has passed
        if(micros() >= endOfSpike + BREATH_DELAY_2){
          capTotal = 0;
          capReadings = 0;
          state = MEASURE_2;
        }
        break;

      //Meaure the final capacitance value of the sensor
      case MEASURE_2:
        InitialStatesCheck(retFlag);
        if (retFlag == 2)
          break;

        //Update the display
        if(dispUpdate + refreshRate < micros() ){
          prepareScreen();
          display.setCursor(0, 50);
          display.print("Processing");
          display.setCursor(45, 70);
          display.print("...");
          drawAndDisplayUI();
        }

        //Update the capacitance readings
        capTotal += cap;
        capReadings += 1;
        if(capReadings >= BREATH_SAMPLES){
          //Find the average of the readings
          capFinal_2 = capTotal/capReadings;
          state = WAIT_AFTER_BREATH_3;

        }
        break;

      case WAIT_AFTER_BREATH_3:
        InitialStatesCheck(retFlag);
        if (retFlag == 2)
          break;

        //Update the diplay
        if(dispUpdate + refreshRate < micros() ){
          prepareScreen();
          display.setCursor(0, 50);
          display.print("Processing");
          display.setCursor(45, 70);
          display.print("...");
          drawAndDisplayUI();
        }

        //Check if enough time has passed
        if(micros() >= endOfSpike + BREATH_DELAY_3){
          capTotal = 0;
          capReadings = 0;
          state = MEASURE_3;
        }
        break;

      //Meaure the final capacitance value of the sensor
      case MEASURE_3:
        InitialStatesCheck(retFlag);
        if (retFlag == 2)
          break;

        //Update the display
        if(dispUpdate + refreshRate < micros() ){
          prepareScreen();
          display.setCursor(0, 50);
          display.print("Processing");
          display.setCursor(45, 70);
          display.print("...");
          drawAndDisplayUI();
        }

        //Update the capacitance readings
        capTotal += cap;
        capReadings += 1;
        if(capReadings >= BREATH_SAMPLES){
          //Find the average of the readings
          capFinal_3 = capTotal/capReadings;
          state = COMPLETE;
          inactive_time = micros();
        }
        break;



      //Display the final results
      case COMPLETE:
        // ToDo - this could be a call to InitialStatesCheck 
        if(voltage < SHUTDOWN_VOLTAGE){
          state = LOW_VOLTAGE;
          lv_time = micros();
        }
        //If the virus is detected display that, else display that the result is negative
        if(dispUpdate + refreshRate < micros() ){
          prepareScreen();
//          display.setCursor(0, 50);
//          display.print("Measurements");
          display.setCursor(20, 70);
          display.print("Complete");
          drawAndDisplayUI();
        }

        if(!detectMouthpiece()){
          state = MOUTHPIECE_MISSING;
          inactive_time = micros();
          break;
        }

        if(inactive_time + SLEEP_TIME < micros()){
          state = SHUTDOWN;
        }

        //Check if the button has been pressed
        if(!digitalRead(BUTTON_PIN)){
          state = INITIALISE;
        }

        break;

      //If the mouthpiece is disconnected wait until it is connected again
      case MOUTHPIECE_MISSING:
        if(voltage < SHUTDOWN_VOLTAGE){
          state = LOW_VOLTAGE;
          lv_time = micros();
        }

          pressTotal = 0;
          capTotal = 0;
          capInitial = 0;
          capFinal_1 = 0;
          capFinal_2 = 0;
          capFinal_3 = 0;
          debounce = 0;
          capReadings = 0;
          max_pressure = 0;
          date[0] = 0;
          date[1] = 0;
          date[2] = 0;

          if(dispUpdate + refreshRate < micros() ){
              prepareScreen();
              display.setCursor(20, 50);
              display.print("Connect");
              display.setCursor(0,65);
              display.print("Mouthpiece");
              drawAndDisplayUI();
          }

          delay(1);
          //Check if the mouthpiece has been disconnected
          if(detectMouthpiece()){
            state = INITIALISE;
          }

          //Check if the button has been pressed
          if((!digitalRead(BUTTON_PIN))|(inactive_time + SLEEP_TIME < micros())){
            state = SHUTDOWN;
          }
        break;
      
      case LOW_VOLTAGE:
        if(lv_time + 10000000 < micros()){
          state = SHUTDOWN;
        }

        if(voltage > SHUTDOWN_VOLTAGE + 200){
          state = INITIALISE;
          lv_time = 0;
        }
        
        if(dispUpdate + refreshRate < micros() ){
              prepareScreen();            
              display.setCursor(20, 50);
              display.print("BATTERY");
              display.setCursor(45,65);
              display.print("LOW");
              display.display();
              dispUpdate = micros();
          }
        
        
        break;
      
    }
 

  }
}

void InitialStatesCheck(int &retFlag)
{
  retFlag = 1;
  if (voltage < SHUTDOWN_VOLTAGE)
  {
    state = LOW_VOLTAGE;
    lv_time = micros();
  }
  // Checks if the button has been pressed
  if((!digitalRead(BUTTON_PIN))|(inactive_time + SLEEP_TIME < micros()))
  {
    state = SHUTDOWN;
  }
  // Checks if the mouthpiece is connected
  if (!detectMouthpiece())
  {
    state = MOUTHPIECE_MISSING;
    inactive_time = micros();
    {
      retFlag = 2;
      return;
    };
  }
}

void bleInit(){
// start the BLE radio:
  if (!BLE.begin()) {
    Serial.println("Failed to start BLE");
  }
  // set the peripheral's local name:
  BLE.setLocalName(myName);

  // set the advertised service:
  BLE.setAdvertisedService(examin_service);
  // add the characteristics:
  
  // This creates characteristics for all those defined above
  examin_service.addCharacteristic(VirusSensorCharacteristic);
  examin_service.addCharacteristic(VirusInitialCharacteristic);
  examin_service.addCharacteristic(VirusMeasurement1Characteristic);
  examin_service.addCharacteristic(VirusMeasurement2Characteristic);
  examin_service.addCharacteristic(VirusMeasurement3Characteristic);
  

  //This initializes the characteristics defined above
  VirusSensorCharacteristic.writeValue(0);
  VirusInitialCharacteristic.writeValue(0);
  VirusMeasurement1Characteristic.writeValue(0);
  VirusMeasurement2Characteristic.writeValue(0);
  VirusMeasurement3Characteristic.writeValue(0);
  

  // add the service to the peripheral and advertise it:
  BLE.addService(examin_service);
  BLE.advertise();
}


void bleUpdate(){

  // if (deviceConnected) {
  //   // ToDo - this could be a call to setVirusCharacteristics
  //
// VirusSensorCharacteristic.writeValue(0);
  // VirusInitialCharacteristic.writeValue(0);
  // VirusMeasurement1Characteristic.writeValue(0);
  // VirusMeasurement2Characteristic.writeValue(0);
  // VirusMeasurement3Characteristic.writeValue(0);

//   double valInit = (((double)(capInitial))/8388608)*4096;
//   double valFinal_1 = (((double)(capFinal_1))/8388608)*4096;
//   double valFinal_2 = (((double)(capFinal_2))/8388608)*4096;
//   double valFinal_3 = (((double)(capFinal_3))/8388608)*4096;

  castVirusVariables();

//   String init = String(valInit);
//   String fin1 = String(valFinal_1);
//   String fin2 = String(valFinal_2);
//   String fin3 = String(valFinal_3);

//   transmit = String(init + "," + fin1 + "," + fin2 + "," + fin3);
//   transmit.toCharArray(BLEbuf, 50);
//   pCharacteristicMEAS->setValue(BLEbuf);


//   if(init_pressure != 1){
//     itoa(pressure - init_pressure, BLEbuf, 10);
//     pCharacteristicPRES->setValue(BLEbuf);
//   }else{
//     itoa(0, BLEbuf, 10);
//     pCharacteristicPRES->setValue(BLEbuf);    
//   }

//   double capVal = (((double)(cap))/8388608)*4096;
//   itoa(capVal, BLEbuf, 10);
//   pCharacteristicVS->setValue(BLEbuf);


//   transmit = String(state);
//   transmit.toCharArray(BLEbuf, 50);
//   pCharacteristicSTAT->setValue(BLEbuf);


//   String date_1 = String(date[0]);
//   String date_2 = String(date[1]);
//   String date_3 = String(date[2]);
//   String id_s = String(id);
//   transmit = String(date_1 + "/" + date_2 + "/" +date_3 + "," + id_s);
//   transmit.toCharArray(BLEbuf, 50);
//   pCharacteristicID->setValue(BLEbuf);
// }





  }

  

//************************** AD7745 Capacitance to Digital  *********************
void capInit(){
  Wire.beginTransmission(CAP_ADDR);
  Wire.write(byte(0x07));
  Wire.write(byte(0x80));
  Wire.endTransmission();

  Wire.beginTransmission(CAP_ADDR);
  Wire.write(byte(0x09));
  Wire.write(byte(0x23));
  Wire.endTransmission();

  Wire.beginTransmission(CAP_ADDR);
  Wire.write(byte(0x0A));
  Wire.write(byte(0x21));
  Wire.endTransmission();
}

unsigned long capRead(){
  int capStatus = 0;
  unsigned long capData = 0;

  Wire.requestFrom(CAP_ADDR, 4);

  if(4 <= Wire.available()){
    capStatus = Wire.read();
    byte capDataH = Wire.read();
    byte capDataM = Wire.read();
    byte capDataL = Wire.read();
    capData = (capDataH << 16)|(capDataM << 8)|capDataL;
    
    if((capData <= 8388608)&(verbose == 2)){
      Serial.println("[ERROR]Negative capacitance!"); 
      return 0;
    }
    
    capData = capData - 8388608;
    
    return capData;

  }else{
    if(verbose == 2){
      Serial.println("I2C Failure!");
    }
    return 0;
  }

}

//*********************************** Display *********************************
void displayInit(){
  display.begin(OLED_ADDR);
  display.clearDisplay();
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(0xF);
  display.setCursor(16, 50);
  display.print("INITIALISE");
  display.setCursor(45, 70);
  display.print("...");
  drawUI();
  display.display();
  dispUpdate = micros();  
  
  
  //display.display();
}

void drawUI(){
  if(connection){
    display.drawBitmap(0, 0, ble_bitmap, 8, 16, 0xF);
  } else {
    display.drawBitmap(0, 0, ble_bitmap, 8, 16, 0x6);
  }
  
  display.drawBitmap(70, 0, u_bitmap, 16, 16, 0xF);

  display.setTextSize(1);
  display.setTextColor(0xF);
  display.setCursor(20, 4);
  display.print("EXAMIN");

  itoa(charge, BLEbuf, 10);
  
  uint8_t charge_voltage = (voltage - SHUTDOWN_VOLTAGE)/11;

  if(charge_voltage > 100){
    charge_voltage = 100;
  }

  

  if(charge_voltage == 100){
    display.setCursor(100, 4);  
  }else if(charge_voltage >= 10){
    display.setCursor(105, 4);
  }else{
    display.setCursor(112, 4);
  }
  ////display.setCursor(90, 4); //////////
  itoa(charge_voltage, BLEbuf, 10);
  display.print(BLEbuf);
  display.setCursor(120, 4);
  display.print("%");

  display.setCursor(0, 20);
  display.print("Pres:");
  if(pressure - init_pressure < 9999){
    
    itoa(pressure - init_pressure, BLEbuf, 10);
  } else {
    itoa(0, BLEbuf, 10);
  }
  display.print(BLEbuf);  
  display.print("Pa");

  display.setCursor(66, 31);
  display.print("Virus:");
  double capVal1 = (((double)(cap))/8388608)*4.096;
  itoa(capVal1, BLEbuf, 10);
  display.print(BLEbuf);
  display.print(".");  
  double capVal2 = (int)(capVal1*100) - (int)(capVal1)*100;
  itoa(capVal2, BLEbuf, 10);
  display.print(BLEbuf);

  display.setTextSize(0.25);
  display.setCursor(0, 120);
  
  double valFinal_1 = (((double)(capFinal_1))/8388608)*4096;
  double valFinal_2 = (((double)(capFinal_2))/8388608)*4096;
  double valFinal_3 = (((double)(capFinal_3))/8388608)*4096;

  display.setTextColor(0x6);
  itoa(valFinal_1, BLEbuf, 10);
  display.print("1:");
  display.print(BLEbuf);

  itoa(valFinal_2, BLEbuf, 10);
  display.print(" 2:");
  display.print(BLEbuf);

  itoa(valFinal_3, BLEbuf, 10);
  display.print(" 3:");
  display.print(BLEbuf);

  if(date[0] != 0){
    display.setCursor(0, 110);
    itoa(date[2], BLEbuf, 10);
    display.print(BLEbuf);
    display.print("/");
    itoa(date[0], BLEbuf, 10);
    display.print(BLEbuf);
    display.print("/");
    itoa(date[1], BLEbuf, 10);
    display.print(BLEbuf);
    display.print(" ID:");
    display.print(id);
  }

  display.drawLine(0, 18, 128, 18, 0xF);
  display.drawLine(0, 29, 128, 29, 0xF);
  display.drawLine(0, 40, 128, 40, 0xF);
}

//******************************** Pressure Sensor ****************************
void pressureInit(){
  Wire.beginTransmission(PRESS_ADDR);
  Wire.write(byte(0x0F));
  Wire.endTransmission();
  Wire.requestFrom(PRESS_ADDR, 1);
  Wire.read();

  Wire.beginTransmission(PRESS_ADDR);
  Wire.write(byte(0x10));
  Wire.write(byte(0b01110000));
  Wire.endTransmission();
}

double pressureRead(){
  Wire.beginTransmission(PRESS_ADDR);
  Wire.write(byte(0x28));
  Wire.endTransmission();
  Wire.requestFrom(PRESS_ADDR,3);
  uint32_t data[3];
  data[0] = Wire.read();
  data[1] = Wire.read();
  data[2] = Wire.read();
  uint32_t ret = data[0];
  ret |= data[1] << 8;
  ret |= data[2] << 16;

  return (ret * 1000)/40960;
}



  

//****************************** System Functions ******************************
bool detectMouthpiece(){
  Wire.beginTransmission(PRESS_ADDR);
  Wire.write(byte(0x0F));
  Wire.endTransmission();
  Wire.requestFrom(PRESS_ADDR,1);
  if(Wire.read() == 0xb3){
    return 1;
  }else{
    return 0;
  }

  


}

void sensorsRead(){
  cap = capRead(); 
  pressure = pressureRead();
  // ToDo - I still don't know why pressureInit is called here - you would think it would happen before pressureRead?
  pressureInit();
}

void dataLog(){
  if(verbose == 1){
    double capVal = (((double)(cap))/8388608)*4096;
    double valInit = (((double)(capInitial))/8388608)*4096;
    double valFinal = (((double)(capFinal_1))/8388608)*4096;
    double valFinal_1 = (((double)(capFinal_1))/8388608)*4096;
    double valFinal_2 = (((double)(capFinal_2))/8388608)*4096;
    double valFinal_3 = (((double)(capFinal_3))/8388608)*4096;

    String csvString = "";  // Initialize an empty String

// Add each variable to the string with commas
    csvString += String(capVal, 4);  // Convert double to String with 4 decimal places
    csvString += ",";
    csvString += String(valInit, 4);
    csvString += ",";
    csvString += String(valFinal, 4);
    csvString += ",";
    csvString += String(valFinal_1, 4);
    csvString += ",";
    csvString += String(valFinal_2, 4);
    csvString += ",";
    csvString += String(valFinal_3, 4);
    csvString += ",";
    csvString += String(id);
    csvString += ",";
    csvString += String(date[0]) + "/" + String(date[1]) + "/" + String(date[2]);
    csvString += ",";
    csvString += String(state);


// Now csvString contains your comma-separated values


    static bool headingPrinted = false;

    if (!headingPrinted) {
      Serial.println("startCap,Pressure,ValInit,ValFinal_1,ValFinal_2,ValFinal_3,ID,Date,State");
      headingPrinted = true;
    }


// + "," + String(id) + "," + String(date[0]) + "/" + String(date[1]) + "/" + String(date[2]) + "," + String(state)
     Serial.println(csvString);

    // LOG_INFO();
  }

  if(verbose == 2) {
    double capVal = (((double)(cap))/8388608)*4096;
    Serial.print("Virus: ");
    Serial.println(capVal);
    Serial.print("Pressure: ");
    Serial.println(pressure);
    Serial.print("State: ");
    Serial.println(state);

    //Serial.print("PULSE STATUS-----: ");
    //Serial.println(body.extStatus);

    //Serial.print("FINGER STATUS-----: ");
    //Serial.println(body.extStatus);

    Serial.print("Battery Voltage: ");
    Serial.println(voltage);

  }
}


 

 //****************************** EEPROM ****************************************
uint8_t readEEPROM(uint8_t addr){
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write(byte(addr));
  Wire.endTransmission();
  Wire.requestFrom(EEPROM_ADDR, 1);
  return Wire.read();
}

void writeEEPROM(uint8_t addr, uint8_t data){
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write(byte(addr));
  Wire.write(byte(data));
  Wire.endTransmission();
}


