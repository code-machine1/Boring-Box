#include "ina226_handle.h"
#include "perf_counter.h"
#include "freertos_app.h"
ina226_t ina226;

/* I2C 写寄存器函数 */
void INA226_WriteReg(uint8_t Register, uint8_t Data_H, uint8_t Data_L)
{
    i2c_start();
    i2c_send_byte(INA226_W);
    i2c_wait_ack(I2C_ACK_TIMEOUT);
    i2c_send_byte(Register);
    i2c_wait_ack(I2C_ACK_TIMEOUT);
    i2c_send_byte(Data_H);
    i2c_wait_ack(I2C_ACK_TIMEOUT);
    i2c_send_byte(Data_L);
    i2c_wait_ack(I2C_ACK_TIMEOUT);
    i2c_stop();
}

/* I2C 读寄存器函数 */
uint32_t INA226_ReadReg(uint8_t RegAddress)
{
    uint32_t Data;
    i2c_start();
    i2c_send_byte(INA226_W);
    i2c_wait_ack(I2C_ACK_TIMEOUT);
    i2c_send_byte(RegAddress);
    i2c_wait_ack(I2C_ACK_TIMEOUT);
    i2c_start();
    i2c_send_byte(INA226_R);
    i2c_wait_ack(I2C_ACK_TIMEOUT);
    Data = i2c_receive_byte();
    i2c_ack();
    Data = (Data << 8) | i2c_receive_byte();
    i2c_stop();
    return Data;
}

/* INA226 初始化 */
void INA226_Init(ina226_t *ina226)
{
    /* 写入配置寄存器 */
    INA226_WriteReg(INA226_Configuration, Configuration_H, Configuration_L);
    /* 写入校准寄存器: 2048 (0x0800) - 方案4 */
    INA226_WriteReg(INA226_Calibration, Calibration_H, Calibration_L);

    /* 初始化数据结构 */
    if (ina226 != NULL)
    {
        ina226->BusVoltage = 0;
        ina226->Current = 0;
        ina226->Power = 0;
        ina226->ShuntVoltage = 0;
    }
}

/* INA226 复位 */
void INA226_Reset(void)
{
    INA226_WriteReg(INA226_Configuration, Configuration_H, Configuration_L);
    INA226_WriteReg(INA226_Calibration, Calibration_H, Calibration_L);
}

/* 获取分流电压 (V) */
float INA226_GetShuntVoltage(void)
{
    /*
     * 注意: 分流电压寄存器是有符号的
     * LSB = 2.5μV = 0.0000025V
     */
    int16_t raw_value = (int16_t)INA226_ReadReg(INA226_Shuntvoltage);
    return (float)raw_value * INA226_SHUNT_LSB_V;
}

/* 获取总线电压 (V) */
float INA226_GetBusVoltage(void)
{
    /*
     * 总线电压寄存器是无符号的
     * LSB = 1.25mV = 0.00125V
     */
    uint16_t raw_value = (uint16_t)INA226_ReadReg(INA226_Busvoltage);
    return (float)raw_value * INA226_BUS_LSB_V;
}

/* 获取电流 (mA) - 返回有符号值，正负表示方向 */
float INA226_GetCurrent(void)
{
    /*
     * 电流寄存器是有符号的
     * LSB = 0.25 mA
     * 正值: 电流从VIN+流向VIN-
     * 负值: 电流从VIN-流向VIN+
     */
    int16_t raw_value = (int16_t)INA226_ReadReg(INA226_Current);
    return (float)raw_value * INA226_CURRENT_LSB_MA;
}

/* 获取功率 (W) */
float INA226_GetPower(void)
{
    /*
     * 功率寄存器是无符号的
     * 方案4: LSB = 0.00625 W
     */
    uint16_t raw_value = (uint16_t)INA226_ReadReg(INA226_Power);
    return (float)raw_value * INA226_POWER_LSB_W;
}

/* 获取功率 (mW) */
float INA226_GetPower_mW(void)
{
    float power_w = INA226_GetPower();
    return power_w * 1000.0f;
}

/* 校准检查函数 */
void INA226_CalibrationCheck(void)
{
    uint16_t cal_reg = (uint16_t)INA226_ReadReg(INA226_Calibration);
    /* 这里可以添加调试打印，实际使用时根据需求实现 */
    /*
    printf("校准寄存器: 0x%04X (%u)\n", cal_reg, cal_reg);

    if (cal_reg != 2048) {  // 0x0800 = 2048
        printf("警告: 校准寄存器值不匹配！\n");
        printf("期望值: 2048 (0x0800)\n");
        printf("实际值: %u (0x%04X)\n", cal_reg, cal_reg);
    }
    */
    /* 计算实际LSB值 */
    float actual_current_lsb = 0.00512f / ((float)cal_reg * INA226_SHUNT_RESISTOR);
    float actual_power_lsb = 25.0f * actual_current_lsb;
    /*
    printf("理论电流LSB: %.6f mA/LSB\n", INA226_CURRENT_LSB_MA);
    printf("实际电流LSB: %.6f mA/LSB\n", actual_current_lsb * 1000.0f);
    printf("理论功率LSB: %.6f W/LSB\n", INA226_POWER_LSB_W);
    printf("实际功率LSB: %.6f W/LSB\n", actual_power_lsb);
    */
}

/* 自测试函数 */
void INA226_SelfTest(void)
{
    /* 1. 检查制造商ID (应为0x5449) */
    uint16_t manu_id = (uint16_t)INA226_ReadReg(INA226_ManufacturerID);
    /* 2. 检查芯片ID (应为0x2260) */
    uint16_t die_id = (uint16_t)INA226_ReadReg(INA226_DieID);
    /*
    printf("制造商ID: 0x%04X ", manu_id);
    if (manu_id == 0x5449) {
        printf("(正确: TI)\n");
    } else {
        printf("(错误! 期望: 0x5449)\n");
    }

    printf("芯片ID: 0x%04X ", die_id);
    if (die_id == 0x2260) {
        printf("(正确: INA226)\n");
    } else {
        printf("(错误! 期望: 0x2260)\n");
    }
    */
    /* 3. 校准检查 */
    INA226_CalibrationCheck();
}

/* 获取制造商ID */
uint16_t INA226_GetManufacturerID(void)
{
    return (uint16_t)INA226_ReadReg(INA226_ManufacturerID);
}

/* 获取芯片ID */
uint16_t INA226_GetDieID(void)
{
    return (uint16_t)INA226_ReadReg(INA226_DieID);
}

/* INA226 主处理函数 */
void ina226_handle(void)
{
#if 1
    static uint8_t state = 0;

    switch (state)
    {
    case 0:
        /* 读取总线电压 */
        ina226.BusVoltage = INA226_GetBusVoltage();
        state ++;
        break;

    case 1:
        /* 读取电流 */
        ina226.Current = INA226_GetCurrent();

        /* 零点漂移校正  */
        if (ina226.Current == 0.25f || ina226.Current == -0.25f)
        {
            ina226.Current = 0;
        }

        if (ina226.Current < 0.0f)
        {
            ina226.Current = 0;
        }

        /* 读取功率 */
        // ina226.Power = INA226_GetPower_mW();

        /* 分流电压可选项（如果需要） */
        // ina226.ShuntVoltage = INA226_GetShuntVoltage();

        /* 发送到队列 */
        if (xQueueSend(ina226_value_handle, &ina226, 0) == pdPASS)
        {
            /* 发送成功 */
            //printf("send ok\r\n");
        }
        else
        {
            //printf("send error\r\n");
        }

        state = 0;
        break;

    default:
        state = 0;
        break;
    }

#endif
}