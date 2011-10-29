/*
 * Copyright (c) 2011, Takashi TOYOSHIMA <toyoshim@gmail.com>
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
#if defined(TEST)
# if defined(__native_client__)
#  include "native_client/nacl_main.h"
# endif // defined(__native_client__)
# include <stdio.h>
# include <stdlib.h>
# include <fcntl.h>
# if !defined(__native_client__)
#  include <termios.h>
# endif // !defined(__native_client__)
# include <signal.h>
# include <time.h>
#else // defined(TEST)
# include <avr/io.h>
# include <avr/interrupt.h>
#endif // defined(TEST)

#if defined(TEST)
# if !defined(__native_client__)
struct termios org_to;
# endif // !defined(__native_client__)
int org_flags;
int fifo_flag;
int wait_flag;
#else // if defined(TEST)
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

#endif // defined(TEST)

static void
put_halfhex
(unsigned char c)
{
  if (c < 10) uart_putchar('0' + c);
  else uart_putchar('A' - 10 + c);
}

#if !defined(TEST)
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
#endif // !defined(TEST)

#if defined(TEST)
void
uart_term
(int num)
{
# if !defined(__native_client__)
  tcsetattr(0, 0, &org_to);
  fcntl(0, F_SETFL, org_flags);
# endif // !defined(__native_client__)
  exit(0);
}
#endif // defined(TEST)

void
uart_init
(void)
{
#if defined(TEST)
# if !defined(__native_client__)
  tcgetattr(0, &org_to);
  org_flags = fcntl(0, F_GETFL, 0);
  signal(SIGTERM, uart_term);

  struct termios to;
  tcgetattr(0, &to);
  to.c_iflag &= ~(ICRNL);
  to.c_lflag &= ~(ECHO | ICANON);
  int flags = org_flags | O_NDELAY;

  tcsetattr(0, TCSANOW, &to);
  fcntl(0, F_SETFL, flags);
# endif // !defined(__native_client__)

  fifo_flag = -1;
  wait_flag = -1;
#else // defined(TEST)
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
#endif // defined(TEST)
}

void
uart_putchar
(unsigned char c)
{
#if defined(TEST)
# if defined(__native_client__)
  nacl_putc(c);
# else // defined(__native_client__)
  fputc((int)c, stdout);
  fflush(stdout);
# endif // defined(__native_client__)
  wait_flag = -1;
#else // defined(TEST)
  disable_int();
  unsigned short rc = uart_tx(c);
  if (0 != rc) uart_push(rc);
  reset_int();
  enable_int();
  if (0 == (PINC & _BV(DDC0))) uart_int();
#endif // defined(TEST)
}

void
uart_puthex
(unsigned char c)
{
  put_halfhex(c >> 4);
  put_halfhex(c & 15);
}

void
uart_putnum_u16
(unsigned short n, int digit)
{
  unsigned short d = 10000;
  if (digit > 0) {
    d = 1;
    for (digit--; digit > 0; digit--) d *= 10;
  }
  do {
    int num = n / d;
    n = n % d;
    d /= 10;
    uart_putchar('0' + num);
  } while (0 != d);
}

void
uart_puts
(char *s)
{
#if defined(__native_client__)
  nacl_puts(s);
  wait_flag = -1;
#else // defined(__native_client__)
  while (0 != *s) uart_putchar(*s++);
#endif // defined(__native_client__)
}

void
uart_putsln
(char *s)
{
  uart_puts(s);
  uart_puts("\r\n");
}

int
uart_getchar
(void)
{
#if defined(TEST)
  wait_flag = -1;
  if (-1 == fifo_flag) uart_peek();
  wait_flag = -1;
  if (-1 != fifo_flag) {
    int rc = fifo_flag;
    fifo_flag = -1;
    return rc;
  }
# if !defined(__native_client__)
  struct timespec req;
  struct timespec rem;
  req.tv_sec = 0;
  req.tv_nsec = 1000 * 1000 * 10; // 10msec
  nanosleep(&req, &rem);
# else // !defined(__native_client__)
  nacl_sleep();
# endif // !defined(__native_client__)
  return -1;
#else // defined(TEST)
  if (fifo_rdptr == fifo_wrptr) return -1;
  int rc = fifo[fifo_rdptr++];
  fifo_rdptr &= 7;
  return rc;
#endif // defined(TEST)
}

int
uart_peek
(void)
{
#if defined(TEST)
# if !defined(__native_client__)
  if (-1 != wait_flag) {
    struct timespec req;
    struct timespec rem;
    req.tv_sec = 0;
    req.tv_nsec = 1000 * 1000 * 10; // 10msec
    nanosleep(&req, &rem);
  }
# else // !defined(__native_client__)
  nacl_sleep();
# endif // !defined(__native_client__)
  wait_flag = 0;
# if defined(__native_client__)
  fifo_flag = nacl_getc();
# else // defined(__native_client__)
  fifo_flag = fgetc(stdin);
# endif // defined(__native_client__)
  if (-1 != fifo_flag) return 1;
  return 0;
#else // defined(TEST)
  return (fifo_wrptr - fifo_rdptr) & 7;
#endif // defined(TEST)
}
