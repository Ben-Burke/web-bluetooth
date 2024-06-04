//************************** AD7745 Capacitance to Digital  *********************
void capInit(){
  Wire.beginTransmission(CAP_ADDR);
  Wire.write(byte(0x07));
  Wire.write(byte(0x80));
  Wire.endTransmission();

  Wire.beginTransmission(CAP_ADDR);
  Wire.write(byte(0x09));
  Wire.write(byte(0x23));
  Wire.endTransmission();

  Wire.beginTransmission(CAP_ADDR);
  Wire.write(byte(0x0A));
  Wire.write(byte(0x21));
  Wire.endTransmission();
}


unsigned long capRead(){
  int capStatus = 0;
  unsigned long capData = 0;

  Wire.requestFrom(CAP_ADDR, 4);

  if(4 <= Wire.available()){
    capStatus = Wire.read();
    byte capDataH = Wire.read();
    byte capDataM = Wire.read();
    byte capDataL = Wire.read();
    capData = (capDataH << 16)|(capDataM << 8)|capDataL;
    
    if((capData <= 8388608)&(verbose == 2)){
      Serial.println("[ERROR]Negative capacitance!"); 
      return 0;
    }
    
    capData = capData - 8388608;
    
    return capData;

  }else{
    if(verbose == 2){
      Serial.println("I2C Failure!");
    }
    return 0;
  }

}