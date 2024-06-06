#define PRESS_ADDR          0x5D
//******************************** Pressure Sensor ****************************
void pressureInit(){
  LOG_DEBUG("Initializing pressure sensor...");
  Wire.beginTransmission(PRESS_ADDR);
  Wire.write(byte(0x0F));
  Wire.endTransmission();
  Wire.requestFrom(PRESS_ADDR, 1);
  Wire.read();

  LOG_DEBUG("Configuring pressure sensor...");
  Wire.beginTransmission(PRESS_ADDR);
  Wire.write(byte(0x10));
  Wire.write(byte(0b01110000));
  Wire.endTransmission();
}

double pressureRead(){
  LOG_DEBUG("Reading pressure sensor...");
  Wire.beginTransmission(PRESS_ADDR);
  Wire.write(byte(0x28));
  Wire.endTransmission();
  Wire.requestFrom(PRESS_ADDR,3);
  uint32_t data[3];
  data[0] = Wire.read();
  data[1] = Wire.read();
  data[2] = Wire.read();
  uint32_t ret = data[0];
  ret |= data[1] << 8;
  ret |= data[2] << 16;

  
  // double local_pressure = (ret * 1000) / 40960;
  // String pressureString = String(local_pressure, 2); 
  
  // LOG_DEBUG("Pressure value: " + pressureString );
  // return local_pressure;

  double local_pressure = (ret * 1000) / 40960;
  String pressureString = String(local_pressure, 2);  

  // Convert String to char array for logging
  char pressureBuffer[16]; // Adjust size as needed
  pressureString.toCharArray(pressureBuffer, sizeof(pressureBuffer));

  LOG_DEBUG("Pressure value: %s", pressureBuffer); 
  return local_pressure;

}