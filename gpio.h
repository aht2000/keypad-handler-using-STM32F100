/*
 * gpio.h
 *
 *  Created on: Oct 26, 2014
 *  Author: Ahmed Talaat (aa_talaat@yahoo.com)
 */

#ifndef GPIO_H_
#define GPIO_H_

#define KEYPAD_PORT		GPIOB
#define KEYPAD_CLK		RCC_APB2Periph_GPIOB

#define LED_PORT		GPIOC
#define LED_CLK			RCC_APB2Periph_GPIOC

#define KEYPAD_ROW1		GPIO_Pin_8
#define KEYPAD_ROW2		GPIO_Pin_9
#define KEYPAD_ROW3		GPIO_Pin_10
#define KEYPAD_ROW4		GPIO_Pin_11

#define KEYPAD_COL1		GPIO_Pin_12
#define KEYPAD_COL2		GPIO_Pin_13
#define KEYPAD_COL3		GPIO_Pin_14
#define KEYPAD_COL4		GPIO_Pin_15

#define LED_BLUE_PIN	GPIO_Pin_8
#define LED_GREEN_PIN	GPIO_Pin_9

/*
 * To identify the key that the user pressed among the 16 keys of the 4x4 keypad.
 * We will be changing the way we interface with the keypad. It can be row as pull-up inputs with falling edge interrupt, and
 * columns as output or it can be row as outputs and columns as pull-up inputs
 */
typedef enum {ROW_IN_COL_OUT, ROW_OUT_COL_IN} KEYPAD_GPIO_MODE;

void GPIO_SetAllAnalogInput(void);
void GPIO_ConfigKeyPad(KEYPAD_GPIO_MODE mode);
void GPIO_ConfigDiscoveryLEDs(void);

#endif /* GPIO_H_ */
