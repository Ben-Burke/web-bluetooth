//************************** AD7745 Capacitance to Digital  *********************
#define CAP_ADDR            0x48
void capInit(){
  LOG_DEBUG("Initializing Capacitance Sensor - AD7745 - transmission 1");
  Wire.beginTransmission(CAP_ADDR);
  Wire.write(byte(0x07));
  Wire.write(byte(0x80));
  Wire.endTransmission();

  LOG_DEBUG("Initializing Capacitance Sensor - AD7745 - transmission 2");
  Wire.beginTransmission(CAP_ADDR);
  Wire.write(byte(0x09));
  Wire.write(byte(0x23));
  Wire.endTransmission();

  LOG_DEBUG("Initializing Capacitance Sensor - AD7745 - transmission 3");
  Wire.beginTransmission(CAP_ADDR);
  Wire.write(byte(0x0A));
  Wire.write(byte(0x21));
  Wire.endTransmission();
}


unsigned long capRead(){
  int capStatus = 0;
  unsigned long capData = 0;
  LOG_DEBUG("Reading Capacitance Sensor - AD7745 - requestFrom");
  Wire.requestFrom(CAP_ADDR, 4);

  if(4 <= Wire.available()){
    LOG_DEBUG("Reading Capacitance Sensor - AD7745 - Wire.read");
    capStatus = Wire.read();
    LOG_DEBUG("capStatus: %d", capStatus);
    byte capDataH = Wire.read();
    LOG_DEBUG("capDataH: %d", capDataH);
    byte capDataM = Wire.read();
    LOG_DEBUG("capDataM: %d", capDataM);
    byte capDataL = Wire.read();
    LOG_DEBUG("capDataL: %d", capDataL);
    capData = (capDataH << 16)|(capDataM << 8)|capDataL;
    
    if((capData <= 8388608)&(verbose == 2)){
      LOG_ERROR("Capacitance Sensor - AD7745 - Data is less than 8388608");
      return 0;
    }
    
    capData = capData - 8388608;
    LOG_DEBUG("Capacitance Sensor - AD7745 - Data: %d", capData);
    return capData;

  }else{
    LOG_ERROR("I2C Failure in capREAD");
    return 0;
  }

}