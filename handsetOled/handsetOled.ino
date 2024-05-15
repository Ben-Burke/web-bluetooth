#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1327.h>
// #include <stdio.h> 
// #include "driver/rtc_io.h"
// #include "string.h"

//**************************** System Settings **********************************
#define DEVICE_NAME         "Examin_Breathalyzer_1"
//**************************** Hardware Parameters ******************************

#define OLED_ADDR           0x3C
#define OLED_RESET_PIN      8
#define OLED_DC_PIN         7
#define OLED_CS_PIN         10
#define OLED_CLK_PIN        13
#define OLED_MOSI_PIN       11
#define OLED_DIN_PIN        11


#define I2C1_SCL_PIN        12
#define I2C1_SDA_PIN        11

// #define I2C2_SCL_PIN        38                
// #define I2C2_SDA_PIN        21  




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

Adafruit_SSD1327 display(128, 128, &Wire, OLED_RESET_PIN, 400000);
TwoWire I2C1 = TwoWire(1);


//**************************** Global Variables *********************************
bool deviceConnected = false;

char BLEbuf[50];

//********************************* Initialization *******************************
void setup() {
  Serial.begin(15200);
  delay(5000);
  Serial.println("Starting Examin Breathalyzer Merge Code"); 

  pinMode(10, OUTPUT);
  digitalWrite(10, 1);
  
  Wire.begin(I2C1_SDA_PIN, I2C1_SCL_PIN, 400000);
  Wire.setTimeout(100);


  displayInit();

  // capInit();

  // //Initialize the pulse oximeter
  // bodyInit();

  // mlx.begin(MLX_ADDR, &I2C2);

  // bleInit();

  // BQ27425Init();

}




void DisplayTest(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(0xF);
  display.setCursor(0, 0);
  display.print("Hello World");
  display.display();
}


//******************************** Main Controll Loop ***************************
void loop() {
  
          drawUI();
          display.display();
          delay(1000);

}








//*********************************** Display *********************************
void displayInit(){
  display.begin(OLED_ADDR);
  display.clearDisplay();
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(0xF);
  display.setCursor(16, 50);
  display.print("Initilize");
  display.setCursor(45, 70);
  display.print("...");
  drawUI();
  display.display();
  
  
  //display.display();
}

void drawUI(){
    display.drawBitmap(0, 0, ble_bitmap, 8, 16, 0xF);
    display.drawBitmap(0, 0, ble_bitmap, 8, 16, 0x6);
  }
  
  display.drawBitmap(70, 0, u_bitmap, 16, 16, 0xF);

  display.setTextSize(1);
  display.setTextColor(0xF);
  display.setCursor(20, 4);
  display.print("EXAMIN");

  display.setCursor(0, 20);
  display.print("Pres:");

  display.print("Pa");

  display.print("HR:");
  display.setCursor(66, 31);
  display.print("Virus:");
  display.setTextSize(0.25);
  display.setCursor(0, 120);
  

  display.drawLine(0, 18, 128, 18, 0xF);
  display.drawLine(0, 29, 128, 29, 0xF);
  display.drawLine(0, 40, 128, 40, 0xF);
}

void updateDisplay(){
  if(micros() - dispUpdate > refreshRate){
    display.clearDisplay();
    drawUI();
    display.display();
    dispUpdate = micros();
  }
}

