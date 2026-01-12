/**
  **************************************************************************
  * @file     soft i2c.h
  * @version  v2.0.0
  * @date     2020-11-02
  * @brief    i2c application libray header file
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

/*!< define to prevent recursive inclusion -------------------------------------*/
#ifndef __SOFT_I2C_H
#define __SOFT_I2C_H

/* includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "at32f421.h"


  
#define I2Cx_SCL_PIN                     GPIO_PINS_6
#define I2Cx_SCL_GPIO_PORT               GPIOF
#define I2Cx_SCL_GPIO_CLK                CRM_GPIOF_PERIPH_CLOCK

#define I2Cx_SDA_PIN                     GPIO_PINS_7
#define I2Cx_SDA_GPIO_PORT               GPIOF
#define I2Cx_SDA_GPIO_CLK                CRM_GPIOF_PERIPH_CLOCK

#define I2C_DIR_TRANSMIT                 0x00
#define I2C_DIR_RECEIVE                  0x01

#define I2C_ACK_TIMEOUT                  50

#define I2C_SCL_HIGH()                   gpio_bits_set(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN)
#define I2C_SCL_LOW()                    gpio_bits_reset(I2Cx_SCL_GPIO_PORT, I2Cx_SCL_PIN)

#define I2C_SDA_HIGH()                   gpio_bits_set(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN)
#define I2C_SDA_LOW()                    gpio_bits_reset(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN)

#define I2C_SDA_READ()                   (gpio_input_data_bit_read(I2Cx_SDA_GPIO_PORT, I2Cx_SDA_PIN) == SET)



void i2c_config(void);
void i2c_start(void);
void i2c_stop(void);
void i2c_send_byte(uint8_t data);
void i2c_check(void);
void i2c_ack(void);
void i2c_no_ack(void);
void i2c_check_addr(void);
uint8_t i2c_receive_byte(void);

uint8_t i2c_wait_ack(uint8_t timeout);
uint8_t i2c_master_transmit (uint16_t address, uint8_t* pdata, uint16_t size);
uint8_t i2c_master_receive  (uint16_t address, uint8_t* pdata, uint16_t size);
         
uint8_t i2c_memory_write    (uint16_t address, uint16_t mem_address, uint8_t* pdata, uint16_t size);
uint8_t i2c_memory_read     (uint16_t address, uint16_t mem_address, uint8_t* pdata, uint16_t size);

#endif
