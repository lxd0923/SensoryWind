#include "LIS2DH12.h"


LIS2DH12::LIS2DH12(TwoWire *pWire,uint8_t addr)
{
  _deviceAddr = addr;
  _pWire = pWire;
  
  mode = LOW_POWER_MODE;  // Set default mode to Low-Power Mode

}

bool LIS2DH12::begin(void)
{
  _pWire->begin();
  uint8_t identifier = 0;
  bool ret = false;

  _reset = 1;
  readReg(REG_CARD_ID,&identifier,1);
  DBG(identifier);
  if(identifier == 0x33){
    ret = true;
  }else if(identifier == 0 || identifier == 0xff){
    DBG("Communication failure");
    ret = false;
  }else{
    DBG("the ic is not LIS2DH12");
    ret = false;
  }

  return ret;
}

uint8_t LIS2DH12::getAcquireRate(void)
{
    uint8_t reg1;
    readReg(REG_CTRL_REG1, &reg1, 1);
    
    // Extract the ODR (Output Data Rate) bits (bits 4 to 7)
    uint8_t rate = (reg1 & 0xF0) >> 4;

    // Convert the ODR value to a meaningful rate (e.g., Hz)
    switch (rate) {
        case 0x0: return 1;   // 1 Hz
        case 0x1: return 10;  // 10 Hz
        case 0x2: return 25;  // 25 Hz
        case 0x3: return 50;  // 50 Hz
        case 0x4: return 100; // 100 Hz
        case 0x5: return 200; // 200 Hz
        case 0x6: return 400; // 400 Hz
        case 0x7: return 800; // 800 Hz
        default:  return 0;   // Invalid or unsupported rate
    }
}

float LIS2DH12::getTimeInterval(void)
{
    uint8_t rate = getAcquireRate();
    if (rate == 0) {
        // Return a very large value or indicate an error if rate is not supported
        return -1.0f; // Error indicator
    }
    // Calculate the time interval in seconds
    return 1.0f / rate;
}

void LIS2DH12::getAcceleration(int16_t* x, int16_t* y, int16_t* z)
{
  readReg(REG_OUT_X_L|0x80, _sDataBuffer, 6);
  *x = (((int16_t)_sDataBuffer[1]) << 8) | _sDataBuffer[0];
  *y = (((int16_t)_sDataBuffer[3]) << 8) | _sDataBuffer[2];
  *z = (((int16_t)_sDataBuffer[5]) << 8) | _sDataBuffer[4];
}

/*!
 * @brief Check if new accelerometer data is available from the LIS2DH12 sensor.
 * @return True if new data is available; otherwise, false.
 */
bool LIS2DH12::isDataAvailable() {
    uint8_t status = 0;
    // Read the STATUS_REG register to get the status bits
    readReg(REG_STATUS_REG, &status, 1);
    return (status & 0x08) != 0;  // 0x08 = 0000 1000 in binary, which checks if the third bit is set
}


int32_t LIS2DH12::readAccX()
{
  int8_t sensorData[2];
  int32_t a;

  readReg(REG_OUT_X_L|0x80, sensorData, 2);

  // Combine sensorData[0] and sensorData[1] into a 16-bit signed value
  int16_t rawData = (sensorData[1] << 8) | (sensorData[0] & 0xFF);

  // Apply scale and sign based on the platform
  #if defined(__AVR__) 
    a = - (rawData * (uint8_t)_mgScaleVel_1)/(uint8_t)_mgScaleVel_2;
  #else 
    a = (rawData * (uint8_t)_mgScaleVel_1)/(uint8_t)_mgScaleVel_2;
  #endif

  return a;
}


int32_t LIS2DH12::readAccY()
{
  int8_t sensorData[2];
  int32_t a;

  readReg(REG_OUT_Y_L | 0x80, sensorData, 2);

  // Combine sensorData[0] and sensorData[1] into a 16-bit signed value
  int16_t rawData = (sensorData[1] << 8) | (sensorData[0] & 0xFF);

  // Apply scaling factor based on the platform
  #if defined(__AVR__) 
    a = - (rawData * (uint8_t)_mgScaleVel_1)/(uint8_t)_mgScaleVel_2;
  #else 
    a = (rawData * (uint8_t)_mgScaleVel_1)/(uint8_t)_mgScaleVel_2;
  #endif

  return a;
}

