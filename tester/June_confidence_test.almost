#include <ArduinoBLE.h>
#define LED_BUILTIN 2

// Define the service and characteristic UUIDs
#define MY_UUID(val) ("555a0002-" val "-467a-9538-01f0652c74ef")
const char* deviceName = "SCRBLE_1";
BLEService examinService(MY_UUID("0000"));
BLEIntCharacteristic virusSensorCharacteristic(MY_UUID("0005"), BLERead | BLENotify);
BLEIntCharacteristic virusInitialCharacteristic(MY_UUID("0006"), BLERead | BLENotify);
BLEIntCharacteristic virusMeasurement1Characteristic(MY_UUID("0007"), BLERead | BLENotify);
BLEIntCharacteristic virusMeasurement2Characteristic(MY_UUID("0008"), BLERead | BLENotify);
BLEIntCharacteristic virusMeasurement3Characteristic(MY_UUID("0009"), BLERead | BLENotify);
BLEIntCharacteristic virusFinalCharacteristic(MY_UUID("0010"), BLERead | BLENotify);


void setup() {
  Serial.begin(115200);
  // Initialize BLE
  if (!BLE.begin()) {
    Serial.println("Failed to initialize BLE!");
    while (1);
  }
  Serial.println("BLE Started");
  pinMode(LED_BUILTIN, OUTPUT);
  // Set the local name peripheral advertises
  BLE.setLocalName(deviceName);
  BLE.setAdvertisedService(examinService); // Advertise the service UUID

  // Add characteristics to the service
  examinService.addCharacteristic(virusSensorCharacteristic);
  examinService.addCharacteristic(virusInitialCharacteristic);
  examinService.addCharacteristic(virusMeasurement1Characteristic);
  examinService.addCharacteristic(virusMeasurement2Characteristic);
  examinService.addCharacteristic(virusMeasurement3Characteristic);
  examinService.addCharacteristic(virusFinalCharacteristic);

  // Add the service
  BLE.addService(examinService);
  
  // Start advertising
  BLE.advertise();
  Serial.println("Waiting for connections...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    digitalWrite(LED_BUILTIN, HIGH); 

    while (central.connected()) { 
      // Generate random values between 1000 and 2000
      int randomValue1 = random(1000, 2001);
      int randomValue2 = random(1000, 2001);
      int randomValue3 = random(1000, 2001);
      int randomValue4 = random(1000, 2001);
      int randomValue5 = random(1000, 2001);
      int randomValue6 = random(1000, 2001);

      // Write the random values to the characteristics (no conversion needed)
      virusSensorCharacteristic.writeValue(randomValue1);
      virusInitialCharacteristic.writeValue(randomValue2);
      virusMeasurement1Characteristic.writeValue(randomValue3);
      virusMeasurement2Characteristic.writeValue(randomValue4);
      virusMeasurement3Characteristic.writeValue(randomValue5);
      virusFinalCharacteristic.writeValue(randomValue6); // Use the new characteristic

      delay(1000); // Update every second (adjust as needed)
    }
  } else {
    digitalWrite(LED_BUILTIN, LOW); 
  }
}

