/* add user code begin Header */
/**
 **************************************************************************
 * @file     main.c
 * @brief    main program
 **************************************************************************
 *                       Copyright notice & Disclaimer
 *
 * The software Board Support Package (BSP) that is made available to
 * download from Artery official website is the copyrighted work of Artery.
 * Artery authorizes customers to use, copy, and distribute the BSP
 * software and its related documentation for the purpose of design and
 * development in conjunction with Artery microcontrollers. Use of the
 * software is governed by this copyright notice and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
 * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
 * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
 * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
 * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
 *
 **************************************************************************
 */
/* add user code end Header */

/* Includes ------------------------------------------------------------------*/
#include "at32f421_wk_config.h"
#include "wk_adc.h"
#include "wk_tmr.h"
#include "wk_usart.h"
#include "wk_dma.h"
#include "wk_gpio.h"
#include "wk_system.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "iap.h"
#include "tmt.h"

#include "ina226_handle.h"
#include "perf_counter.h"
/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */

/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */
#define IAP_TASK_RUN_TIME 10
#define HEARTBEAT_TIME 10
#define SERVO_TIME 5
/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */

/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */
void iap_task(void);
void led_task(void);
void ina226_task(void);
void send_heartbeat_task(void);
void servo_task(void);

void uart_printf(const char *format, ...);
void Servo_SetAngle(uint8_t TIM, float Angle);
/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */

/* add user code end 0 */

/**
 * @brief main function.
 * @param  none
 * @retval none
 */
int main(void)
{
  /* add user code begin 1 */
  nvic_vector_table_set(NVIC_VECTTAB_FLASH, 0x4000);
  /* add user code end 1 */

  /* system clock config. */
  wk_system_clock_config();

  /* config periph clock. */
  wk_periph_clock_config();

  /* nvic config. */
  wk_nvic_config();

  /* timebase config. */
  wk_timebase_init();

  /* init gpio function. */
  wk_gpio_config();

  /* init adc1 function. */
  wk_adc1_init();

  /* init dma1 channel1 */
  wk_dma1_channel1_init();
  /* config dma channel transfer parameter */
  /* user need to modify define values DMAx_CHANNELy_XXX_BASE_ADDR and DMAx_CHANNELy_BUFFER_SIZE in at32xxx_wk_config.h */
  wk_dma_channel_config(DMA1_CHANNEL1,
                        DMA1_CHANNEL1_PERIPHERAL_BASE_ADDR,
                        DMA1_CHANNEL1_MEMORY_BASE_ADDR,
                        DMA1_CHANNEL1_BUFFER_SIZE);
  dma_channel_enable(DMA1_CHANNEL1, FALSE);

  /* init usart1 function. */
  wk_usart1_init();

  /* init tmr3 function. */
  wk_tmr3_init();

  /* add user code begin 2 */
  init_cycle_counter(true);
  tmt_init();
  iap_init();
  tmt.create(iap_task, IAP_TASK_RUN_TIME);
  tmt.create(ina226_task, INA226_HANDLE_TIME);
  tmt.create(send_heartbeat_task, HEARTBEAT_TIME);
  // tmt.create(servo_task, SERVO_TIME);

  i2c_config();
  wk_delay_ms(100);
  INA226_Init(&ina226);

  /* wait for system reday */

  /* add user code end 2 */

  while (1)
  {
    /* add user code begin 3 */
    tmt.run();
    /* add user code end 3 */
  }
}

/* add user code begin 4 */

/**
 * @brief  iap task function
 * @param  none
 * @retval none
 */
void iap_task(void)
{
  iap_command_handle();
}
static bool error_flag = false;
void ina226_task(void)
{

  static uint16_t error_time = 0;
  ina226_handle();

  if (ina226.Current >= 600)
  {
    error_flag = true;
  }
  if (error_flag)
  {
    error_time++;
    if (error_time >= 200) // 200*50 = 2s
    {
      error_time = 0;
      error_flag = false;
    }
  }
}

void send_heartbeat_task(void)
{
  uart_printf("电压%f\r\n", ina226.BusVoltage);
  uart_printf("电流 %f\r\n", ina226.Current);
}

void servo_task(void)
{
  static uint8_t count = 0;
  static bool count_flag = false;

  if (count_flag == false)
  {
    count++;
    if (count > 180)
    {
      count_flag = true;
    }
  }
  else
  {
    count--;
    if (count == 0)
    {
      count_flag = false;
    }
  }
  if (error_flag == false)
  {
    Servo_SetAngle(1, count);
    Servo_SetAngle(2, count);
  }
  else
  {
    tmr_channel_enable(TMR3, TMR_SELECT_CHANNEL_1, FALSE);
    tmr_channel_enable(TMR3, TMR_SELECT_CHANNEL_2, FALSE);
  }
}

/**
 * @brief  servo task function
 * @param  none
 * @retval none
 */
