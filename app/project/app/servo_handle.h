#ifndef SERVO_HANDLE_H
#define SERVO_HANDLE_H

#include "at32f421.h"
#include "freertos_app.h"

// 舵机ID定义
typedef enum {
    SERVO_LID = 0,      // 盖子舵机
    SERVO_ROD,          // 推杆舵机
    SERVO_NUM           // 舵机总数
} Servo_ID;

// 舵机状态枚举
typedef enum {
    SERVO_IDLE = 0,     // 空闲
    SERVO_MOVING,       // 运动中
    SERVO_STALL,        // 堵转
    SERVO_ERROR         // 错误
} Servo_State;

// 舵机配置结构体（适配MG90S）
typedef struct {
    uint16_t min_angle;     // 最小角度(0°)
    uint16_t max_angle;     // 最大角度(180°)
    uint16_t min_pulse_us;  // 最小脉宽(500us = 0.5ms)
    uint16_t max_pulse_us;  // 最大脉宽(2500us = 2.5ms)
    uint16_t zero_offset;   // 零位偏移(校准用)
    uint8_t invert;         // 方向反转(0:正常, 1:反转)
    uint8_t smooth_factor;  // 平滑系数(0-100)
} Servo_Config;

// 舵机控制结构体
typedef struct {
    Servo_State state;
    uint16_t current_angle;     // 当前角度
    uint16_t target_angle;      // 目标角度
    uint32_t current_compare;   // 当前比较值
    uint32_t last_update_time;  // 最后更新时间
    uint8_t is_initialized;     // 初始化标志
    float speed_factor;         // 速度因子(0.0-1.0)
    
    // 堵转检测相关
    uint8_t stall_detected;     // 堵转检测标志
    uint32_t stall_start_time;  // 堵转开始时间
    uint8_t stall_count;        // 连续堵转计数
} Servo_Control;

// 函数声明

// 初始化函数
void Servo_System_Init(void);
void Servo_Init(Servo_ID servo);

// 基本控制函数
uint8_t Set_Servo_Angle(Servo_ID servo, uint16_t angle);  // 返回是否成功
uint8_t Set_Servo_Angle_Speed(Servo_ID servo, uint16_t angle, float speed);
void Set_Servo_Compare_Value(Servo_ID servo, uint32_t compare_value);
void Stop_Servo(Servo_ID servo);
void Disable_Servo(Servo_ID servo);
void Enable_Servo(Servo_ID servo);

// 状态获取函数
uint16_t Get_Servo_Angle(Servo_ID servo);
uint32_t Get_Servo_Compare_Value(Servo_ID servo);
Servo_State Get_Servo_State(Servo_ID servo);
uint8_t Is_Servo_Moving(Servo_ID servo);
uint8_t Is_Servo_Stalled(Servo_ID servo);

// 平滑移动函数
uint8_t Move_Servo_Smoothly(Servo_ID servo, uint16_t target_angle, uint8_t steps, uint8_t speed);
void Move_Servo_Linear(Servo_ID servo, uint16_t target_angle, uint32_t duration_ms);

// 校准和配置函数
void Calibrate_Servo(Servo_ID servo, uint16_t min_pulse_us, uint16_t max_pulse_us);
void Set_Servo_Zero_Offset(Servo_ID servo, int16_t offset);
void Set_Servo_Invert(Servo_ID servo, uint8_t invert);

// 堵转处理函数
void Servo_Check_Stall_From_Queue(void);  // 从INA226队列检查堵转
void Servo_Clear_Stall(Servo_ID servo);   // 清除堵转状态
void Servo_Emergency_Stop(void);          // 紧急停止所有舵机

// 高级功能
void Servo_Test_Sweep(Servo_ID servo);  // 舵机扫掠测试
void Set_Servo_Smoothness(Servo_ID servo, uint8_t smoothness);  // 设置平滑度
void Set_Servo_Speed_Limit(Servo_ID servo, float max_speed);    // 设置速度限制

// 系统状态检查
void Servo_Check_System_Status(void);    // 检查系统状态

// 转换函数（移除Compare_To_Angle的全局声明，改为内部函数）
uint32_t Angle_To_Compare(Servo_ID servo, uint16_t angle);

// 堵转检测参数
#define STALL_CURRENT_THRESHOLD_MA  600    // 堵转电流阈值600mA
#define STALL_DETECTION_TIME_MS     200    // 堵转检测时间窗口200ms
#define STALL_RECOVERY_DELAY_MS     500    // 堵转恢复延迟500ms

// 硬件配置
#define SERVO_TIM               TMR3
#define SERVO_TIM_CLK           CRM_TMR3_PERIPH_CLOCK
#define SERVO_PRESCALER         1200    // 实际值=1199+1
#define SERVO_PERIOD            2000    // 实际值=1999+1

// 舵机引脚定义
#define SERVO_LID_PORT          GPIOB
#define SERVO_LID_PIN           GPIO_PINS_4
#define SERVO_LID_CHANNEL       TMR_SELECT_CHANNEL_1

#define SERVO_ROD_PORT          GPIOB
#define SERVO_ROD_PIN           GPIO_PINS_5
#define SERVO_ROD_CHANNEL       TMR_SELECT_CHANNEL_2

// MG90S默认配置
#define DEFAULT_MIN_PULSE_US    500     // 0.5ms
#define DEFAULT_MAX_PULSE_US    2500    // 2.5ms
#define DEFAULT_MIN_ANGLE       0
#define DEFAULT_MAX_ANGLE       180

// 全局变量声明
extern Servo_Config servo_configs[SERVO_NUM];
extern Servo_Control servo_controls[SERVO_NUM];
extern uint8_t servo_system_initialized;

#endif /* SERVO_HANDLE_H */