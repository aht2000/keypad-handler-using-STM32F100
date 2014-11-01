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
	- Trigger the debounce timer to generate an interrupt in 20 ms.
	- Return back.
	
TIMx ISR:
	- This ISR is invoked when a debouncing time has expired.
	- Disable the debounce timer.
	- Read the GPIO that caused the interrupt earlier.
	- If the GPIO is High:
		- Change the button state to BT_UP.
		- Generate a msg BT_UP with value the column index of the key processed
	- Else
		- Perform row scan to find out the row index corresponding to the button pressed.
		- Based on the col and row index, find out the ascii code of the button pressed.
		- Generate a msg BT_DOWN with value the ascii code of the pressed button to be processing in the main loop.
		- Reconfigure the GPIOs for detecting a button up through the ISR (Row out, Col in) and later on for a new keypad parsing.

	- Enable the buttons interrupt again.
	- Return
	
Main Loop:
	- Upon receiving a button down message, do whatever was planned to do. For debug purpose, turn on LED
	- Upon receiving a button up message, the msg content has the key index. Change the button state to idle. For debug purpose, turn
	LED off
	
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
uint8_t	checkPassword(uint8_t keyPressed);

uint8_t	passwordIndex=0;
uint8_t password[4];

int main(void) {

	uint8_t rc;

	msgQueueDef readValue;

	initializeQueue(&IsrToMainQueue);

	HSI_RCC_Configuration();			// Configure system clock to HSI @ 8MHz

	// Configure two bits for preemption and two bits for priority
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	Config_NVIC();

	TIM4_Configuration ();				// Configure the debounce timer

	GPIO_SetAllAnalogInput();			// change all IOs into Analog INP to save power

	GPIO_ConfigDiscoveryLEDs();			// Debug using Discovery 2 LEDs

	Config_Keypad(ROW_OUT_COL_IN);		// initially configure colum pins as input that generate interrupts and row as output

										// Go to STOP mode to save power and wait for a key to be pressed to enter the main loop
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);

	while (1)  {						// Infinite loop

    	rc = getItemFromQueue(&IsrToMainQueue,&readValue);

    	if (rc != 0xff) {	//queue is not empty
			switch (readValue.msgID) {
				case MSG_BT_DOWN:						// A button down was detected, and the msg content holds the
														// ascii code of the key pressed
					GPIO_SetBits(LED_PORT, LED_BLUE_PIN);
														// Test if the content = 1234 as a test password
					if (checkPassword(readValue.msgContent)) {
						LED_PORT->ODR ^= LED_GREEN_PIN;	// Toggle Green LED
					}
					break;

				case MSG_BT_UP:							// Rest specific button status to idle
					GPIO_ResetBits(LED_PORT, LED_BLUE_PIN);
					keypadColState[readValue.msgContent] = BT_IDLE;
														// key full processed, then go to STOP mode to save power
					PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
					break;

				default:

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


/*
 * This function checks the pressed used button. If it matches the right sequence, it returns TRUE, else reset, and return FALSE
 */
uint8_t	checkPassword(uint8_t keyPressed) {

	switch (passwordIndex) {
		case 0:
			if (keyPressed == '1') {
				passwordIndex++;
			} else {
				passwordIndex = 0;
			}
			break;
		case 1:
			if (keyPressed == '2') {
				passwordIndex++;
			} else {
				passwordIndex = 0;
			}

			break;
		case 2:
			if (keyPressed == '3') {
				passwordIndex++;
			} else {
				passwordIndex = 0;
			}

			break;
		case 3:
			if (keyPressed == '4') {
				passwordIndex = 0;
				return 1;
			} else {
				passwordIndex = 0;
			}

			break;

		default:
			break;
	}
	return 0;

}
