/*
 * main.c
 *
 *  Created on: Oct 26, 2014
 *  Author: Ahmed Talaat (aa_talaat@yahoo.com)
 */
/*
 * This program shows how to read a 4x4 membrane keypad.
 * The keypad provides 8 pins of which the first 4 pins are connected to the 4 columns and the other 4 pins are connected to the 4 rows.
 * The 8 keypad PINs are connected as follows to the Discovery VL Board
 *
 * KEYPAD			Discovery VL Board
 * ======			==================
 * 	1 (row 1)		PB8
 * 	2 (row 2)		PB9
 * 	3 (row 3)		PB10
 * 	4 (row 4)		PB11
 * 	5 (col 1)		PB12
 * 	6 (col 2)		PB13
 * 	7 (col 3)		PB14
 * 	8 (col 4)		PB15
 *
 *	Keypad Pins 1-4 are the rows and Pins 5-8 are the columns.
 *	PB12-15 will be configured when needed as external interrupt source.
 *	The discovery board pins were selected this way as their alternate function are least used.
 *
 *	The logic of the program is as follows:
 *	- Initialize GPIOs, NVIC, TIM,..
 *	- Initially rows are configured as output pins and set to low.
 *	- Column pins are configured as input pull-up and generate interrupt on falling/rising edge
 *	- Go into a main loop. The main loop retrieves messages from a queue that is being filled by the ISRs
 *	- Upon a keypad key is pressed, one of the four columns pin will generate an interrupt.
 *
 EXIT ISR:
	- Store the exact pin that caused the interrupt.
	- Mask interrupt, and clear pending interrupt flags.
	- Read this Pin. 
	- If Pin is Low and current button state is BT_IDLE, then this is a correct transition.
	- Set the button state as BT_GOING_DOWN.
	- If Pin is High and current button state is BT_DOWN, then this is a correct transition.
	- Set the button state to GOING_UP.
	- Otherwise, this is an invalid case, enable interrupts are return.
	- For valid transition, store the pin that caused the interrupt.
	- Trigger the debounce timer to generate an interrupt in 10 ms.
	- Return back.
	
TIMx ISR:
	- This ISR is invoked when a debouncing time has expired.
	- Read the GPIO that caused the interrupt earlier
	- If the current button state is BT_GOING_DOWN, and the GPIO is low:
		- Change the button state to BT_DOWN.
		- Perform row scan to find out the row index corresponding to the button pressed.
		- Based on the col and row index, find out the ascii code of the button pressed.
		- Generate a msg BT_DOWN with value the ascii code of the pressed button to be processing in the main loop.
		- Reconfigure the GPIOs for detecting a button up through the ISR (Row out, Col in) and later on for a new keypad parsing.
		
	- If the current button state is BT_GOING_UP, and the GPIO is high:
		- Change the button state to BT_UP.
		- Post a msg BT_UP to the main loop with no value. 
	
	- Otherwise, this is an invalid case, change the button state to BT_IDLE
	- Enable the buttons interrupt again.
	- Return
	
Main Loop:
	- Upon receiving a button down message, do whatever was planned to do 
	- Upon receiving a button up message, the msg content has the key index. Change the button state to idle.
	
Unhandled cases:
	- What will happen in the user presses one key down, and while down, he presses a second key down, then release both in any order?
	
 *
 */

/* Includes */
#include "stm32f10x.h"
#include "gpio.h"
#include "TIM4.h"
#include "queues.h"
#include "buttons.h"

/* Private functions */
void HSI_RCC_Configuration(void);
void Config_NVIC(void);

int main(void) {

	uint8_t rc, i=0;
	uint8_t test[10];

	msgQueueDef readValue;

	initializeQueue(&IsrToMainQueue);

	HSI_RCC_Configuration();			// Configure system clock to HSI @ 8MHz

	// Configure two bits for preemption and two bits for priority
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	Config_NVIC();

	TIM4_Configuration ();				// Configure the debounce timer

	GPIO_SetAllAnalogInput();			// change all IOs into Analog INP to save power


	Config_Keypad(ROW_OUT_COL_IN);		// initially configure colum pins as input that generate interrupts and row as output


	while (1)  {						// Infinite loop

    	rc = getItemFromQueue(&IsrToMainQueue,&readValue);

    	if (rc != 0xff) {	//queue is not empty
			switch (readValue.msgID) {
				case MSG_BT_DOWN:						// A button down was detected, and the msg content holds the
														// ascii code of the key pressed
					test[i] = readValue.msgContent;
					if (i == 9) {
						i=0;							// Put a break point here and watch the test array for the last
					} else {							// 10 keys typed by the user
						i++;
					}
					break;

				case MSG_BT_UP:							// Rest specific button status to idle
					keypadColState[readValue.msgContent] = BT_IDLE;
					break;

				default:								// Anything else put CPU to sleep
					__WFI();
					break;
			}
		}
	}
}

/*
 * Config NVIC
 */
void Config_NVIC(void){
	NVIC_InitTypeDef   	NVIC_InitStructure;

	/* Enable and set EXTI10_15 Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/*
 * Configure the STM32 to use the internal high speed clock (HSI) at 8MHz
 */
void HSI_RCC_Configuration(void)
{

  /* Enable HSI Clock */
  RCC_HSICmd(ENABLE);		//1680-1543ua

  /*!< Wait till HSI is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET) {}

  RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);

  RCC_HSEConfig(RCC_HSE_OFF);

  if(RCC_GetFlagStatus(RCC_FLAG_HSERDY) != RESET )
  {
    while(1);
  }

  /* Enable PWR mngt */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
}
