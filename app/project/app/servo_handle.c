#include "servo_handle.h"
#include "freertos_app.h"  // 包含ina226_value_handle队列定义
#include <stdlib.h>
#include <string.h>

// 全局变量定义
Servo_Config servo_configs[SERVO_NUM];
Servo_Control servo_controls[SERVO_NUM];
uint8_t servo_system_initialized = 0;

// 内部函数声明
static uint16_t Compare_To_Angle(Servo_ID servo, uint32_t compare_value);
static void Servo_Process_Current_Data(float current_ma);
static void Servo_Handle_Stall_Detection(Servo_ID servo, float current_ma, uint32_t current_time);

// 系统初始化
void Servo_System_Init(void) {
    if (servo_system_initialized) {
        return;
    }
    
    // 初始化舵机配置 (适配MG90S)
    for (int i = 0; i < SERVO_NUM; i++) {
        // MG90S默认配置
        servo_configs[i].min_angle = DEFAULT_MIN_ANGLE;
        servo_configs[i].max_angle = DEFAULT_MAX_ANGLE;
        servo_configs[i].min_pulse_us = DEFAULT_MIN_PULSE_US;
        servo_configs[i].max_pulse_us = DEFAULT_MAX_PULSE_US;
        servo_configs[i].zero_offset = 0;
        servo_configs[i].invert = 0;
        servo_configs[i].smooth_factor = 50;
        
        // 初始化控制结构
        servo_controls[i].state = SERVO_IDLE;
        servo_controls[i].current_angle = 90;  // 默认中间位置
        servo_controls[i].target_angle = 90;
        servo_controls[i].current_compare = Angle_To_Compare(i, 90);
        servo_controls[i].last_update_time = 0;
        servo_controls[i].is_initialized = 0;
        servo_controls[i].speed_factor = 1.0;
        
        // 堵转检测初始化
        servo_controls[i].stall_detected = 0;
        servo_controls[i].stall_start_time = 0;
        servo_controls[i].stall_count = 0;
    }
    
    // 初始化INA226电流检测
    INA226_Init(&ina226);
    
    // 初始化各个舵机
    Servo_Init(SERVO_LID);
    Servo_Init(SERVO_ROD);
    
    servo_system_initialized = 1;
    
    // 设置初始安全位置
    Set_Servo_Angle(SERVO_LID, 0);    // 盖子关闭
    Set_Servo_Angle(SERVO_ROD, 0);    // 推杆缩回
}

// 单个舵机初始化
void Servo_Init(Servo_ID servo) {
    if (servo >= SERVO_NUM) {
        return;
    }
    
    // 确保定时器3已经初始化
    // 设置初始位置
    uint32_t init_compare = Angle_To_Compare(servo, servo_controls[servo].current_angle);
    Set_Servo_Compare_Value(servo, init_compare);
    
    servo_controls[servo].is_initialized = 1;
    servo_controls[servo].state = SERVO_IDLE;
}

// 角度转比较值
uint32_t Angle_To_Compare(Servo_ID servo, uint16_t angle) {
    Servo_Config *config = &servo_configs[servo];
    
    // 限制角度范围
    if (angle < config->min_angle) angle = config->min_angle;
    if (angle > config->max_angle) angle = config->max_angle;
    
    // 应用零位偏移
    int32_t adjusted_angle = (int32_t)angle + config->zero_offset;
    
    // 限制调整后的角度
    if (adjusted_angle < config->min_angle) adjusted_angle = config->min_angle;
    if (adjusted_angle > config->max_angle) adjusted_angle = config->max_angle;
    
    // 计算角度百分比
    float angle_percent;
    if (config->max_angle == config->min_angle) {
        angle_percent = 0.5f;
    } else {
        angle_percent = (float)(adjusted_angle - config->min_angle) / 
                       (float)(config->max_angle - config->min_angle);
    }
    
    // 线性映射角度到脉宽(微秒)
    uint32_t pulse_us = config->min_pulse_us + 
                       (uint32_t)(angle_percent * (config->max_pulse_us - config->min_pulse_us));
    
    // 应用方向反转
    if (config->invert) {
        pulse_us = config->max_pulse_us + config->min_pulse_us - pulse_us;
    }
    
    // 将微秒转换为比较值 (每个计数10us)
    uint32_t compare_value = pulse_us / 10;
    
    // 限制比较值范围
    if (compare_value > SERVO_PERIOD - 1) {
        compare_value = SERVO_PERIOD - 1;
    }
    
    return compare_value;
}

