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

#if defined(__native_client__)
# include "platform_nacl.h"
# define HAVE_NACL_MAIN_H
#else
# define HAVE_TERMIOS_H
# define HAVE_SIGNAL_H
# define HAVE_TIME_H
#endif // defined(__native_client__)

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#if defined(HAVE_SIGNAL_H)
# include <signal.h>
#endif // defined(HAVE_SIGNAL_H)
#if defined(HAVE_TERMIOS_H)
# include <termios.h>
#endif // defined(HAVE_TERMIOS_H)
#if defined(HAVE_TIME_H)
# include <time.h>
#endif // defined(HAVE_TIME_H)

#if defined(HAVE_TERMIOS_H)
struct termios org_to;
#endif // defined(HAVE_TERMIOS_H)

int org_flags;
int fifo_flag;

static void
sleep
(void)
{
#if defined(HAVE_TIME_H)
  struct timespec req;
  struct timespec rem;
  req.tv_sec = 0;
  req.tv_nsec = 1000 * 1000 * 10; // 10msec
  nanosleep(&req, &rem);
#elif defined(HAVE_NACL_MAIN_H)
  nacl_sleep();
#endif
}

#if defined(HAVE_SIGNAL_H)
void
uart_term
(int num)
{
#if defined(HAVE_TERMIOS_H)
  tcsetattr(0, 0, &org_to);
#endif // defined(HAVE_TERMIOS_H)

  fcntl(0, F_SETFL, org_flags);
  exit(0);
}
#endif // defined(HAVE_SIGNAL_H)

void
uart_init
(void)
{
#if defined(HAVE_TERMIOS_H)
  tcgetattr(0, &org_to);
#endif // defined(HAVE_TERMIOS_H)

  org_flags = fcntl(0, F_GETFL, 0);

#if defined(HAVE_SIGNAL_H)
  signal(SIGTERM, uart_term);
#endif // defined(HAVE_SIGNAL_H)

#if defined(HAVE_TERMIOS_H)
  struct termios to;
  tcgetattr(0, &to);
  to.c_iflag &= ~(ICRNL);
  to.c_lflag &= ~(ECHO | ICANON);
  tcsetattr(0, TCSANOW, &to);
#endif // defined(HAVE_TERMIOS_H)

  fcntl(0, F_SETFL, org_flags | O_NDELAY);

  fifo_flag = -1;
}

void
uart_putchar
(unsigned char c)
{
  fputc((int)c, stdout);
  fflush(stdout);
}

int
uart_getchar
(void)
{
  if (-1 == fifo_flag) uart_peek();
  if (-1 != fifo_flag) {
    int rc = fifo_flag;
    fifo_flag = -1;
    return rc;
  }
  return -1;
}

int
uart_peek
(void)
{
  sleep();
  fifo_flag = fgetc(stdin);
  if (-1 != fifo_flag) return 1;
  return 0;
}
