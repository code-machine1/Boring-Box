/* add user code begin Header */
/**
  **************************************************************************
  * @file     at32f421_int.c
  * @brief    main interrupt service routines.
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

/* includes ------------------------------------------------------------------*/
#include "at32f421_int.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */
#include "wk_tmr.h"
#include "wk_usart.h"
#include "tmt.h"
#include "iap.h"
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

/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */

/* add user code end 0 */

/* external variables ---------------------------------------------------------*/
/* add user code begin external variables */

/* add user code end external variables */

/**
  * @brief  this function handles nmi exception.
  * @param  none
  * @retval none
  */
void NMI_Handler(void)
{
  /* add user code begin NonMaskableInt_IRQ 0 */
    /* add user code end NonMaskableInt_IRQ 0 */

  /* add user code begin NonMaskableInt_IRQ 1 */
    /* add user code end NonMaskableInt_IRQ 1 */
}

/**
  * @brief  this function handles hard fault exception.
  * @param  none
  * @retval none
  */
void HardFault_Handler(void)
{
  /* add user code begin HardFault_IRQ 0 */

    /* add user code end HardFault_IRQ 0 */
  /* go to infinite loop when hard fault exception occurs */
  while (1)
  {
    /* add user code begin W1_HardFault_IRQ 0 */
        /* add user code end W1_HardFault_IRQ 0 */
  }
}

/**
  * @brief  this function handles memory manage exception.
  * @param  none
  * @retval none
  */
void MemManage_Handler(void)
{
  /* add user code begin MemoryManagement_IRQ 0 */

    /* add user code end MemoryManagement_IRQ 0 */
  /* go to infinite loop when memory manage exception occurs */
  while (1)
  {
    /* add user code begin W1_MemoryManagement_IRQ 0 */
        /* add user code end W1_MemoryManagement_IRQ 0 */
  }
}

/**
  * @brief  this function handles bus fault exception.
  * @param  none
  * @retval none
  */
void BusFault_Handler(void)
{
  /* add user code begin BusFault_IRQ 0 */

    /* add user code end BusFault_IRQ 0 */
  /* go to infinite loop when bus fault exception occurs */
  while (1)
  {
    /* add user code begin W1_BusFault_IRQ 0 */
        /* add user code end W1_BusFault_IRQ 0 */
  }
}

/**
  * @brief  this function handles usage fault exception.
  * @param  none
  * @retval none
  */
void UsageFault_Handler(void)
{
  /* add user code begin UsageFault_IRQ 0 */

    /* add user code end UsageFault_IRQ 0 */
  /* go to infinite loop when usage fault exception occurs */
  while (1)
  {
    /* add user code begin W1_UsageFault_IRQ 0 */
        /* add user code end W1_UsageFault_IRQ 0 */
  }
}

/**
  * @brief  this function handles svcall exception.
  * @param  none
  * @retval none
  */
void SVC_Handler(void)
{
  /* add user code begin SVCall_IRQ 0 */
    /* add user code end SVCall_IRQ 0 */
  /* add user code begin SVCall_IRQ 1 */
    /* add user code end SVCall_IRQ 1 */
}

/**
  * @brief  this function handles debug monitor exception.
  * @param  none
  * @retval none
  */
void DebugMon_Handler(void)
{
  /* add user code begin DebugMonitor_IRQ 0 */
    /* add user code end DebugMonitor_IRQ 0 */
  /* add user code begin DebugMonitor_IRQ 1 */
    /* add user code end DebugMonitor_IRQ 1 */
}

/**
  * @brief  this function handles pendsv_handler exception.
  * @param  none
  * @retval none
  */
void PendSV_Handler(void)
{
  /* add user code begin PendSV_IRQ 0 */
    /* add user code end PendSV_IRQ 0 */
  /* add user code begin PendSV_IRQ 1 */
    /* add user code end PendSV_IRQ 1 */
}

/**
  * @brief  this function handles systick handler.
  * @param  none
  * @retval none
  */
void SysTick_Handler(void)
{
  /* add user code begin SysTick_IRQ 0 */
    /* add user code end SysTick_IRQ 0 */


  /* add user code begin SysTick_IRQ 1 */
    /* add user code end SysTick_IRQ 1 */
}

/**
  * @brief  this function handles TMR3 handler.
  * @param  none
  * @retval none
  */
void TMR3_GLOBAL_IRQHandler(void)
{
  /* add user code begin TMR3_GLOBAL_IRQ 0 */
    if (tmr_interrupt_flag_get(TMR3, TMR_OVF_FLAG) == SET)
    {
        tmr_flag_clear(TMR3, TMR_OVF_FLAG);

        if (get_data_from_usart_flag)
        {
            if ((++time_ira_cnt) == 0x00)
            {
                time_ira_cnt = 0xFF;
            }

            if (time_ira_cnt > 2)
            {
                back_err();
            }

            if (time_ira_cnt > 5)
                ;
        }
    }

    /* add user code end TMR3_GLOBAL_IRQ 0 */


  /* add user code begin TMR3_GLOBAL_IRQ 1 */
    /* add user code end TMR3_GLOBAL_IRQ 1 */
}

/**
  * @brief  this function handles USART1 handler.
  * @param  none
  * @retval none
  */
void USART1_IRQHandler(void)
{
  /* add user code begin USART1_IRQ 0 */
    uint16_t reval;
    time_ira_cnt = 0;  /* clear upgrade time out flag */

    if (usart_interrupt_flag_get(USART1, USART_RDBF_FLAG) != RESET)
    {
        reval = usart_data_receive(USART1);

        if (usart_group_struct.count > (USART_REC_LEN - 1))
        {
            usart_group_struct.count = 0;
            usart_group_struct.head = 0;
            usart_group_struct.tail = 0;
        }
        else
        {
            usart_group_struct.count++;
            usart_group_struct.buf[usart_group_struct.head++] = reval;

            if (usart_group_struct.head > (USART_REC_LEN - 1))
            {
                usart_group_struct.head = 0;
            }
        }
    }

    /* add user code end USART1_IRQ 0 */
  /* add user code begin USART1_IRQ 1 */
    /* add user code end USART1_IRQ 1 */
}

/* add user code begin 1 */

/* add user code end 1 */
