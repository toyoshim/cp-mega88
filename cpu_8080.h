/*
 * Copyright (c) 2010, Takashi TOYOSHIMA <toyoshim@gmail.com>
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

#if !defined(__cpu_8080_h__)
# define __cpu_8080_h__

# if defined(__cplusplus)
extern "C" {
# endif // defined(__cplusplus)

typedef unsigned char (*cpu_8080_load_8)(unsigned short addr);
typedef void (*cpu_8080_store_8)(unsigned short addr, unsigned char val);
typedef unsigned char (*cpu_8080_in)(unsigned char port);
typedef void (*cpu_8080_out)(unsigned char port, unsigned char val);

typedef struct _cpu_8080_work {
  unsigned char a;
  unsigned char f;
  unsigned char b;
  unsigned char c;
  unsigned char d;
  unsigned char e;
  unsigned char h;
  unsigned char l;
  unsigned short pc;
  unsigned short sp;
  cpu_8080_load_8 load_8;
  cpu_8080_store_8 store_8;
  cpu_8080_in in;
  cpu_8080_out out;
  unsigned char op;
} cpu_8080_work;

void cpu_8080_reset(cpu_8080_work *);
int cpu_8080_step(cpu_8080_work *);

# if defined(__cplusplus)
};
# endif // defined(__cplusplus)

#endif // !defined(__cpu_8080_h__)
