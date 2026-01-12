#ifndef _INA226_HANDLE_H
#define _INA226_HANDLE_H

#include "at32f421.h"
#include "soft_i2c.h"
/*IIC ???---------------------------------------------------------------------------------------------*/
#define INA226_W 0x80
#define INA226_R 0x81
/*????????--------------------------------------------------------------------------------------------*/
#define INA226_Configuration 0x00
#define INA226_Shuntvoltage 0x01
#define INA226_Busvoltage 0x02
#define INA226_Power 0x03
#define INA226_Current 0x04
#define INA226_Calibration 0x05
#define INA226_Mask 0x06
#define INA226_AlertLimit 0x07
#define INA226_ManufacturerID 0xFE
#define INA226_DieID 0xFF
/* INA226_curation Bit15-0 --------------------------------------------------------------------------*/
#define RST 0
#define Reservation 0x04
#define AVG 0x01
#define VBUSCT 0x04
#define VSHCT 0x04
#define MODE 0x07

#define Configuration_H (RST << 7) | (Reservation << 4) | (AVG << 1) | (VBUSCT >> 2)
#define Configuration_L ((VBUSCT & 0x03) << 6) | (VSHCT << 3) | (MODE)

#define INA226_HANDLE_TIME 50

/* INA226_Calibration ---------------------------------------------------------------------------------*/
// #define Calibration_H 0x04
// #define Calibration_L 0x00

// #define Calibration_H 0x06
// #define Calibration_L 0x8E

// #define Calibration_H 0x03
// #define Calibration_L 0xC5

#define Calibration_H 0x03
#define Calibration_L 0xFA

typedef struct ina226_cfg
{
  float BusVoltage;
  float ShuntVoltage;
  float Current;
  float Power;
} ina226_t;

extern ina226_t ina226;

void INA226_Init(ina226_t *ina226);
void INA226_Reset(void);
float INA226_GetShuntVoltage(void); //???????? =  ?????? * LSB(2.5uA)
float INA226_GetBusVoltage(void);   //??????? =  ?????? * LSB(1.25mV)
float INA226_GetCurrent(void);      //????? = ?????? * Current_LSB(0.05mA)

float INA226_GetPower(void); //???? = ?????? * Power_LSB(1.25mW)

void ina226_handle(void);

#endif