// 比较值转角度（内部函数）
static uint16_t Compare_To_Angle(Servo_ID servo, uint32_t compare_value) {
    Servo_Config *config = &servo_configs[servo];
    
    // 限制比较值范围
    if (compare_value > SERVO_PERIOD - 1) {
        compare_value = SERVO_PERIOD - 1;
    }
    
    // 将比较值转换为微秒
    uint32_t pulse_us = compare_value * 10;
    
    // 如果有反转，先反转换算
    uint32_t normalized_pulse_us = pulse_us;
    if (config->invert) {
        normalized_pulse_us = config->max_pulse_us + config->min_pulse_us - pulse_us;
    }
    
    // 限制脉宽范围
    if (normalized_pulse_us < config->min_pulse_us) normalized_pulse_us = config->min_pulse_us;
    if (normalized_pulse_us > config->max_pulse_us) normalized_pulse_us = config->max_pulse_us;
    
    // 计算脉宽百分比
    float pulse_percent;
    if (config->max_pulse_us == config->min_pulse_us) {
        pulse_percent = 0.5f;
    } else {
        pulse_percent = (float)(normalized_pulse_us - config->min_pulse_us) / 
                       (float)(config->max_pulse_us - config->min_pulse_us);
    }
    
    // 线性映射脉宽到角度
    uint16_t angle = config->min_angle + 
                    (uint16_t)(pulse_percent * (config->max_angle - config->min_angle));
    
    // 减去零位偏移
    int32_t final_angle = (int32_t)angle - config->zero_offset;
    
    // 限制最终角度
    if (final_angle < config->min_angle) final_angle = config->min_angle;
    if (final_angle > config->max_angle) final_angle = config->max_angle;
    
    return (uint16_t)final_angle;
}

// 设置舵机角度(立即) - 添加堵转检查
uint8_t Set_Servo_Angle(Servo_ID servo, uint16_t angle) {
    if (servo >= SERVO_NUM || !servo_controls[servo].is_initialized) {
        return 0;
    }
    
    // 检查是否已经堵转
    if (servo_controls[servo].stall_detected) {
        printf("Servo %d is stalled! Cannot move.\r\n", servo);
        return 0;
    }
    
    // 转换为比较值
    uint32_t compare_value = Angle_To_Compare(servo, angle);
    
    // 更新状态
    servo_controls[servo].state = SERVO_MOVING;
    servo_controls[servo].target_angle = angle;
    
    // 设置PWM比较值
    Set_Servo_Compare_Value(servo, compare_value);
    
    // 更新当前角度和比较值
    servo_controls[servo].current_angle = angle;
    servo_controls[servo].current_compare = compare_value;
    servo_controls[servo].last_update_time = xTaskGetTickCount();
    
    // 短暂延时，期间检查堵转
    uint32_t start_time = xTaskGetTickCount();
    while ((xTaskGetTickCount() - start_time) < pdMS_TO_TICKS(20)) {
        // 每5ms检查一次堵转
        vTaskDelay(pdMS_TO_TICKS(5));
        
        // 检查INA226队列中的电流数据
        Servo_Check_Stall_From_Queue();
        
        // 如果检测到堵转，立即停止
        if (servo_controls[servo].stall_detected) {
            Stop_Servo(servo);
            printf("Servo %d movement interrupted by stall!\r\n", servo);
            return 0;
        }
    }
    
    servo_controls[servo].state = SERVO_IDLE;
    return 1;  // 成功完成
}

// 设置舵机角度(带速度控制)
uint8_t Set_Servo_Angle_Speed(Servo_ID servo, uint16_t angle, float speed) {
    if (servo >= SERVO_NUM || !servo_controls[servo].is_initialized) {
        return 0;
    }
    
    // 检查是否堵转
    if (servo_controls[servo].stall_detected) {
        printf("Servo %d is stalled! Cannot move.\r\n", servo);
        return 0;
    }
    
    // 限制速度范围
    if (speed < 0.01f) speed = 0.01f;
    if (speed > 1.0f) speed = 1.0f;
    
    servo_controls[servo].speed_factor = speed;
    servo_controls[servo].state = SERVO_MOVING;
    servo_controls[servo].target_angle = angle;
    
    // 计算步进
    uint16_t current_angle = servo_controls[servo].current_angle;
    int16_t angle_diff = (int16_t)angle - current_angle;
    
    uint8_t steps = (uint8_t)(abs(angle_diff) / (2.0f * speed));
    if (steps < 1) steps = 1;
    
    // 执行平滑移动
    return Move_Servo_Smoothly(servo, angle, steps, (uint8_t)(speed * 100));
}

