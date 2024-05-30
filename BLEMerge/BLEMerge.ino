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
const char myName[] = "looping_values";

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

//**************************** Global Variables *********************************
bool deviceConnected = false;

char BLEbuf[50];

uint32_t temp;
float handTemp;
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
  if(micros() >= lastRead + PERIOD){
    lastRead = micros();

    
    
   
    

    


    switch(state){

      case IDLE:
        if(voltage < SHUTDOWN_VOLTAGE){
          state = LOW_VOLTAGE;
          lv_time = micros();
        }


        state = INITIALISE;

        digitalWrite(ENABLE_12V_PIN, 1);
        digitalWrite(GREEN_LED_PIN, 1);



        break;


      //If the mouthpiece is disconnected wait until it is connected again
      
      
    }
 

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
  display.print("FrogsEyes");
  display.setCursor(45, 70);
  display.print("...");
  // drawUI();
  display.display();
  dispUpdate = micros();  
  
  
  //display.display();
}

