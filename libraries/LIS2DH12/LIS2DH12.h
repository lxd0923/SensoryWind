#ifndef LIS2DH12_H
#define LIS2DH12_H

#include "Arduino.h"
#include <Wire.h>
#include <SPI.h>

//#define ENABLE_DBG

#ifdef ENABLE_DBG
#define DBG(...) {Serial.print("["); Serial.print(__FUNCTION__); Serial.print("(): "); Serial.print(__LINE__); Serial.print(" ] "); Serial.println(__VA_ARGS__);}
#else
#define DBG(...)
#endif

#define LIS2DH12_ADDR    0x18


#define AXIS_X 0  ///< X-axis index
#define AXIS_Y 1  ///< Y-axis index
#define AXIS_Z 2  ///< Z-axis index

class LIS2DH12
{
public:
  #define REG_CARD_ID      0x0F     ///<Chip id
  #define REG_CTRL_REG1    0x20     ///<Control register 1
  #define REG_CTRL_REG4    0x23     ///<Control register 4
  #define REG_CTRL_REG2    0x21     ///<Control register 2
  #define REG_CTRL_REG3    0x22     ///<Control register 3
  #define REG_CTRL_REG5    0x24     ///<Control register 5
  #define REG_CTRL_REG6    0x25     ///<Control register 6
  #define REG_STATUS_REG   0x27     ///<Status register
  #define REG_OUT_X_L      0x28     ///<The low order of the X-axis acceleration register
  #define REG_OUT_X_H      0x29     ///<The high point of the X-axis acceleration register
  #define REG_OUT_Y_L      0x2A     ///<The low order of the Y-axis acceleration register
  #define REG_OUT_Y_H      0x2B     ///<The high point of the Y-axis acceleration register
  #define REG_OUT_Z_L      0x2C     ///<The low order of the Z-axis acceleration register
  #define REG_OUT_Z_H      0x2D     ///<The high point of the Z-axis acceleration register
  #define REG_INT1_THS     0x32     ///<Interrupt source 1 threshold
  #define REG_INT2_THS     0x36     ///<Interrupt source 2 threshold
  #define REG_INT1_CFG     0x30     ///<Interrupt source 1 configuration register
  #define REG_INT2_CFG     0x34     ///<Interrupt source 2 configuration register
  #define REG_INT1_SRC     0x31     ///<Interrupt source 1 status register
  #define REG_INT2_SRC     0x35     ///<Interrupt source 2 status register
  
public:
/**
 * @fn  eResolutionMode_t 
 * @brief Enumeration for selecting the resolution mode of the LIS2DH12 sensor.
 * This enumeration defines the different resolution modes that can be set for the 
 * LIS2DH12 sensor. Each mode offers a different balance between power consumption and 
 * resolution. The selected mode affects the noise level, power consumption, and 
 * the precision of the sensor's output.
 * 
 */
typedef enum{
    LOW_POWER_MODE = 0,
    NORMAL_MODE,
    HIGH_RESOLUTION_MODE
}eResolutionMode_t;

/**
 * @fn  ePowerMode_t
 * @brief  Power mode selection, determine the frequency of data collection Represents the number of data collected per second
 */
typedef enum{
    ePowerDown_0Hz  = 0,
    eDataRate_1Hz   =  0x10,
    eDataRate_10Hz  = 0x20,
    eDataRate_25Hz  = 0x30,
    eDataRate_50Hz  = 0x40,
    eDataRate_100Hz = 0x50,
    eDataRate_200Hz = 0x60,
    eDataRate_400Hz = 0x70
}ePowerMode_t;

/**
 * @fn  eRange_t
 * @brief  Sensor range selection
 */
typedef enum{
    eLIS2DH12_2g = 0x00,/**<±2g>*/
    eLIS2DH12_4g = 0x10,/**<±4g>*/
    eLIS2DH12_8g = 0x20,/**<±8g>*/
    eLIS2DH12_16g = 0x30/**<±16g>*/
}eRange_t;

/**
 * @fn  eInterruptEvent_t
 * @brief  Interrupt event
 */
typedef enum{
  eXLowerThanTh = 0x01,    /**<The acceleration in the x direction is less than the threshold>*/
  eXHigherThanTh = 0x02,  /**<The acceleration in the x direction is greater than the threshold>*/
  eYLowerThanTh = 0x04,   /**<The acceleration in the y direction is less than the threshold>*/
  eYHigherThanTh = 0x08,  /**<The acceleration in the y direction is greater than the threshold>*/
  eZLowerThanTh = 0x10,   /**<The acceleration in the z direction is less than the threshold>*/
  eZHigherThanTh = 0x20,  /**<The acceleration in the z direction is greater than the threshold>*/
  eEventError = 0,        /**< No event>*/
}eInterruptEvent_t;

/**
 * @fn  eInterruptSource_t
 * @brief  Interrupt pin selection
 */
typedef enum{
  eINT1 = 0, /**<int1 >*/
  eINT2,     /**<int2>*/
}eInterruptSource_t;

public:

  /**
   * @fn DFRobot_LIS2DH12
   * @brief Constructor 
   * @param pWire I2c controller
   * @param addr  I2C address(0x19/0x18)
   */
  LIS2DH12(TwoWire* pWire = &Wire,uint8_t addr = LIS2DH12_ADDR);

  eResolutionMode_t mode;  // Public variable to store the current mode

  void setMode(eResolutionMode_t newMode);


  /**
   * @fn begin
   * @brief Initialize the function
   * @return true(Succeed)/false(Failed)
   */
  bool begin(void);

  /**
   * @fn isDataAvailable
   * @brief Check if new data is available from the sensor.
   * @return true if data is available, false otherwise.
   */
  bool isDataAvailable(void);

  /**
   * @fn setRange
   * @brief Set the measurement range
   * @param range Range(g)
   * @n           eLIS2DH12_2g, //2g
   * @n           eLIS2DH12_4g, //4g
   * @n           eLIS2DH12_8g, //8g
   * @n           eLIS2DH12_16g, //16g
   */
  void setRange(eRange_t range);

  /**
   * @fn setAcquireRate
   * @brief Set data measurement rate
   * @param rate rate(HZ)
   * @n          ePowerDown_0Hz 
   * @n          eDataRate_1Hz 
   * @n          eDataRate_10Hz 
   * @n          eDataRate_25Hz 
   * @n          eDataRate_50Hz 
   * @n          eDataRate_100Hz
   * @n          eDataRate_200Hz
   * @n          eDataRate_400Hz
   */
  void setAcquireRate(ePowerMode_t rate);

  uint8_t getAcquireRate(void);

  // Method to calculate the time interval based on the acquisition rate
  float getTimeInterval(void);

  /**
   * @fn getID
   * @brief Get chip id
   * @return 8 bit serial number
   */
  uint8_t getID();

  /**
   * @fn getAcceleration
   * @brief Get the acceleration values in the x, y, and z directions.
   * @param x Pointer to store the acceleration (raw data) in the x direction.
   * @param y Pointer to store the acceleration (raw data) in the y direction.
   * @param z Pointer to store the acceleration (raw data) in the z direction.
   * @details This function retrieves the acceleration data for all three axes and stores them in the provided pointers.
   *          The measurement range is set by the setRange() function and could be ±2g, ±4g, ±8g, or ±16g.
   */
  void getAcceleration(int16_t* x, int16_t* y, int16_t* z);


  /**
   * @fn readAccX
   * @brief Get the acceleration in the x direction
   * @return acceleration from x (unit:g), the mearsurement range is ±100g or ±200g, set by setRange() function.
   */
  int32_t readAccX();

  /**
   * @fn readAccY
   * @brief Get the acceleration in the y direction
   * @return acceleration from y(unit:g), the mearsurement range is ±100g or ±200g, set by setRange() function.
   */
  int32_t readAccY();

  /**
   * @fn readAccZ
   * @brief Get the acceleration in the z direction
   * @return acceleration from z(unit:g), the mearsurement range is ±100g or ±200g, set by setRange() function.
   */
  int32_t readAccZ();
  
