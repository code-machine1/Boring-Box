#ifndef _INA226_H
#define _INA226_H

#include "at32f415.h"
#include <stdint.h>
#include <stdbool.h>

/*IIC 地址---------------------------------------------------------------------------------------------*/
#define INA226_W 0x80
#define INA226_R 0x81
/*寄存器地址--------------------------------------------------------------------------------------------*/
#define	INA226_Configuration                          0x00
#define INA226_Shuntvoltage                           0x01
#define INA226_Busvoltage                             0x02
#define INA226_Power                                  0x03
#define INA226_Current                                0x04
#define INA226_Calibration                            0x05
#define	INA226_Mask                                   0x06
#define	INA226_AlertLimit                             0x07
#define	INA226_ManufacturerID                         0xFE
#define	INA226_DieID                                  0xFF
/* INA226_curation Bit15-0 --------------------------------------------------------------------------*/
#define RST 			            0  	  // 0   设置成1复位 （Bit15）
#define Reservation                 0x04  // 100 （Bit14-12 保留）
#define AVG 						0x01  // 001 平均次数 4 （Bit11-9）
#define VBUSCT  					0x04  // 100 总线电压转换时间 1.1ms （Bit116-8）
#define VSHCT		  				0x04  // 100 分流电压转换时间 1.1ms（Bit3-5）
#define	MODE	                    0x07  // 111 运行模式  连续检测（默认）（Bit0–2）

#define Configuration_H (RST << 7)|(Reservation << 4)|(AVG << 1)|(VBUSCT >> 2)
#define Configuration_L ((VBUSCT & 0x03) << 6)|(VSHCT << 3)|(MODE)

#define Calibration_H 0x03
#define Calibration_L 0xC5

typedef int16_t (*i2c_write_byte)(uint8_t, uint8_t, uint8_t *, uint8_t);
typedef int16_t (*i2c_read_byte)(uint8_t, uint8_t, uint8_t *, uint8_t);

typedef struct {
    /** Component mandatory fields **/
    uint8_t ina226_i2c_address;
    i2c_write_byte  write_reg;
    i2c_read_byte   read_reg;
} ina226_t;



#endif