/*
 * gpio.c
 *
 *  Created on: Oct 26, 2014
 *  Author: Ahmed Talaat (aa_talaat@yahoo.com)
 *
 * Contains all the functions required to configure the GPIO peripherals that serve other peripherals.
 *
 *
 */
#include "stm32f10x.h"
#include "gpio.h"

/* Change all IOs into Analog INP to save power except the user input buttons to allow waking up the device from STOP
 * mode by the user.
*/
void GPIO_SetAllAnalogInput(void){

	GPIO_InitTypeDef 	GPIO_InitStruct;

	  /* Enable GPIOs clock */
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);

	  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_All;
	  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;

	  GPIO_Init(GPIOC, &GPIO_InitStruct);
	  GPIO_Init(GPIOD, &GPIO_InitStruct);

	  // Keep the STLink Pins alive.
	  GPIO_InitStruct.GPIO_Pin = GPIO_Pin_All & (~(GPIO_Pin_13 | GPIO_Pin_14));
	  GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /* disable All PORTs clock */
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, DISABLE);
}


/*
 * Configure the GPIO pins connected to the keypad. The mode parameter decides if the rows will be pullup inputs and columns as
 * outputs or vice versa.
 * The output pins are reset all to low.
 */

void GPIO_ConfigKeyPad(KEYPAD_GPIO_MODE keypadMode) {
	GPIO_InitTypeDef 	GPIO_InitStruct;

	RCC_APB2PeriphClockCmd(KEYPAD_CLK, ENABLE);

	switch (keypadMode) {
		case ROW_IN_COL_OUT:
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
			GPIO_InitStruct.GPIO_Pin = KEYPAD_ROW1 | KEYPAD_ROW2 | KEYPAD_ROW3 | KEYPAD_ROW4;
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
			GPIO_Init(KEYPAD_PORT, &GPIO_InitStruct);

			GPIO_InitStruct.GPIO_Pin = KEYPAD_COL1 | KEYPAD_COL2 | KEYPAD_COL3 | KEYPAD_COL4;
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
			GPIO_Init(KEYPAD_PORT, &GPIO_InitStruct);
													// Set all output pins to low
			GPIO_ResetBits(KEYPAD_PORT, KEYPAD_COL1 | KEYPAD_COL2 | KEYPAD_COL3 | KEYPAD_COL4);
			break;

		case ROW_OUT_COL_IN:
			GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
			GPIO_InitStruct.GPIO_Pin = KEYPAD_COL1 | KEYPAD_COL2 | KEYPAD_COL3 | KEYPAD_COL4;
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
			GPIO_Init(KEYPAD_PORT, &GPIO_InitStruct);

			GPIO_InitStruct.GPIO_Pin = KEYPAD_ROW1 | KEYPAD_ROW2 | KEYPAD_ROW3 | KEYPAD_ROW4;
			GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
			GPIO_Init(KEYPAD_PORT, &GPIO_InitStruct);
													// Set all output pins to low
			GPIO_ResetBits(KEYPAD_PORT, KEYPAD_ROW1 | KEYPAD_ROW2 | KEYPAD_ROW3 | KEYPAD_ROW4);
			break;

		default:
			break;
	}

}
