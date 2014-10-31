/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */
/*
 *  Created on: Oct 26, 2014
 *  Author: Ahmed Talaat (aa_talaat@yahoo.com)
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "stm32f10x_it.h"
#include "TIM4.h"
#include "buttons.h"
#include "queues.h"

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t keypadColIndex;
uint16_t keypadPin;
msgQueueDef msgContent;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles RTC global interrupt request.
  * @param  None
  * @retval None
  */

/*
 * Handle the interrupt generated when a user button is pressed/released. It will find out which set of column keys were pressed on the
 * keypad. Will read the current GPIO PIN value, and check the previous button status to take a decision if this is a valid
 * sequence or not. If valid, then it will trigger the debounce timer otherwise, it will
 */
void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line12) != RESET) {
		keypadColIndex=0;
		keypadPin = KEYPAD_COL1;
	} else

	if(EXTI_GetITStatus(EXTI_Line13) != RESET) {
		keypadColIndex=1;
		keypadPin = KEYPAD_COL2;
	} else

	if(EXTI_GetITStatus(EXTI_Line14) != RESET)  {
		keypadColIndex=2;
		keypadPin = KEYPAD_COL3;
	} else

	if(EXTI_GetITStatus(EXTI_Line15) != RESET)  {
		keypadColIndex=3;
		keypadPin = KEYPAD_COL4;
	}

	DisableKeypadExti_IRQ();							// Disable interrupt to avoid any debounce effects

	if (GPIO_ReadInputDataBit(KEYPAD_PORT, keypadPin)) {
		if (keypadColState[keypadColIndex] == BT_DOWN) {// We read a high bit, so if it was low before then valid
			keypadColState[keypadColIndex] = BT_GOING_UP;
			enableDebounceTimer();						// Trigger a debounce delay, and read the key later in the timer ISR
		} else {										// Invalid transition, enable interrupt back and return
			EnableKeypadExti_IRQ();
			keypadColState[keypadColIndex] = BT_IDLE;	// Reset button state to default
		}
	} else {											// We read a low bit, so see which valid transition we can handle
		if (keypadColState[keypadColIndex] == BT_IDLE) {// button just pressed down from idle state
			keypadColState[keypadColIndex] = BT_GOING_DOWN;
			enableDebounceTimer();
		} else {										// Invalid transition, enable interrupt back and return
			EnableKeypadExti_IRQ();
			keypadColState[keypadColIndex] = BT_IDLE;	// Reset button state to default
		}
	}
}


/**
  * @brief  This function handles TIM4 global interrupt request.
  * @param  None
  * @retval None
  */

/*
 * triggered after 20 ms from enabling the timer in the exti ISR.
 * It will disable the timer and clear its interrupt flags
 * It will read the tested button GPIO and update its status accordingly. If a valid state, will post a message
 * to the main program loop accordingly
 */
void TIM4_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  {

	  disableDebounceTimer();
	  if (GPIO_ReadInputDataBit(KEYPAD_PORT, keypadPin)) {
		  	  	  	  	  	  	  	  	  	  	  	  	// We read a high bit
	  		if (keypadColState[keypadColIndex] == BT_GOING_UP) {
	  			keypadColState[keypadColIndex] = BT_UP;		// Update state to up
	  		    msgContent.msgID = MSG_BT_UP;
	  		    msgContent.msgContent = keypadColIndex;		// Let the main loop knows which key was pressed up
	  			putItemInQueue(&IsrToMainQueue, &msgContent);	// post a message to the main loop that a valid button up was detected
	  		} else {									// Invalid transition
	  			keypadColState[keypadColIndex] = BT_IDLE;		// Reset button state to default
	  		}
	  } else {											// We read a low bit, so see which valid transition we can handle
	  		if (keypadColState[keypadColIndex] == BT_GOING_DOWN) {
	  			keypadColState[keypadColIndex] = BT_DOWN;
	  			msgContent.msgID = MSG_BT_DOWN;
	  													// Get the ascii code of the button pressed based on column index
	  													// and extra scanning of rows
	  		    msgContent.msgContent = getKeyPressed(keypadColIndex);
	  		    										// post a message to the main loop that a valid button down was detected
	  			putItemInQueue(&IsrToMainQueue, &msgContent);

	  			Config_Keypad(ROW_OUT_COL_IN);			// Configure column pins as input that generate interrupts and
	  													// row as output

	  		} else {									// Invalid transition
	  													// Reset button state to default
	  			keypadColState[keypadColIndex] = BT_IDLE;
	  		}
	  	}
	  EnableKeypadExti_IRQ();							// Enable interrupt again to parse a new key
  }
}

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 


/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
