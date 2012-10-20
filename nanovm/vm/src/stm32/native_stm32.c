//
//  NanoVM, a tiny java VM for the ST-Microelectronics STM32 family
//  Copyright (C) 2012 by Truong-Khang LE <tkle@fxrsolutions.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

//
//  native_stm32.c
//

#include "types.h"
#include "debug.h"
#include "config.h"
#include "error.h"

#ifndef OLIMEXINO
#ifdef STM32
#include "stm32f10x.h"

#include "vm.h"
#include "native.h"
#include "native_stm32.h"
#include "stack.h"
#include "uart.h"
#include "eeprom.h"

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

volatile static nvm_int_t ticks;

/* Virtual address defined by the user: 0xFFFF value is prohibited */
uint16_t VirtAddVarTab[NumbOfVar] = {0x5555, 0x6666, 0x7777};


/**
  * @brief  This function handles TIM1 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM1_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM1, TIM_IT_Update) != RESET)
	{
		/* Update system's ticks */
		ticks++;

		/* Clear UIF flag */
		TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
	}
}

void native_init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	uint16_t PrescalerValue = 0;

	/*!< At this stage the microcontroller clock setting is already configured,
	   this is done through SystemInit() function which is called from startup
	   file (startup_stm32f10x_xx.s) before to branch to application main.
	   To reconfigure the default setting of SystemInit() function, refer to
	   system_stm32f10x.c file
	 */

	/* TIM1 clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

	/* Configure the NVIC Preemption Priority Bits */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	/* Enable the TIM1 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	/* ---------------------------------------------------------------
		TIM1 Configuration: Up counting mode
		TIM1 counter clock at 1 MHz
		TIM1 generates an update event every 1000 pulses
	--------------------------------------------------------------- */

	/* Compute the prescaler value */
	PrescalerValue = (uint16_t) (SystemCoreClock/1000000) - 1;

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 1000;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_RepetitionCounter	= 0;

	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	/* Prescaler configuration */
	TIM_PrescalerConfig(TIM1, PrescalerValue, TIM_PSCReloadMode_Immediate);

	/* Auto-reload preload configuration */
	TIM_ARRPreloadConfig(TIM1, ENABLE);

	/* TIM IT enable */
	TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);

	/* TIM1 enable counter */
	TIM_Cmd(TIM1, ENABLE);

	/* Unlock the Flash Program Erase controller */
	FLASH_Unlock();

	/* EEPROM Init */
	EE_Init();
}

// the STM32 class
void native_stm32_stm32_invoke(u08_t mref){
	if(mref == NATIVE_METHOD_GETCLOCK) {
		stack_push(CLOCK/1000);
	} else
		error(ERROR_NATIVE_UNKNOWN_METHOD);
}

// the gpio class
void native_stm32_gpio_invoke(u08_t mref){
	if(mref == NATIVE_METHOD_SETINPUT) {
		u08_t bit  = stack_pop();
		u08_t port = stack_pop();
		DEBUGF("native setinput %bd/%bd\n", port, bit);
//		*ddrs[port] &= ~_BV(bit);
	} else if(mref == NATIVE_METHOD_SETOUTPUT) {
		u08_t bit  = stack_pop();
		u08_t port = stack_pop();
		DEBUGF("native setoutput %bd/%bd\n", port, bit);
//		*ddrs[port] |= _BV(bit);
	} else if(mref == NATIVE_METHOD_SETBIT) {
		u08_t bit  = stack_pop();
		u08_t port = stack_pop();
		DEBUGF("native setbit %bd/%bd\n", port, bit);
		GPIO_SetBits((GPIO_TypeDef *)(GPIOA_BASE + port*0x00000400),  (uint16_t)(bit<<1));
	} else if(mref == NATIVE_METHOD_CLRBIT) {
		u08_t bit  = stack_pop();
		u08_t port = stack_pop();
		DEBUGF("native clrbit %bd/%bd\n", port, bit);
		GPIO_ResetBits((GPIO_TypeDef *)(GPIOA_BASE + port*0x00000400),  (uint16_t)(bit<<1));
	} else
		error(ERROR_NATIVE_UNKNOWN_METHOD);
}

// the timer class, based on STM32 32-bit timer 1
void native_stm32_timer_invoke(u08_t mref){
	if(mref == NATIVE_METHOD_SETSPEED) {
		/*OCR1A =*/ stack_pop_int();  // set reload value
	} else if(mref == NATIVE_METHOD_GET) {
		stack_push(ticks);
	} else if(mref == NATIVE_METHOD_TWAIT) {
		nvm_int_t wait = stack_pop_int();
		ticks = 0;
	while(ticks < wait);      // reset watchdog here if enabled
	} else if(mref == NATIVE_METHOD_SETPRESCALER) {
		/*TCCR1B = */stack_pop_int();
	} else
		error(ERROR_NATIVE_UNKNOWN_METHOD);
}

// the Adc class
void native_stm32_adc_invoke(u08_t mref){

}

// the Pwm class
void native_stm32_pwm_invoke(u08_t mref){

}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}

#endif //USE_FULL_ASSERT

#endif //STM32
#endif //!OLIMEXINO
