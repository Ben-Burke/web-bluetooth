#include <string.h>
#define SSD1327_WHITE 0xF

int max(int a, int b) {
    return (a > b) ? a : b;
}

void displayBaseMessage(const char* line1, const char* line2 = nullptr, const char* line3 = nullptr) {
    display.clearDisplay();  
    display.setTextColor(SSD1327_WHITE);
    display.setTextSize(2);  // Set text size to 2

    // Calculate line heights and spacing based on text size 2
    int lineHeight = 20; // Approximate height of text with size 2
    int ySpacing = 10; // Vertical spacing between lines

    // Display line1 (always present)
    int len1 = strlen(line1);
    int x1 = (128 - (len1 * 12)) / 2; // Double the character width for size 2
    display.setCursor(max(0, x1), lineHeight);
    display.println(line1);

    // Display line2 (optional)
    if (line2) {
        int len2 = strlen(line2);
        int x2 = (128 - (len2 * 12)) / 2;
        display.setCursor(max(0, x2), 2 * lineHeight + ySpacing); // Adjust for size 2 and spacing
        display.println(line2);
    }

    // Display line3 (optional)
    if (line3) {
        int len3 = strlen(line3);
        int x3 = (128 - (len3 * 12)) / 2;
        display.setCursor(max(0, x3), 3 * lineHeight + 2 * ySpacing); // Further adjust for size 2 and spacing
        display.println(line3);
    }

    drawUI();  
    display.display();
}
