//
//  NanoVM, a tiny java VM for the Atmel AVR family
//  Copyright (C) 2005 by Till Harbaum <Till@Harbaum.org>
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

#ifndef EEPROM_H
#define EEPROM_H

#ifndef STM32
typedef u08_t* eeprom_addr_t;
#else
typedef uint16_t eeprom_addr_t;
#endif

#ifdef UNIX
#include <string.h> // for memcpy

#define eeprom_write_byte(a, d) { *a = d; }
#define eeprom_read_block(a, s, l) { memcpy(a, s, l); }
#define eeprom_write_block(a, s, l) { memcpy(s, a, l); }
#define EEPROM
#else
#ifdef AVR

// TH: Otherwise gcc-3.4.5/avr-lib-1.4.2 complain about unknow
// symbol asm ... is there a more beautiful solution?
#define asm __asm__

#include <avr/eeprom.h>
#define EEPROM __attribute__((section (".eeprom")))
#else
#ifdef STM32
#include <../../bsp/Emu/eeprom.h>	// eeprom emulator

#define eeprom_write_byte(addr, data) {  EE_WriteVariable(addr, (uint16_t)(data)); }
#define eeprom_read_block(dst, src, l) { u08_t* _ptr=(u08_t *)dst; while (l--) { EE_ReadVariable(src, _ptr++); }; }
#define eeprom_write_block(src, dst, l) { u08_t* _ptr=(u08_t *)src; while (l--) { EE_WriteVariable(dst, (uint16_t)(*_ptr++)); }; }
#define EEPROM
#else
#warning "Unknown EEPROM setup"
#endif
#endif
#endif

#endif // UART_H
