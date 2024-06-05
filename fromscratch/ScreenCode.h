// Declaration for an SSD1327 display connected to I2C (SDA, SCL pins)
#include <string.h>
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // I2C display address for this specific display, check datasheet
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 128 // OLED display height, in pixels


// it looks like SSD1327_SWITCHCAPVCC is no longer defined in the header file
//  assign  it here
#define  SDD1327_SWITCHCAPVCC 0x3C
#define SSD1327_WHITE 0xF
Adafruit_SSD1327 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


//****************************** Screen Bitmaps *********************************
static const unsigned char PROGMEM bluetooth_bitmap[] = {
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

void drawUI(){

  display.drawBitmap(70, 0, u_bitmap, 16, 16, 0xF);
  display.drawBitmap(0, 0, bluetooth_bitmap, 8, 16, 0x6);

  display.setTextSize(1);
  display.setTextColor(0xF);
  display.setCursor(20, 4);
  display.print("EXAMIN");

  
  
  display.drawLine(0, 18, 128, 18, 0xF);
  display.drawLine(0, 29, 128, 29, 0xF);
  display.drawLine(0, 40, 128, 40, 0xF);
}



//   void displayBaseMessage(const char* line1, const char* line2) {
//       // get the number of characters in line1 and line2
//       int len1 = strlen(line1);
//       int len2 = strlen(line2);
//       display.clearDisplay();
//       display.setTextColor(0xF); 
//       display.setTextSize(1);
//       // calculate the X and Y coordinates for the first line based on the length of the line1
//       // Place the string in the center of the display
//       display.setCursor((128 - (len1 * 16)) / 2, 20);
//       display.setCursor(5, 50);
//       display.print(line1);
//       // calculate the X and Y coordinates for the second line based on the length of the line2
//       // Place the string in the center of the display
//       // display.setCursor((128 - (len2 * 16)) / 2, 65);
//       display.setCursor(60, 65);
//       display.print(line2);
//       drawUI();
//       display.display();
// }

int max(int a, int b) {
    return (a > b) ? a : b;
}

void displayBaseMessage(const char* line1, const char* line2 = nullptr, const char* line3 = nullptr) {
    display.clearDisplay();  
    display.setTextColor(SSD1327_WHITE);
    display.setTextSize(1);

    // Display line1 (always present)
    int len1 = strlen(line1);
    int x1 = (128 - (len1 * 6)) / 2; // Adjusted character width calculation
    display.setCursor(max(0, x1), 20); // Ensure text doesn't go off-screen to the left
    display.println(line1);

    // Display line2 (optional)
    if (line2) {
        int len2 = strlen(line2);
        int x2 = (128 - (len2 * 6)) / 2;
        display.setCursor(max(0, x2), 40); // Lowered to accommodate line3
        display.println(line2);
    }

    // Display line3 (optional)
    if (line3) {
        int len3 = strlen(line3);
        int x3 = (128 - (len3 * 6)) / 2;
        display.setCursor(max(0, x3), 60); 
        display.println(line3);
    }

    drawUI();  // Assuming you have a function to draw other UI elements
    display.display();
}


// / Function to initialize the display
bool initDisplay() {
    if (!display.begin(OLED_ADDR)) {
        LOG_ERROR("SSD1327 allocation failed - startup failed");
        return false;
    }
      displayBaseMessage("Breathalyzer", "Starting...");

    return true;
}
