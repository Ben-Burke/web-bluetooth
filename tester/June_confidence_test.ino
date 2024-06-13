#include <ArduinoBLE.h>
#define LED_BUILTIN 2

// Define the service and characteristic UUIDs
#define MY_UUID(val) ("555a0002-" val "-467a-9538-01f0652c74ef")
const char* deviceName = "SCRBLE_1";
BLEService examinService(MY_UUID("0000"));
// Notice the BLENotify flag is now included
BLEIntCharacteristic virusSensorCharacteristic(MY_UUID("0005"), BLERead | BLENotify); 
BLEIntCharacteristic virusInitialCharacteristic(MY_UUID("0006"), BLERead | BLENotify);
BLEIntCharacteristic virusMeasurement1Characteristic(MY_UUID("0007"), BLERead | BLENotify);
BLEIntCharacteristic virusMeasurement2Characteristic(MY_UUID("0008"), BLERead | BLENotify);
BLEIntCharacteristic virusMeasurement3Characteristic(MY_UUID("0009"), BLERead | BLENotify);
BLEIntCharacteristic virusFinalCharacteristic(MY_UUID("0010"), BLERead | BLENotify);

void setup() {
  // ... (same as before)
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH); 

    while (central.connected()) { 
      // Generate random values between 1000 and 2001
      int randomValues[] = {
        random(1000, 2001), random(1000, 2001), random(1000, 2001),
        random(1000, 2001), random(1000, 2001), random(1000, 2001)
      };

      // Write the random values to the characteristics
      virusSensorCharacteristic.writeValue(randomValues[0]);
      virusInitialCharacteristic.writeValue(randomValues[1]);
      virusMeasurement1Characteristic.writeValue(randomValues[2]);
      virusMeasurement2Characteristic.writeValue(randomValues[3]);
      virusMeasurement3Characteristic.writeValue(randomValues[4]);
      virusFinalCharacteristic.writeValue(randomValues[5]);

      delay(1000); // Update every second (adjust as needed)
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW); 
  }
}
