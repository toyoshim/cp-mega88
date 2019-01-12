/*
 * Copyright (c) 2019, Takashi TOYOSHIMA <toyoshim@gmail.com>
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

#include "io.h"

#include <avr/io.h>

void
io_init
(void)
{
  // PB3: /IOE -output
  // PB6: /READY - input, pull-up
  // PB7: /DATA - output
  DDRB &= ~_BV(PINB6);
  DDRB |= _BV(PINB3) | _BV(PINB7);
  PORTB |= _BV(PINB3) | _BV(PINB6);
}

void
io_out
(unsigned char port, unsigned char val)
{
  PORTD = port;
  PORTB |= _BV(PINB7) | _BV(PINB0);  // Address phase for READ access
  PORTB &= ~_BV(PINB3);  // Activate IOEXT

  while (PINB & _BV(PINB6));  // Wait until /READY is activated

  PORTB |= _BV(PINB3);  // Inctivate IOEXT
  while (!(PINB & _BV(PINB6)));  // Wait until /READY is inactivated

  PORTD = val;
  PORTB &= ~_BV(PINB7);  // Data phase
  PORTB &= ~_BV(PINB3);  // Activate IOEXT

  while (PINB & _BV(PINB6));  // Wait until /READY is activated

  PORTB |= _BV(PINB3);  // Inctivate IOEXT
  while (!(PINB & _BV(PINB6)));  // Wait until /READY is inactivated
}

unsigned char
io_in(unsigned char port)
{
  PORTD = port;
  PORTB |= _BV(PINB7);  // Address phase
  PORTB &= ~_BV(PINB0);  // for WRITE access
  PORTB &= ~_BV(PINB3);  // Activate IOEXT

  while (PINB & _BV(PINB6));  // Wait until /READY is activated

  PORTB |= _BV(PINB3);  // Inctivate IOEXT
  while (!(PINB & _BV(PINB6)));  // Wait until /READY is inactivated

  DDRD = 0;  // Set data ports as input
  PORTB &= ~_BV(PINB7);  // Data phase
  PORTB &= ~_BV(PINB3);  // Activate IOEXT

  while (PINB & _BV(PINB6));  // Wait until /READY is activated

  unsigned char data = PORTD;

  PORTB |= _BV(PINB3);  // Inctivate IOEXT
  while (!(PINB & _BV(PINB6)));  // Wait until /READY is inactivated

  DDRD = 0xff;  // Set back data portsas output
  return data;
}
