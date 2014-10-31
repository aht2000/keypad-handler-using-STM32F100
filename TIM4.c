/*
 * TIM4.c
 *
 *  Created on: Oct 26, 2014
 *  Author: Ahmed Talaat (aa_talaat@yahoo.com)
 * TIM4 is configured to generate a 20ms interrupt to use it as a debounce delay
 *
 */

#include "stm32f10x.h"
#include "TIM4.h"

/* Private function prototypes -----------------------------------------------*/

/*
 * TIM4 configures to generate an interrupt each 20ms
 *
 */

void TIM4_Configuration (){

TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE );
    /* Enable the TIM4 global Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    NVIC_Init(&NVIC_InitStructure);

    /* Time base configuration */

    // TIM2 frequency = counter clock / (period + 1) = 1000 / (19+1) = 50 Hz --> 20ms
    // Prescaler = (SystemCoreClock / Fx) - 1 where FX is the timer clock we want to use

    TIM_TimeBaseInitStruct.TIM_Period = 19;
    TIM_TimeBaseInitStruct.TIM_Prescaler = (uint16_t) (SystemCoreClock / 1000) - 1;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = 0;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;

    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStruct);
}

/*
 * Loads the timer with a 10ms debouce delay, and configure it to generate an interrupt once expired
 */
void enableDebounceTimer(void) {
    /* TIM4 enable counter */
	TIM_SetCounter(TIM4,0);							// Reset its counter
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);		// Enable TIM Interrupt
    TIM_Cmd(TIM4, ENABLE);							// Enable Timer
}

/*
 * Disable timer, and clear all interrupts
 */
void disableDebounceTimer(void) {
    TIM_Cmd(TIM4, DISABLE);							// Disable Timer
    TIM_ITConfig(TIM4, TIM_IT_Update, DISABLE);		// Disable TIM Interrupt
	TIM_SetCounter(TIM4,0);							// Reset its counter
	TIM_ClearITPendingBit(TIM4, TIM_IT_Update);		// Clear any pending interrupt bit so that we do not come here again
}
