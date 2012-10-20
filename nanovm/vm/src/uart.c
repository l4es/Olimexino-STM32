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

//
//  uart.c
//

#include "types.h"
#include "config.h"
#include "debug.h"

#include "uart.h"
#include "delay.h"

// unix uart emulation
#ifdef UNIX
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <termios.h>
#include <stdio.h>
#include <sys/select.h>

struct termios old_t;

FILE *in = NULL, *out = NULL;

void uart_bye(void) {
#ifdef UART_PORT
  fclose(in);  // out is identical
#else
  // restore terminal settings
  tcsetattr( 0, TCSANOW, &old_t);
#endif
}

void uart_sigproc() {
  exit(-1);   // exit (and thus call uart_bye)
}

void uart_init(void) {
#ifdef UART_PORT
  out = in = fopen(UART_PORT, "w+b");
  if(!in) {
    printf("unable to open %s\n", UART_PORT);
    exit(-1);
  }

#else
  struct termios new_t;

  in = stdin;
  out = stdout;

  if(tcgetattr( 0, &old_t) == -1)
    perror("tcgetattr() failed");

  memcpy( &new_t, &old_t, sizeof(struct termios));

  tcflush( 0, TCIFLUSH);

  // no cr/lf translation
  new_t.c_iflag &= ~(ICRNL);

  // echo and kernel buffers off
  new_t.c_lflag &= ~(ECHO|ICANON);

  tcsetattr( 0, TCSANOW, &new_t);

  // libc buffers off
  setbuf(stdin, NULL);
#endif

  atexit(uart_bye);
  signal(SIGINT, uart_sigproc);
}

void uart_write_byte(u08_t byte) {
  fputc(byte, out);
  fflush(out);
}

u08_t uart_read_byte(void) {
  return fgetc(in);
}

// unix can't tell us how many bytes in the input buffer are,
// so just return one as long as there's data
u08_t uart_available(void) {
  fd_set fds;
  struct timeval tv = { 0, 100 };

  FD_ZERO(&fds);
  FD_SET(fileno(in), &fds);

  return (select(FD_SETSIZE, &fds, NULL, NULL, &tv) == 1)?1:0;
}

#endif  // UNIX

#ifdef AVR
#include <avr/io.h>
#include <avr/interrupt.h>

#define UART_BITRATE_CONFIG (u16_t)(((CLOCK/16l)/(UART_BITRATE))-1)
#define UART_BUFFER_SIZE  (1<<(UART_BUFFER_BITS))
#define UART_BUFFER_MASK  ((UART_BUFFER_SIZE)-1)

#if defined(ATMEGA168)
#define UBRRH UBRR0H
#define UBRRL UBRR0L
#define UCSRA UCSR0A
#define UCSRB UCSR0B
#define UCSRC UCSR0C
#define TXEN TXEN0
#define RXEN RXEN0
#define RXCIE RXCIE0
#define URSEL UMSEL00
#define UCSZ0 UCSZ00
#define UDR UDR0
#define UDRE UDRE0
#define SIG_UART_RECV SIG_USART_RECV
#endif

#if defined(NIBO)
#define UBRRH UBRR0H
#define UBRRL UBRR0L
#define UCSRA UCSR0A
#define UCSRB UCSR0B
#define UCSRC UCSR0C
#define UDR UDR0

#define URSEL UBRR0H
#define SIG_UART_RECV SIG_UART0_RECV
/*
#define TXEN TXEN0
#define RXEN RXEN0
#define RXCIE RXCIE0
#define UCSZ0 UCSZ00
#define UDRE UDRE0
#define SIG_UART_RECV SIG_USART_RECV
*/
#endif


u08_t uart_rd, uart_wr;
u08_t uart_buf[UART_BUFFER_SIZE];

void uart_init(void) {
  uart_rd = uart_wr = 0;   // init buffers

  UBRRH = (u08_t)((UART_BITRATE_CONFIG>>8) & 0xf);
  UBRRL = (u08_t)((UART_BITRATE_CONFIG) & 0xff);

  UCSRA = 0;
  UCSRB =
    _BV(RXEN) | _BV(RXCIE) |          // enable receiver and irq
    _BV(TXEN);                        // enable transmitter

  UCSRC = _BV(URSEL) | (3 << UCSZ0);  // 8n1

  sei();
}

