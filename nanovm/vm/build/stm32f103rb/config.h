//
// config.h
//
// NanoVM configuration file for the stm32 test board
//

#ifndef CONFIG_H
#define CONFIG_H

// cpu related setup
//#define STM32
#define CLOCK 8000000

// uart setup
#define UART_BITRATE 9600

#define CODESIZE 1024	// MUST BE < (EEPROM_END_ADDRESS - EEPROM_START_ADDRESS + 1)
#define HEAPSIZE 1024

// stm32 specific native init routines
#define NATIVE_INIT  native_init()

// vm setup
#undef NVM_USE_STACK_CHECK      // enable check if method returns empty stack
#define NVM_USE_ARRAY            // enable arrays
#define NVM_USE_SWITCH           // support switch instruction
#define NVM_USE_INHERITANCE      // support for inheritance

// native setup
#define NVM_USE_STDIO            // enable native stdio support

// marker used to indicate, that this item is stored in eeprom
#define NVMFILE_FLAG     0x8000

#endif // CONFIG_H
