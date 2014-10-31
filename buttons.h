/*
 * buttons.h
*  Created on: Oct 26, 2014
*  Author: Ahmed Talaat (aa_talaat@yahoo.com)
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_

#include "gpio.h"

typedef enum {BT_IDLE, BT_GOING_DOWN, BT_DOWN, BT_GOING_UP, BT_UP} BUTTON_STATE;

extern BUTTON_STATE	keypadColState[4];		// Holds the status of the pressed columns (1-4) of keys in the keypad

void Config_Keypad(KEYPAD_GPIO_MODE keypadMode);
void EnableKeypadExti_IRQ(void);
void DisableKeypadExti_IRQ(void);
uint8_t	getKeyPressed(uint8_t colIndex);

#endif /* BUTTONS_H_ */