// 平滑移动函数(带堵转检查)
uint8_t Move_Servo_Smoothly(Servo_ID servo, uint16_t target_angle, uint8_t steps, uint8_t speed) {
    if (servo >= SERVO_NUM || !servo_controls[servo].is_initialized) {
        return 0;
    }
    
    // 检查是否已经堵转
    if (servo_controls[servo].stall_detected) {
        printf("Servo %d is stalled! Cannot move.\r\n", servo);
        return 0;
    }
    
    if (steps < 1) steps = 1;
    if (speed > 100) speed = 100;
    
    uint16_t start_angle = servo_controls[servo].current_angle;
    int16_t angle_diff = (int16_t)target_angle - start_angle;
    
    // 应用速度因子
    float speed_multiplier = servo_controls[servo].speed_factor;
    uint32_t step_delay = (uint32_t)((100 - speed) * 2 * (1.0f / speed_multiplier));
    
    servo_controls[servo].state = SERVO_MOVING;
    servo_controls[servo].target_angle = target_angle;
    
    // 插值移动
    for (uint8_t i = 1; i <= steps; i++) {
        // 检查堵转
        if (servo_controls[servo].stall_detected) {
            Stop_Servo(servo);
            printf("Servo %d movement interrupted by stall at step %d!\r\n", servo, i);
            return 0;
        }
        
        // 计算当前步的角度(线性插值)
        float t = (float)i / steps;
        uint16_t current_angle = start_angle + (uint16_t)(angle_diff * t);
        
        // 设置角度
        uint32_t compare_value = Angle_To_Compare(servo, current_angle);
        Set_Servo_Compare_Value(servo, compare_value);
        
        // 更新状态
        servo_controls[servo].current_angle = current_angle;
        
        // 延时(步进间隔)
        vTaskDelay(pdMS_TO_TICKS(step_delay));
        
        // 在延时期间检查堵转
        Servo_Check_Stall_From_Queue();
    }
    
    // 确保到达目标
    if (!servo_controls[servo].stall_detected) {
        Set_Servo_Compare_Value(servo, Angle_To_Compare(servo, target_angle));
        servo_controls[servo].current_angle = target_angle;
        servo_controls[servo].state = SERVO_IDLE;
        return 1;  // 成功完成
    } else {
        Stop_Servo(servo);
        return 0;  // 被堵转中断
    }
}

// 线性移动(指定持续时间)
void Move_Servo_Linear(Servo_ID servo, uint16_t target_angle, uint32_t duration_ms) {
    if (servo >= SERVO_NUM || !servo_controls[servo].is_initialized || duration_ms == 0) {
        return;
    }
    
    uint16_t start_angle = servo_controls[servo].current_angle;
    int16_t angle_diff = (int16_t)target_angle - start_angle;
    
    // 计算需要的步数和每步时间
    uint8_t steps = (uint8_t)(abs(angle_diff) / 2.0f);
    if (steps < 1) steps = 1;
    
    uint32_t step_delay = duration_ms / steps;
    if (step_delay < 5) step_delay = 5;
    
    Move_Servo_Smoothly(servo, target_angle, steps, (uint8_t)(100 * (20.0f / step_delay)));
}

// 直接设置比较值
void Set_Servo_Compare_Value(Servo_ID servo, uint32_t compare_value) {
    if (servo >= SERVO_NUM) {
        return;
    }
    
    // 安全限制
    uint32_t min_compare = DEFAULT_MIN_PULSE_US / 10;
    uint32_t max_compare = DEFAULT_MAX_PULSE_US / 10;
    
    if (compare_value < min_compare) compare_value = min_compare;
    if (compare_value > max_compare) compare_value = max_compare;
    
    // 根据舵机ID设置对应的定时器通道
    switch (servo) {
        case SERVO_LID:
            tmr_channel_value_set(SERVO_TIM, SERVO_LID_CHANNEL, compare_value);
            break;
        case SERVO_ROD:
            tmr_channel_value_set(SERVO_TIM, SERVO_ROD_CHANNEL, compare_value);
            break;
        default:
            return;
    }
    
    // 更新控制结构
    servo_controls[servo].current_compare = compare_value;
    servo_controls[servo].current_angle = Compare_To_Angle(servo, compare_value);
    servo_controls[servo].last_update_time = xTaskGetTickCount();
}

// 停止舵机
void Stop_Servo(Servo_ID servo) {
    if (servo >= SERVO_NUM) {
        return;
    }
    
    // 将状态设为空闲
    servo_controls[servo].state = SERVO_IDLE;
}

// 禁用舵机
void Disable_Servo(Servo_ID servo) {
    if (servo >= SERVO_NUM) {
        return;
    }
    
    // 将比较值设为0（舵机可能失去保持力）
    Set_Servo_Compare_Value(servo, 0);
    servo_controls[servo].is_initialized = 0;
    servo_controls[servo].state = SERVO_IDLE;
}

