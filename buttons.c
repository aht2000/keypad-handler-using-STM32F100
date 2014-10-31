/*
 * buttons.c
 *
 *  Created on: Oct 26, 2014
 *      Author: Ahmed
 */

#include "stm32f10x.h"
#include "buttons.h"
#include "gpio.h"

void ConfigKeypadInterrupt(void);

// Holds the status of the pressed columns (1-4) of keys in the keypad
BUTTON_STATE	keypadColState[4] = {BT_IDLE, BT_IDLE, BT_IDLE, BT_IDLE};

uint8_t	keyMap[16]={'1','2','3','A',
					'4','5','6','B',
					'7','8','9','C',
					'*','0','#','D'};

/*
 * This function will configure the GPIO, and associated external interrupt configuration as per the keypad scanning mode.
 * The mode parameter decides if the rows will be pullup inputs with falling edge interrupt and columns as outputs or vice versa
 */

void Config_Keypad(KEYPAD_GPIO_MODE keypadMode) {

	GPIO_ConfigKeyPad(keypadMode);	// do the GPIO configuration as requested

	switch (keypadMode) {
		case ROW_IN_COL_OUT:
									// Nothing is required here.
			break;

		case ROW_OUT_COL_IN:
			ConfigKeypadInterrupt();// configure column pins Alternate function as external interrupt source
									// and link each pin to its interrupt line
			EnableKeypadExti_IRQ();	// Clear pending interrupts, and Enable interrupt mask for these pins
			break;
		default:
			break;
	}
}

/*
 * Configure the alternate function of the buttons as external interrupt source and link it to the external pins
 */
void ConfigKeypadInterrupt(void) {
	EXTI_InitTypeDef   	EXTI_InitStructure;

	/* Enable AFIO clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	/* Connect EXTI12-15 Line to PB.12-15 pin */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource12);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource13);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource15);

	EXTI_InitStructure.EXTI_Line = EXTI_Line12 | EXTI_Line13 | EXTI_Line14 | EXTI_Line15;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
}

/*
 * Clear the interrupt mask for the 4 EXTI lines
*/
void EnableKeypadExti_IRQ(void){
														// Clear the  EXTI line 12 pending bit
	EXTI_ClearITPendingBit(EXTI_Line12 | EXTI_Line13 | EXTI_Line14 | EXTI_Line15);
	EXTI->IMR = EXTI_IMR_MR12 | EXTI_IMR_MR13 | EXTI_IMR_MR14 | EXTI_IMR_MR15;
}

/*
 * Set the interrupt mask for the 4 EXTI lines
*/
void DisableKeypadExti_IRQ(void){
														// Mask interrupt
	EXTI->IMR &= ~(EXTI_IMR_MR12 | EXTI_IMR_MR13 | EXTI_IMR_MR14 | EXTI_IMR_MR15);
														// Clear the EXTI line 12-15 pending bit
	EXTI_ClearITPendingBit(EXTI_Line12 | EXTI_Line13 | EXTI_Line14 | EXTI_Line15);
}


/*
 * This function is called after the user has pressed a key on the keypad. The key column activated due to user selection is
 * already known through the interrupt routines, and is passed to this function.
 * The function will switch the GPIO setup of the rows and columns to make the columns as output low, and will scan row by row
 * till it finds one which is low. Now that we know the row, and column index, using a lookup table, the function will
 * return an ascii code of the button pressed.
 * The function will return 0 if it cannot find and row with low logic. This can happen if the time between detecting the column
 * index and calling this function is too long so that the user has already removed his finger, and a button up message is
 * received in the main loop
 */

uint8_t	getKeyPressed(uint8_t colIndex) {

	uint8_t	rowIndex;

	Config_Keypad(ROW_IN_COL_OUT);			// Configure keypad with column pins as output low and row pins as input pullup

	if (!GPIO_ReadInputDataBit(KEYPAD_PORT, KEYPAD_ROW1)) {	// If a low level detected
		rowIndex=0;
	} else
	if (!GPIO_ReadInputDataBit(KEYPAD_PORT, KEYPAD_ROW2)) {	// If a low level detected
		rowIndex=1;
	} else
	if (!GPIO_ReadInputDataBit(KEYPAD_PORT, KEYPAD_ROW3)) {	// If a low level detected
		rowIndex=2;
	} else
	if (!GPIO_ReadInputDataBit(KEYPAD_PORT, KEYPAD_ROW4)) {	// If a low level detected
		rowIndex=3;
	} else {
		return 0;											// error detected
	}

	return (keyMap[4*rowIndex+colIndex]);
}
