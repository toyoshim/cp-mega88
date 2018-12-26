/*
 * Copyright (c) 2018, Takashi TOYOSHIMA <toyoshim@gmail.com>
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

void
io_init
(void)
{
}

// z80pack defines following ports.
//   0: console status
//   1: console data
//   2: printer status
//   3: printer data
//   4: auxiliary status
//   5: auxiliary data
//  10: FDC drive
//  11: FDC track
//  12: FDC sector (low)
//  13: FDC command
//  14: FDC status
//  15: DMA destination address low
//  16: DMA destination address high
//  17: FDC sector high
//  20: MMU initialisation
//  21: MMU bank select
//  22: MMU select segment size (in pages a 256 bytes)
//  23: MMU write protect/unprotect common memory segment
//  25: clock command
//  26: clock data
//  27: 10ms timer causing maskable interrupt
//  28: x * 10ms delay circuit for busy waiting loops
//  29: hardware control
//  30: CPU speed low
//  31: CPU speed high
//  40: passive socket #1 status
//  41: passive socket #1 data
//  42: passive socket #2 status
//  43: passive socket #2 data
//  44: passive socket #3 status
//  45: passive socket #3 data
//  46: passive socket #4 status
//  47: passive socket #4 data
//  50: client socket #1 status
//  51: client socket #1 data

void
io_out
(unsigned char port, unsigned char val)
{
  // Following ports are handled in machine.c.
  //  1: console data
  // 10: FDC drive
  // 11: FDC track
  // 12: FDC sector (low)
  // 13: FDC command
  // 15: DMA destination address low
  // 16: DMA destination address high
}

unsigned char
io_in(unsigned char port)
{
  // Following ports are handled in machine.c.
  //  0: console status
  //  1: console data
  // 14: FDC status
  return 0;
}
