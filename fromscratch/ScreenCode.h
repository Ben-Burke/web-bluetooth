// Declaration for an SSD1327 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // I2C display address for this specific display, check datasheet
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 128 // OLED display height, in pixels


// it looks like SSD1327_SWITCHCAPVCC is no longer defined in the header file
//  assign  it here
#define  SDD1327_SWITCHCAPVCC 0x3C
Adafruit_SSD1327 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


//****************************** Screen Bitmaps *********************************
// static const unsigned char PROGMEM bluetooth_bitmap[] = {
//   0b00011000,  0b00010100,
//   0b00010010,  0b10010001,
//   0b01010010,  0b00110100,
//   0b00011000,  0b00011000,
//   0b00110100,  0b01010010,
//   0b10010001,  0b00010010,
//   0b00010100,  0b00011000,
//   0b00000000,  0b00000000
//   };

// static const unsigned char PROGMEM u_bitmap[] = {
//   0b11111110, 0b01111111,
//   0b11111110, 0b01111111,
//   0b11111110, 0b01111111,
//   0b11111110, 0b01111111,
//   0b00111100, 0b00111100,
//   0b00111100, 0b00111100,
//   0b00111100, 0b00111100,
//   0b00111100, 0b00111100,
//   0b00111100, 0b00111100,
//   0b00111100, 0b00111100,
//   0b00111111, 0b11111100,
//   0b00111111, 0b11111100,
//   0b00011111, 0b11111000,
//   0b00001111, 0b11110000,
//   0b00000111, 0b11100000,
//   0b00000000, 0b00000000,
//   };  