void Servo_SetAngle(uint8_t TIM, float Angle)
{
  if (TIM == 1)
  {
    if (Angle >= 180)
    {
      Angle = 180;
    }

    if (Angle <= 0)
    {
      Angle = 0;
    }

    Angle = (Angle / 180 * 2000 + 500) / 10;
    tmr_channel_enable(TMR3, TMR_SELECT_CHANNEL_1, TRUE);
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_1, Angle);
  }
  else
  {
    if (Angle >= 180)
    {
      Angle = 180;
    }

    if (Angle <= 0)
    {
      Angle = 0;
    }

    Angle = (Angle / 180 * 2000 + 500) / 10;
    tmr_channel_enable(TMR3, TMR_SELECT_CHANNEL_2, TRUE);
    tmr_channel_value_set(TMR3, TMR_SELECT_CHANNEL_2, Angle);
  }
}

// 将浮点数转换为字符串
static void float_to_str(char *buffer, float num, int precision)
{
  int integer_part = (int)num;
  float decimal_part = num - integer_part;

  // 处理负数
  if (integer_part < 0)
  {
    decimal_part = -decimal_part;
  }

  // 转换整数部分
  int i = 0;
  int temp = integer_part;

  if (temp == 0)
  {
    buffer[i++] = '0';
  }
  else
  {
    if (temp < 0)
    {
      buffer[i++] = '-';
      temp = -temp;
    }

    char temp_buf[12];
    int j = 0;
    while (temp > 0)
    {
      temp_buf[j++] = (temp % 10) + '0';
      temp /= 10;
    }

    while (j > 0)
    {
      buffer[i++] = temp_buf[--j];
    }
  }

  // 添加小数点
  if (precision > 0)
  {
    buffer[i++] = '.';

    // 转换小数部分
    for (int p = 0; p < precision; p++)
    {
      decimal_part *= 10;
      int digit = (int)decimal_part;
      buffer[i++] = digit + '0';
      decimal_part -= digit;
    }
  }

  buffer[i] = '\0';
}

// 增强版串口打印函数，支持浮点数
void uart_printf(const char *format, ...)
{
  va_list args;
  va_start(args, format);

  char buffer[128];
  int index = 0;
  int precision = 6; // 默认精度

  while (*format && index < sizeof(buffer) - 1)
  {
    // 检查精度设置
    if (*format == '%' && *(format + 1) == '.' &&
        (*(format + 2) >= '0' && *(format + 2) <= '9'))
    {
      format += 2; // 跳过%.
      precision = 0;

      // 解析精度数字
      while (*format >= '0' && *format <= '9')
      {
        precision = precision * 10 + (*format - '0');
        format++;
      }

      if (*format == 'f')
      {
        float num = (float)va_arg(args, double);
        char float_buf[32];
        float_to_str(float_buf, num, precision);

        // 复制到缓冲区
        char *p = float_buf;
        while (*p && index < sizeof(buffer) - 1)
        {
          buffer[index++] = *p++;
        }

        format++;
        precision = 6; // 重置默认精度
        continue;
      }
    }
    else if (*format == '%')
    {
      format++;
      switch (*format)
      {
      case 'd': // 整数
      {
        int num = va_arg(args, int);
        char temp[12];
        int i = 0;

        if (num < 0)
        {
          buffer[index++] = '-';
          num = -num;
        }

        if (num == 0)
        {
          temp[i++] = '0';
        }

        while (num > 0 && i < 11)
        {
          temp[i++] = (num % 10) + '0';
          num /= 10;
        }

        while (i > 0)
        {
          buffer[index++] = temp[--i];
        }
        break;
      }
      case 'f': // 浮点数（默认精度6位）
      {
        float num = (float)va_arg(args, double);
        char float_buf[32];
        float_to_str(float_buf, num, precision);

        // 复制到缓冲区
        char *p = float_buf;
        while (*p && index < sizeof(buffer) - 1)
        {
          buffer[index++] = *p++;
        }
        break;
      }
      case 's': // 字符串
      {
        char *str = va_arg(args, char *);
        while (*str && index < sizeof(buffer) - 1)
        {
          buffer[index++] = *str++;
        }
        break;
      }
      case 'c': // 字符
        buffer[index++] = (char)va_arg(args, int);
        break;
      case 'x': // 十六进制小写
      {
        unsigned int num = va_arg(args, unsigned int);
        buffer[index++] = '0';
        buffer[index++] = 'x';

        char hex_digits[] = "0123456789abcdef";
        char temp[9];
        int i = 0;

        do
        {
          temp[i++] = hex_digits[num & 0xF];
          num >>= 4;
        } while (num > 0 && i < 8);

        // 反转
        while (i > 0)
        {
          buffer[index++] = temp[--i];
        }
        break;
      }
      case 'X': // 十六进制大写
      {
        unsigned int num = va_arg(args, unsigned int);
        buffer[index++] = '0';
        buffer[index++] = 'x';

        char hex_digits[] = "0123456789ABCDEF";
        char temp[9];
        int i = 0;

        do
        {
          temp[i++] = hex_digits[num & 0xF];
          num >>= 4;
        } while (num > 0 && i < 8);

        // 反转
        while (i > 0)
        {
          buffer[index++] = temp[--i];
        }
        break;
      }
      default:
        buffer[index++] = *format;
        break;
      }
    }
    else
    {
      buffer[index++] = *format;
    }

    format++;
  }

  // 发送数据
  for (int i = 0; i < index; i++)
  {
    usart_data_transmit(USART1, buffer[i]);
    while (usart_flag_get(USART1, USART_TDC_FLAG) == RESET)
      ;
  }

  va_end(args);
}
/* add user code end 4 */