SIGNAL(SIG_UART_RECV) {
  /* irq driven input */
  uart_buf[uart_wr] = UDR;

  /* and increase write pointer */
  uart_wr = ((uart_wr+1) & UART_BUFFER_MASK);
}

u08_t uart_available(void) {
  return(UART_BUFFER_MASK & (uart_wr - uart_rd));
}

void uart_write_byte(u08_t byte) {
  /* Wait for empty transmit buffer */
  while(!(UCSRA & _BV(UDRE)));

  // asuro needs echo cancellation, since the ir receiver "sees"
  // the transmitter
#ifdef ASURO
  // disable receiver
  UCSRB &= ~(_BV(RXEN) | _BV(RXCIE));
#endif

  // start transmission
  UDR = byte;

#ifdef ASURO
  // Wait for empty transmit buffer
  while(!(UCSRA & _BV(UDRE)));
  delay(MILLISEC(5));

  // re-enable receiver
  UCSRB |= _BV(RXEN) | _BV(RXCIE);
#endif
}

u08_t uart_read_byte(void) {
  u08_t ret = uart_buf[uart_rd];

  /* and increase read pointer */
  uart_rd = ((uart_rd+1) & UART_BUFFER_MASK);

  return ret;
}

#endif // AVR

#ifdef STM32
#include "stm32f10x.h"


/* Used COM port definitions -------------------------------------------------
  Notes:
 *----------------------------------------------------------------------------*/
#define USART                   USART1
#define USART_GPIO              GPIOA
#define USART_CLK               RCC_APB2Periph_USART1
#define USART_GPIO_CLK          RCC_APB2Periph_GPIOA
#define USART_RxPin             GPIO_Pin_10
#define USART_TxPin             GPIO_Pin_9
#define USART_IRQn              USART1_IRQn
#define USART_IRQHandler        USART1_IRQHandler

/* Buffers' definitions ------------------------------------------------------
  Notes:
  The length of the receive and transmit buffers must be a power of 2.
  Each buffer has a next_in and a next_out index.
  If next_in = next_out, the buffer is empty.
  (next_in - next_out) % buffer_size = the number of characters in the buffer.
 *----------------------------------------------------------------------------*/
#define TBUF_SIZE   256	     /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/
#define RBUF_SIZE   256      /*** Must be a power of 2 (2,4,8,16,32,64,128,256,512,...) ***/
/*----------------------------------------------------------------------------
 *----------------------------------------------------------------------------*/
#if TBUF_SIZE < 2
#error TBUF_SIZE is too small.  It must be larger than 1.
#elif ((TBUF_SIZE & (TBUF_SIZE-1)) != 0)
#error TBUF_SIZE must be a power of 2.
#endif

#if RBUF_SIZE < 2
#error RBUF_SIZE is too small.  It must be larger than 1.
#elif ((RBUF_SIZE & (RBUF_SIZE-1)) != 0)
#error RBUF_SIZE must be a power of 2.
#endif

/*----------------------------------------------------------------------------
 *----------------------------------------------------------------------------*/
struct buf_st {
  u08_t in;                                // Next In Index
  u08_t out;                               // Next Out Index
  u08_t buf [RBUF_SIZE];                   // Buffer
};

static struct buf_st rbuf = { 0, 0, };
#define SIO_RBUFLEN ((unsigned short)(rbuf.in - rbuf.out))

static struct buf_st tbuf = { 0, 0, };
#define SIO_TBUFLEN ((unsigned short)(tbuf.in - tbuf.out))

static u08_t tx_restart = 1;               // NZ if TX restart is required

/*----------------------------------------------------------------------------
  USART1_IRQHandler
  Handles USART1 global interrupt request.
 *----------------------------------------------------------------------------*/
/**
  * @brief  This function handles USARTy global interrupt request.
  * @param  None
  * @retval None
  */