// 使能舵机
void Enable_Servo(Servo_ID servo) {
    if (servo >= SERVO_NUM) {
        return;
    }
    
    // 重新设置当前位置
    uint32_t current_compare = servo_controls[servo].current_compare;
    Set_Servo_Compare_Value(servo, current_compare);
    
    servo_controls[servo].is_initialized = 1;
    servo_controls[servo].state = SERVO_IDLE;
}

// 获取当前角度
uint16_t Get_Servo_Angle(Servo_ID servo) {
    if (servo >= SERVO_NUM) {
        return 0;
    }
    
    return servo_controls[servo].current_angle;
}

// 获取当前比较值
uint32_t Get_Servo_Compare_Value(Servo_ID servo) {
    if (servo >= SERVO_NUM) {
        return 0;
    }
    
    return servo_controls[servo].current_compare;
}

// 获取舵机状态
Servo_State Get_Servo_State(Servo_ID servo) {
    if (servo >= SERVO_NUM) {
        return SERVO_ERROR;
    }
    
    return servo_controls[servo].state;
}

// 检查舵机是否在运动中
uint8_t Is_Servo_Moving(Servo_ID servo) {
    if (servo >= SERVO_NUM) {
        return 0;
    }
    
    return (servo_controls[servo].state == SERVO_MOVING);
}

// 检查舵机是否堵转
uint8_t Is_Servo_Stalled(Servo_ID servo) {
    if (servo >= SERVO_NUM) {
        return 0;
    }
    
    return (servo_controls[servo].stall_detected || servo_controls[servo].state == SERVO_STALL);
}

// 从INA226队列检查堵转
void Servo_Check_Stall_From_Queue(void) {
    ina226_t current_data;
    
    // 尝试从队列读取电流数据（非阻塞）
    if (xQueueReceive(ina226_value_handle, &current_data, 0) == pdPASS) {
        // 处理电流数据
        Servo_Process_Current_Data(current_data.Current);
    }
}

// 处理电流数据
static void Servo_Process_Current_Data(float current_ma) {
    uint32_t current_time = xTaskGetTickCount();
    
    // 检查电流是否超过阈值（600mA）
    if (current_ma >= STALL_CURRENT_THRESHOLD_MA) {
        // 遍历所有舵机，检查哪些在运动中
        for (int i = 0; i < SERVO_NUM; i++) {
            if (servo_controls[i].state == SERVO_MOVING) {
                Servo_Handle_Stall_Detection(i, current_ma, current_time);
            }
        }
    } else {
        // 电流正常，重置所有舵机的堵转检测
        for (int i = 0; i < SERVO_NUM; i++) {
            if (servo_controls[i].stall_detected && 
                servo_controls[i].state == SERVO_STALL) {
                // 堵转恢复
                printf("Servo %d recovered from stall.\r\n", i);
            }
            
            servo_controls[i].stall_detected = 0;
            servo_controls[i].stall_start_time = 0;
            servo_controls[i].stall_count = 0;
            
            // 如果是堵转状态，恢复到空闲
            if (servo_controls[i].state == SERVO_STALL) {
                servo_controls[i].state = SERVO_IDLE;
            }
        }
    }
}

// 处理单个舵机的堵转检测
static void Servo_Handle_Stall_Detection(Servo_ID servo, float current_ma, uint32_t current_time) {
    // 第一次检测到堵转
    if (!servo_controls[servo].stall_detected) {
        servo_controls[servo].stall_start_time = current_time;
        servo_controls[servo].stall_count = 1;
    } else {
        // 持续检测堵转
        servo_controls[servo].stall_count++;
        
        // 检查是否持续堵转超过阈值时间（200ms）
        uint32_t stall_duration = current_time - servo_controls[servo].stall_start_time;
        if (stall_duration >= pdMS_TO_TICKS(STALL_DETECTION_TIME_MS)) {
            // 确认堵转
            servo_controls[servo].stall_detected = 1;
            servo_controls[servo].state = SERVO_STALL;
            
            // 立即停止舵机
            Stop_Servo(servo);
            
            printf("Servo %d STALL CONFIRMED! Current: %.1fmA\r\n", servo, current_ma);
        }
    }
}

// 紧急停止所有舵机
void Servo_Emergency_Stop(void) {
    for (int i = 0; i < SERVO_NUM; i++) {
        Stop_Servo(i);
        servo_controls[i].stall_detected = 1;  // 标记为堵转
        servo_controls[i].state = SERVO_STALL;
    }
    printf("EMERGENCY STOP: All servos stopped!\r\n");
}