int32_t LIS2DH12::readAccZ()
{
  int8_t sensorData[2];
  int32_t a;

  readReg(REG_OUT_Z_L | 0x80, sensorData, 2);

  // Combine sensorData[0] and sensorData[1] into a 16-bit signed value
  int16_t rawData = (sensorData[1] << 8) | (sensorData[0] & 0xFF);

  // Apply scaling factor based on the platform
  #if defined(__AVR__) 
    a = - (rawData * (uint8_t)_mgScaleVel_1)/(uint8_t)_mgScaleVel_2;
  #else 
    a = (rawData * (uint8_t)_mgScaleVel_1)/(uint8_t)_mgScaleVel_2;
  #endif

  return a;
}


void LIS2DH12::setMode(eResolutionMode_t newMode) 
{
  mode = newMode;
}

void LIS2DH12::setRange(eRange_t range)
{
  switch(range){
    case eLIS2DH12_2g:
      _mgScaleVel_1 = 16;
      break;
    case eLIS2DH12_4g:
      _mgScaleVel_1 = 32;
      break;
    case eLIS2DH12_8g:
      _mgScaleVel_1 = 64;
      break;
    default:
      _mgScaleVel_1 = 192;
      break;
  }
  // Adjust the scale factor based on the mode
  if (mode == NORMAL_MODE) {
    _mgScaleVel_1 >>= 2; // Right shift by 2 bits
    _mgScaleVel_2  = 64;
  } else if (mode == HIGH_RESOLUTION_MODE) {
    _mgScaleVel_1 >>= 4; // Right shift by 4 bits
    _mgScaleVel_2  = 16;
  }

  DBG(range);
  writeReg(REG_CTRL_REG4,&range,1);
}

void LIS2DH12::setAcquireRate(ePowerMode_t rate)
{

    uint8_t reg1, reg4;

    readReg(REG_CTRL_REG1, &reg1, 1);
    readReg(REG_CTRL_REG4, &reg4, 1);

    // Clear the ODR bits (bits 4 to 7) and set them according to the provided 'odr' value
    reg1 = (reg1 & 0x0F) | rate;

    // Set the mode based on the public 'mode' member variable
    switch (mode) {
        case LOW_POWER_MODE:  // Low-power mode
            reg1 |= (1 << 3);   // Set LPen bit for Low Power Mode
            reg4 &= ~(1 << 3);  // Ensure High-Resolution bit (HR) is cleared
            break;
        case NORMAL_MODE:  // Normal mode
            reg1 &= ~(1 << 3);  // Clear LPen bit for Normal Mode
            reg4 &= ~(1 << 3);  // Ensure High-Resolution bit (HR) is cleared
            break;
        case HIGH_RESOLUTION_MODE:  // High-resolution mode
            reg1 &= ~(1 << 3);  // Clear LPen bit for High-Resolution Mode
            reg4 |= (1 << 3);   // Set HR bit for High-Resolution Mode
            break;
        default:
            DBG("Invalid mode");
            return;
    }

    DBG(reg1);
    DBG(reg4);

    writeReg(REG_CTRL_REG1, &reg1, 1);
    writeReg(REG_CTRL_REG4, &reg4, 1);

  
}

uint8_t LIS2DH12::getID()
{
  uint8_t identifier; 
  readReg(REG_CARD_ID,&identifier,1);
  return identifier;
}

void LIS2DH12::writeReg(uint8_t reg, const void * pBuf, size_t size)
{
  if(pBuf == NULL){
	  DBG("pBuf ERROR!! : null pointer");
  }
  uint8_t * _pBuf = (uint8_t *)pBuf;
  _pWire->beginTransmission(_deviceAddr);
  _pWire->write(&reg, 1);

  for(uint16_t i = 0; i < size; i++){
    _pWire->write(_pBuf[i]);
  }
  _pWire->endTransmission();
}

