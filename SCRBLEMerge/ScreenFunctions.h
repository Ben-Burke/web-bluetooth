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