  /**
   * @fn setInt1Th
   * @brief Set the threshold of interrupt source 1 interrupt
   * @param threshold The threshold is within the measurement range(unit:g)
   */
  void setInt1Th(uint8_t threshold);

  /**
   * @fn setInt2Th
   * @brief Set interrupt source 2 interrupt generation threshold
   * @param threshold The threshold is within the measurement range(unit:g）
   */
  void setInt2Th(uint8_t threshold);

  /**
   * @fn enableInterruptEvent
   * @brief Enable interrupt
   * @param source Interrupt pin selection
   * @n           eINT1 = 0,/<int1 >/
   * @n           eINT2,/<int2>/
   * @param event Interrupt event selection
   * @n           eXLowerThanTh ,/<The acceleration in the x direction is less than the threshold>/
   * @n           eXHigherThanTh ,/<The acceleration in the x direction is greater than the threshold>/
   * @n           eYLowerThanTh,/<The acceleration in the y direction is less than the threshold>/
   * @n           eYHigherThanTh,/<The acceleration in the y direction is greater than the threshold>/
   * @n           eZLowerThanTh,/<The acceleration in the z direction is less than the threshold>/
   * @n           eZHigherThanTh,/<The acceleration in the z direction is greater than the threshold>/
   */
  void enableInterruptEvent(eInterruptSource_t source, eInterruptEvent_t event);

  /**
   * @fn getInt1Event
   * @brief Check whether the interrupt event'event' is generated in interrupt 1
   * @param event Interrupt event
   * @n           eXLowerThanTh ,/<The acceleration in the x direction is less than the threshold>/
   * @n           eXHigherThanTh ,/<The acceleration in the x direction is greater than the threshold>/
   * @n           eYLowerThanTh,/<The acceleration in the y direction is less than the threshold>/
   * @n           eYHigherThanTh,/<The acceleration in the y direction is greater than the threshold>/
   * @n           eZLowerThanTh,/<The acceleration in the z direction is less than the threshold>/
   * @n           eZHigherThanTh,/<The acceleration in the z direction is greater than the threshold>/
   * @return true Generated/false Not generated
   */
  bool getInt1Event(eInterruptEvent_t event);

  /**
   * @fn getInt2Event
   * @brief Check whether the interrupt event'event' is generated in interrupt 1
   * @param event Interrupt event
   * @n           eXLowerThanTh ,/<The acceleration in the x direction is less than the threshold>/
   * @n           eXHigherThanTh ,/<The acceleration in the x direction is greater than the threshold>/
   * @n           eYLowerThanTh,/<The acceleration in the y direction is less than the threshold>/
   * @n           eYHigherThanTh,/<The acceleration in the y direction is greater than the threshold>/
   * @n           eZLowerThanTh,/<The acceleration in the z direction is less than the threshold>/
   * @n           eZHigherThanTh,/<The acceleration in the z direction is greater than the threshold>/
   * @return true Generated/false Not generated
   */
  bool getInt2Event(eInterruptEvent_t event);

  protected:
  /**
   * @fn readReg
   * @brief read data from sensor chip register
   * @param reg chip register 
   * @param pBuf  buf for store data to read 
   * @param size  number of data to read
   * @return The number of successfully read data
   */
  uint8_t readReg(uint8_t reg,void * pBuf ,size_t size);
  
  /**
   * @fn writeReg
   * @brief Write data to sensor register 
   * @param reg register
   * @param pBuf  buf for store data to write 
   * @param size  The number of the data in pBuf
   */
  void  writeReg(uint8_t reg,const void *pBuf,size_t size); 

private:
  uint8_t _deviceAddr;
  TwoWire *_pWire;
  uint8_t _mgScaleVel_1 = 16;
  uint8_t _mgScaleVel_2 = 256;
  uint8_t _reset = 0;

  uint8_t _sDataBuffer[6];

  int16_t _accX, _accY, _accZ;
  int16_t _maxAccX, _maxAccY, _maxAccZ;
  int16_t _minAccX, _minAccY, _minAccZ;

};
#endif