uint8_t LIS2DH12::readReg(uint8_t reg, void* pBuf, size_t size)
{
  if(pBuf == NULL){
    DBG("pBuf ERROR!! : null pointer");
  }
  uint8_t * _pBuf = (uint8_t *)pBuf;
  _pWire->beginTransmission(_deviceAddr);
  _pWire->write(&reg, 1);
  if( _pWire->endTransmission() != 0){
      return 0;
  }
  _pWire->requestFrom(_deviceAddr, (uint8_t) size);
  for(uint16_t i = 0; i < size; i++){
	  
    _pBuf[i] = _pWire->read();
  }
  return size;
}

void LIS2DH12::setInt1Th(uint8_t threshold)
{
    uint8_t reg = (threshold * 1024)/_mgScaleVel_1;
    uint8_t reg1 = 0x08;
    uint8_t reg2 = 0x00;
    uint8_t data = 0x40;

    writeReg(REG_CTRL_REG2,&reg2,1);
    writeReg(REG_CTRL_REG3,&data,1);
    writeReg(REG_CTRL_REG5,&reg1,1);
    writeReg(REG_CTRL_REG6,&reg2,1);
    writeReg(REG_INT1_THS,&reg,1);
    readReg(REG_CTRL_REG5,&reg2,1);
    DBG(reg2);
    readReg(REG_CTRL_REG3,&reg2,1);
    DBG(reg2);
}

void LIS2DH12::setInt2Th(uint8_t threshold)
{
    uint8_t reg = (threshold * 1024)/_mgScaleVel_1;
    uint8_t reg1 = 0x02;
    uint8_t reg2 = 0x00;
    uint8_t data = 0x40;

    writeReg(REG_CTRL_REG2,&reg2,1);
    writeReg(REG_CTRL_REG3,&reg2,1);
    writeReg(REG_CTRL_REG5,&reg1,1);
    writeReg(REG_CTRL_REG6,&data,1);
    writeReg(REG_INT2_THS,&reg,1);
    readReg(REG_CTRL_REG5,&reg2,1);
    DBG(reg2);
    readReg(REG_CTRL_REG6,&reg2,1);
    DBG(reg2);
}

void LIS2DH12::enableInterruptEvent(eInterruptSource_t source,eInterruptEvent_t event)
{
   uint8_t data = 0;
  data = 0x80 | event;
  DBG(data);
  if(source == eINT1)
    writeReg(REG_INT1_CFG,&data,1);
  else
    writeReg(REG_INT2_CFG,&data,1);

  readReg(REG_INT1_CFG,&data,1);
  DBG(data);
}

bool LIS2DH12::getInt1Event(eInterruptEvent_t event)
{
  uint8_t data = 0;
  bool ret = false;
  readReg(REG_INT1_SRC,&data,1);
  DBG(data,HEX);
  if(data & 0x40){
    switch(event){
      case eXLowerThanTh:
        if(!(data & 0x01))
          ret = true;
        break;
      case eXHigherThanTh:
        if((data & 0x02) == 0x02)
          ret = true;
        break;
      case eYLowerThanTh:
        if(!(data & 0x04))
          ret = true;
        break;
      case eYHigherThanTh:
        if((data & 0x08) == 0x08)
          ret = true;
        break;
      case eZLowerThanTh:
        if(!(data & 0x10))
          ret = true;
        break;
      case eZHigherThanTh:
        if((data & 0x20) == 0x20)
          ret = true;
        break;
      default:
        ret = false;
    }
  }else{
    ret = false;
  }
  return ret;
}

bool LIS2DH12::getInt2Event(eInterruptEvent_t event)
{
  uint8_t data = 0;
  bool ret = false;
  readReg(REG_INT2_SRC,&data,1);
  DBG(data,HEX);
  if(data & 0x40){
    switch(event){
      case eXLowerThanTh:
        if(!(data & 0x01))
          ret = true;
        break;
      case eXHigherThanTh:
        if((data & 0x02) == 0x02)
          ret = true;
        break;
      case eYLowerThanTh:
        if(!(data & 0x04))
          ret = true;
        break;
      case eYHigherThanTh:
        if((data & 0x08) == 0x08)
          ret = true;
        break;
      case eZLowerThanTh:
        if(!(data & 0x10))
          ret = true;
        break;
      case eZHigherThanTh:
        if((data & 0x20) == 0x20)
          ret = true;
        break;
      default:
        ret = false;
    }
  }else{
    ret = false;
  }
  return ret;
}