// ToDo - look at using the ArdiunoLog library to see if it is vastly better
#ifndef DEBUGLOGGER_H
#define DEBUGLOGGER_H


#define LOG_LEVEL_ERROR 0
#define LOG_LEVEL_WARN 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_DEBUG 3

// Change this to adjust the active log level
#define CURRENT_LOG_LEVEL LOG_LEVEL_DEBUG 

void logMessage(int level, const char* format, ...) {
  if (level > CURRENT_LOG_LEVEL) {
    return; // Filter out unwanted messages
  }
  
  va_list args;
  va_start(args, format);
  
  // Optional: Add level prefix (e.g., "[ERROR]")
  switch (level) {
    case LOG_LEVEL_ERROR: Serial.print("[ERROR] "); break;
    case LOG_LEVEL_WARN: Serial.print("[WARN] "); break;
    case LOG_LEVEL_INFO: Serial.print("[INFO] "); break;
    case LOG_LEVEL_DEBUG: Serial.print("[DEBUG] "); break;
  }
  
  // Print the formatted message
  // Note: Use vsnprintf_P for PROGMEM strings to save RAM
  char buffer[128]; // Adjust size as needed
  vsnprintf(buffer, sizeof(buffer), format, args); 
  Serial.println(buffer);
  
  va_end(args);
}


// Macro definitions
#define LOG_ERROR(format, ...) logMessage(LOG_LEVEL_ERROR, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) logMessage(LOG_LEVEL_WARN, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) logMessage(LOG_LEVEL_INFO, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) logMessage(LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)

#endif // DEBUGLOGGER_H
