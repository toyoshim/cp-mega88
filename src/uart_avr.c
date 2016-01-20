/*
 * Copyright (c) 2016, Takashi TOYOSHIMA <toyoshim@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * - Neither the name of the authors nor the names of its contributors may be
 *   used to endorse or promote products derived from this software with out
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUE
 * NTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include "uart.h"

#include <avr/io.h>
#include <avr/interrupt.h>

extern unsigned short uart_tx(unsigned char);
extern unsigned char uart_rx(void);

static void
reset_int
(void)
{
  PCIFR |= _BV(PCIF1);	// Reset PCINT1 Flag
}

static void
enable_int
(void)
{
  PCICR |= _BV(PCIE1);	// Enable PCINT1
}

static void
disable_int
(void)
{
  PCICR &= ~_BV(PCIE1);	// Disable PCINT1
}

static unsigned char
fifo[8];

static unsigned char
fifo_rdptr = 0;

static unsigned char
fifo_wrptr = 0;

static void
uart_push
(unsigned char c)
{
  if (fifo_rdptr != (fifo_wrptr + 1)) {
    fifo[fifo_wrptr++] = c;
    fifo_wrptr &= 7;
  }
}

static void
uart_int
(void)
{
  disable_int();
  uart_push(uart_rx());
  reset_int();
  enable_int();
}

ISR
(PCINT1_vect)
{
  uart_int();
}

void
uart_init
(void)
{
  /*
   * PC0: RXD
   * PC1: TXD
   */
  // Port Settings
  DDRC   &= ~_BV(DDC0);		// PC0: Input
  PORTC  |=  _BV(DDC0);		//      Pull-up
  DDRC   |=  _BV(DDC1);		// PC1: Output

  PCMSK1 |=  _BV(PCINT8);	// PC0 cause PCINT1

  SREG   |=  _BV(SREG_I);	// Enable Interrupt

  enable_int();
}

void
uart_putchar
(unsigned char c)
{
  disable_int();
  unsigned short rc = uart_tx(c);
  if (0 != rc) uart_push(rc);
  reset_int();
  enable_int();
  if (0 == (PINC & _BV(DDC0))) uart_int();
}

int
uart_getchar
(void)
{
  if (fifo_rdptr == fifo_wrptr) return -1;
  int rc = fifo[fifo_rdptr++];
  fifo_rdptr &= 7;
  return rc;
}

int
uart_peek
(void)
{
  return (fifo_wrptr - fifo_rdptr) & 7;
}