// 清除堵转状态
void Servo_Clear_Stall(Servo_ID servo) {
    if (servo >= SERVO_NUM) {
        return;
    }
    
    servo_controls[servo].stall_detected = 0;
    servo_controls[servo].stall_start_time = 0;
    servo_controls[servo].stall_count = 0;
    
    if (servo_controls[servo].state == SERVO_STALL) {
        servo_controls[servo].state = SERVO_IDLE;
    }
    
    printf("Servo %d stall cleared.\r\n", servo);
}

// 舵机扫掠测试（带堵转检查）
void Servo_Test_Sweep(Servo_ID servo) {
    if (servo >= SERVO_NUM || Is_Servo_Stalled(servo)) {
        return;
    }
    
    Servo_Config *config = &servo_configs[servo];
    
    printf("Testing servo %d sweep...\r\n", servo);
    
    // 从最小角度扫到最大角度
    for (uint16_t angle = config->min_angle; angle <= config->max_angle; angle += 10) {
        if (Is_Servo_Stalled(servo)) {
            printf("Test interrupted: servo %d stalled!\r\n", servo);
            break;
        }
        Set_Servo_Angle(servo, angle);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    // 从最大角度扫到最小角度
    for (uint16_t angle = config->max_angle; angle >= config->min_angle; angle -= 10) {
        if (Is_Servo_Stalled(servo)) {
            printf("Test interrupted: servo %d stalled!\r\n", servo);
            break;
        }
        Set_Servo_Angle(servo, angle);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    // 回到中间位置
    if (!Is_Servo_Stalled(servo)) {
        Set_Servo_Angle(servo, 90);
        printf("Servo %d test complete.\r\n", servo);
    }
}

// 校准舵机
void Calibrate_Servo(Servo_ID servo, uint16_t min_pulse_us, uint16_t max_pulse_us) {
    if (servo >= SERVO_NUM) {
        return;
    }
    
    // 设置新的脉宽范围
    servo_configs[servo].min_pulse_us = min_pulse_us;
    servo_configs[servo].max_pulse_us = max_pulse_us;
    
    printf("Servo %d calibrated: %d-%d us\r\n", servo, min_pulse_us, max_pulse_us);
}

// 设置零位偏移
void Set_Servo_Zero_Offset(Servo_ID servo, int16_t offset) {
    if (servo >= SERVO_NUM) {
        return;
    }
    
    servo_configs[servo].zero_offset = offset;
    printf("Servo %d zero offset set to %d\r\n", servo, offset);
}

// 设置方向反转
void Set_Servo_Invert(Servo_ID servo, uint8_t invert) {
    if (servo >= SERVO_NUM) {
        return;
    }
    
    servo_configs[servo].invert = (invert != 0) ? 1 : 0;
    printf("Servo %d invert set to %d\r\n", servo, invert);
}

// 设置平滑度
void Set_Servo_Smoothness(Servo_ID servo, uint8_t smoothness) {
    if (servo >= SERVO_NUM || smoothness > 100) {
        return;
    }
    
    servo_configs[servo].smooth_factor = smoothness;
    printf("Servo %d smoothness set to %d\r\n", servo, smoothness);
}

// 设置速度限制
void Set_Servo_Speed_Limit(Servo_ID servo, float max_speed) {
    if (servo >= SERVO_NUM) {
        return;
    }
    
    if (max_speed < 0.01f) max_speed = 0.01f;
    if (max_speed > 1.0f) max_speed = 1.0f;
    
    servo_controls[servo].speed_factor = max_speed;
    printf("Servo %d speed limit set to %.2f\r\n", servo, max_speed);
}

// 检查系统状态
void Servo_Check_System_Status(void) {
    printf("=== Servo System Status ===\r\n");
    
    for (int i = 0; i < SERVO_NUM; i++) {
        const char* servo_name = (i == SERVO_LID) ? "LID" : "ROD";
        const char* state_str;
        
        switch (servo_controls[i].state) {
            case SERVO_IDLE: state_str = "IDLE"; break;
            case SERVO_MOVING: state_str = "MOVING"; break;
            case SERVO_STALL: state_str = "STALL"; break;
            case SERVO_ERROR: state_str = "ERROR"; break;
            default: state_str = "UNKNOWN"; break;
        }
        
        printf("Servo %s: Angle=%d, State=%s, Stall=%s\r\n",
               servo_name,
               servo_controls[i].current_angle,
               state_str,
               servo_controls[i].stall_detected ? "YES" : "NO");
    }
    printf("==========================\r\n");
}