void USART_IRQHandler(void)
{
	struct buf_st	*p;

	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		/* Clear interrupt flag */
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);

	    /* Read one byte from the receive data register */
		p = &rbuf;
		if (((p->in - p->out) & ~(RBUF_SIZE-1)) == 0) {
			p->buf[p->in & (RBUF_SIZE-1)] = USART_ReceiveData(USART1);
			p->in++;
		}
	}

	if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
	{

		/* Clear interrupt flag */
		USART_ClearITPendingBit(USART1, USART_IT_TXE);

		/* Write one byte to the transmit data register */
		p = &tbuf;

		if (p->in != p->out)
		{
			USART_SendData(USART1, p->buf [p->out & (TBUF_SIZE-1)]);
			p->out++;
			tx_restart = 0;
		}
		else
		{
			tx_restart = 1;

			/* Disable the USARTy Transmit interrupt */
			USART_ITConfig(USART1, USART_IT_TXE, DISABLE);
		}
	}
}

/*------------------------------------------------------------------------------
  uart_buffer_init
  initialize the buffers
 *------------------------------------------------------------------------------*/
void uart_buffer_init (void) {
	/* Clear com. buffer indexes */
	tbuf.in 	= 0;
	tbuf.out 	= 0;
	tx_restart 	= 1;
	rbuf.in 	= 0;
	rbuf.out 	= 0;
}

/*------------------------------------------------------------------------------
  uart_hardware_init
  initialize the physical com. port
 *------------------------------------------------------------------------------*/
void uart_hardware_init (void) {
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable USART clock */
	RCC_APB2PeriphClockCmd(USART_CLK, ENABLE);

	/* Enable GPIO clock */
	RCC_APB2PeriphClockCmd(USART_GPIO_CLK | RCC_APB2Periph_AFIO, ENABLE);

	/*  */

	/* Enable the USART Pins Software Remapping */
	GPIO_PinRemapConfig(GPIO_Remap_USART1, ENABLE);

	/* Configure USART Rx as input floating */
	GPIO_InitStructure.GPIO_Pin = USART_RxPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(USART_GPIO, &GPIO_InitStructure);

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = USART_TxPin;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(USART_GPIO, &GPIO_InitStructure);


	/* Configure the NVIC Preemption Priority Bits */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

	/* Enable the USARTy Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/*------------------------------------------------------------------------------
  uart_write_byte
  transmit a character
 *------------------------------------------------------------------------------*/
void uart_write_byte (u08_t c) {
	struct buf_st *p = &tbuf;

	// If the buffer is full, return an error value
	if (SIO_TBUFLEN >= TBUF_SIZE)
		return;
	
	// Add data to the transmit buffer.
	p->buf [p->in & (TBUF_SIZE - 1)] = c;
	p->in++;

	// If transmit interrupt is disabled, enable it
	if (tx_restart) {
		tx_restart = 0;
		USART_ITConfig(USART1, USART_IT_TXE, ENABLE);	// enable TX interrupt
	}
}

/*------------------------------------------------------------------------------
  uart_read_byte
  receive a character
 *------------------------------------------------------------------------------*/
 u08_t uart_read_byte (void) {
	struct buf_st *p = &rbuf;

	if (SIO_RBUFLEN == 0)
		return (0);

	return (p->buf [(p->out++) & (RBUF_SIZE - 1)]);
}

/*------------------------------------------------------------------------------
  uart_available
  count bytes in rx buffer
 *------------------------------------------------------------------------------*/
u08_t uart_available(void) {
	struct buf_st *p = &rbuf;
	return((p->out - p->in)& (RBUF_SIZE - 1));
}

void uart_init(void) {
	USART_InitTypeDef USART_InitStructure;

	/* Init UART buffers */
	uart_buffer_init();

	/* Init UART hardware */
	uart_hardware_init();

	/* Set UART configuration */
	USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	/* Apply UART configuration */
	USART_Init(USART, &USART_InitStructure);

	/* Enable USART Receive and Transmit interrupts */
	USART_ITConfig(USART, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USART, USART_IT_TXE, ENABLE);

	/* Enable the USART */
	USART_Cmd(USART, ENABLE);
}

#endif // STM32

// translate nl to cr nl
void uart_putc(u08_t byte) {
	if(byte == '\n')
		uart_write_byte('\r');

	uart_write_byte(byte);
}

