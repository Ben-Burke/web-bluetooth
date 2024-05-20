#include "OLED_Driver.h"
#include "SPI.h"

// Pin assignments for MKR WiFi 1010
#define RST     9    // Reset pin
#define DC      7    // Data/Command select
#define CS      10   // Chip select

void setup() {
  pinMode(RST, OUTPUT);
  pinMode(DC, OUTPUT);
  pinMode(CS, OUTPUT);

  SPI.begin(); // Initialize hardware SPI

  OLED_1in32_Init();       // Initialize the OLED display
  OLED_1in32_Clear(0x00);  // Clear the screen (black)

  // Display some text
  OLED_ShowString(0, 0, "Hello MKR1010!", FONT_1206, WHITE); // Adjust FONT size as needed

  OLED_Refresh_Gram();     // Refresh the display to show the changes
}

void loop() {
  // Add any animations or updates you want here
}

