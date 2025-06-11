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
#include "wk_usart.h"
#include "wk_gpio.h"
#include "wk_system.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */
#include "sensor.h"
#include "iap.h"
#include "tmt.h"
#include "perf_counter.h"
#include "EventRecorder.h"
/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */

/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */

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
void sensor_task(void);
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

  /* init usart1 function. */
  wk_usart1_init();

  /* init gpio function. */
  wk_gpio_config();

  /* add user code begin 2 */
    tmt_init();
    tmt.create(iap_task, 10);
    tmt.create(led_task, 100);
    tmt.create(sensor_task, SENSOR_RUN_TIME);
    
	SystemCoreClock = 15000000ul;
	init_cycle_counter(true);
	EventRecorderInitialize(0, 1);
	iap_init();
	printf("debug log\r\n");
	
    /* add user code end 2 */

  while(1)
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

/**
  * @brief  led task function(only for debug)
  * @param  none
  * @retval none
  */
void led_task(void)
{
    if (gpio_output_data_bit_read(LED_GPIO_PORT, LED_PIN))
    {
        gpio_bits_reset(LED_GPIO_PORT, LED_PIN);
    }
    else
    {
        gpio_bits_set(LED_GPIO_PORT, LED_PIN);
    }
}

/**
  * @brief  sensor task function(only for debug)
  * @param  none
  * @retval none
  */
void sensor_task(void)
{
    SensorProc();
}

/* add user code end 4 */
