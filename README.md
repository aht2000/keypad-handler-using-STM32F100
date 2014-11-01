keypad-handler-using-STM32F100
==============================
How to detect key pressed on 4x4 keypad using STM32F100.
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
