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
//  native_stm32.h
//

#ifndef NATIVE_STM32_H
#define NATIVE_STM32_H

void native_stm32_init(void);
void native_stm32_stm32_invoke(u08_t mref);
void native_stm32_gpio_invoke(u08_t mref);
void native_stm32_timer_invoke(u08_t mref);
void native_stm32_adc_invoke(u08_t mref);
void native_stm32_pwm_invoke(u08_t mref);

#endif // NATIVE_STM32_H
