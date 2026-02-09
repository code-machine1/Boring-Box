#ifndef _INA226_HANDLE_H
#define _INA226_HANDLE_H

#include "soft_i2c.h"

/* IIC 地址 ----------------------------------------------------------------------------------------------*/
#define INA226_W 0x80
#define INA226_R 0x81

/* INA226 寄存器地址 --------------------------------------------------------------------------------------*/
#define INA226_Configuration                          0x00
#define INA226_Shuntvoltage                           0x01
#define INA226_Busvoltage                             0x02
#define INA226_Power                                  0x03
#define INA226_Current                                0x04
#define INA226_Calibration                            0x05
#define INA226_Mask                                   0x06
#define INA226_AlertLimit                             0x07
#define INA226_ManufacturerID                         0xFE
#define INA226_DieID                                  0xFF

/* INA226 配置寄存器 Bit15-0 --------------------------------------------------------------------------*/
#define RST                            0         
#define Reservation                    0x04         
#define AVG                            0x01         
#define VBUSCT                         0x04     
#define VSHCT                          0x04     
#define MODE                           0x07    

/* 方案4: 高精度配置 (0.25mA/LSB) */
#define Calibration_H                   0x08    /* 校准寄存器高字节: 0x08 */
#define Calibration_L                   0x00    /* 校准寄存器低字节: 0x00, 合并为 0x0800 = 2048 */

#define Configuration_H ((RST << 7) | (Reservation << 4) | (AVG << 1) | (VBUSCT >> 2))
#define Configuration_L (((VBUSCT & 0x03) << 6) | (VSHCT << 3) | (MODE))

#define INA226_HANDLE_TIME             5

/* INA226 计算常数 (方案4: 0.25mA/LSB) */
#define INA226_CURRENT_LSB_MA          0.25f   /* 电流LSB: 0.25 mA */
#define INA226_POWER_LSB_W             0.00625f /* 功率LSB: 0.00625 W (25 * 0.00025) */
#define INA226_SHUNT_LSB_V             0.0000025f /* 分流电压LSB: 2.5μV */
#define INA226_BUS_LSB_V               0.00125f   /* 总线电压LSB: 1.25mV */

/* INA226 硬件参数 */
#define INA226_SHUNT_RESISTOR          0.01f   /* 分流电阻: 10mΩ (0.01Ω) */

typedef struct ina226_cfg
{
    float BusVoltage;     /* 单位: V */
    float ShuntVoltage;   /* 单位: V */
    float Current;        /* 单位: mA (方案4: 0.25mA/LSB) */
    float Power;          /* 单位: W */
} ina226_t;

extern ina226_t ina226;

/* 基本功能函数 */
void INA226_WriteReg(uint8_t Register, uint8_t Data_H, uint8_t Data_L);
uint32_t INA226_ReadReg(uint8_t RegAddress);
void INA226_Init(ina226_t *ina226);
void INA226_Reset(void);

/* 数据读取函数 */
float INA226_GetShuntVoltage(void);    /* 返回: V */
float INA226_GetBusVoltage(void);      /* 返回: V */
float INA226_GetCurrent(void);         /* 返回: mA (方案4: 0.25mA/LSB) */
float INA226_GetPower(void);           /* 返回: W (方案4: 0.00625W/LSB) */
float INA226_GetPower_mW(void);        /* 返回: mW */

/* 高级功能函数 */
void INA226_CalibrationCheck(void);    /* 校准检查 */
void INA226_SelfTest(void);            /* 自测试 */
uint16_t INA226_GetManufacturerID(void);
uint16_t INA226_GetDieID(void);

/* 主处理函数 */
void ina226_handle(void);

#endif /* _INA226_HANDLE_H */