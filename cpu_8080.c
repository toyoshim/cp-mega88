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

#include "cpu_8080.h"

#define F_CY 0x01
#define F_P  0x04
#define F_AC 0x10
#define F_Z  0x40
#define F_S  0x80

static unsigned char
tbl_par[16] = {
  1, // 0000_0000
  0, // 0000_0001
  0, // 0000_0010
  1, // 0000_0011
  0, // 0000_0100
  1, // 0000_0101
  1, // 0000_0110
  0, // 0000_0111
  0, // 0000_1000
  1, // 0000_1001
  1, // 0000_1010
  0, // 0000_1011
  1, // 0000_1100
  0, // 0000_1101
  0, // 0000_1110
  1, // 0000_1111
};

static void
flag_zsp
(cpu_8080_work *work, unsigned char val)
{
  if (0 == val) work->f |= F_Z;
  work->f |= (val & F_S);
  if (1 != (tbl_par[val & 0x0f] + tbl_par[val >> 4])) work->f |= F_P;
}

static unsigned short
get_hl
(cpu_8080_work *work)
{
  return (work->h << 8) | work->l;
}

static unsigned char
inr
(cpu_8080_work *work, unsigned char val)
{
  val++;
  work->f &= F_CY;
  if (0x00 == (val & 0x0f)) work->f |= F_AC;
  flag_zsp(work, val);
  return val;
}

static unsigned char
dcr
(cpu_8080_work *work, unsigned char val)
{
  val--;
  work->f &= F_CY;
  if (0x0f == (val & 0x0f)) work->f |= F_AC;
  flag_zsp(work, val);
  return val;
}

void
dad
(cpu_8080_work *work, unsigned short val)
{
  unsigned short hl = get_hl(work);
  unsigned short r = hl + val;
  if (r < hl) work->f |= F_CY;
  else work->f &= ~F_CY;
  work->h = r >> 8;
  work->l = r & 0xff;
}

static unsigned char
add
(cpu_8080_work *work, unsigned char val)
{
  unsigned char r = work->a + val;
  work->f = 0;
  if (r < work->a) work->f |= F_CY;
  if ((r & 0xf) < (work->a & 0xf)) work->f |= F_AC;
  flag_zsp(work, r);
  return r;
}

static unsigned char
adc
(cpu_8080_work *work, unsigned char val)
{
  unsigned char r = work->a + val + (work->f & F_CY);
  unsigned char c = work->f & F_CY;
  work->f = 0;
  if (r < (work->a + c)) work->f |= F_CY;
  if ((r & 0xf) < ((work->a & 0xf) + c)) work->f |= F_AC;
  flag_zsp(work, r);
  return r;
}

static unsigned char
sub
(cpu_8080_work *work, unsigned char val)
{
  unsigned char r = work->a - val;
  work->f = 0;
  if (r > work->a) work->f |= F_CY;
  if ((r & 0xf) > (work->a & 0xf)) work->f |= F_AC;
  flag_zsp(work, r);
  return r;
}

static unsigned char
sbb
(cpu_8080_work *work, unsigned char val)
{
  unsigned char r = work->a - val - (work->f & F_CY);
  unsigned char c = work->f & F_CY;
  work->f = 0;
  if (r > (work->a - c)) work->f |= F_CY;
  if ((r & 0xf) > ((work->a & 0xf) - c)) work->f |= F_AC;
  flag_zsp(work, r);
  return r;
}

static unsigned char
ana
(cpu_8080_work *work, unsigned char val)
{
  unsigned char r = work->a & val;
  work->f = 0;
  work->f |= F_AC;
  flag_zsp(work, r);
  return r;
}

static unsigned char
xra
(cpu_8080_work *work, unsigned char val)
{
  unsigned char r = work->a ^ val;
  work->f = 0;
  flag_zsp(work, r);
  return r;
}

static unsigned char
ora
(cpu_8080_work *work, unsigned char val)
{
  unsigned char r = work->a | val;
  work->f = 0;
  flag_zsp(work, r);
  return r;
}

static void
ret
(cpu_8080_work *work)
{
  unsigned char pcl = work->load_8(work->sp++);
  unsigned char pch = work->load_8(work->sp++);
  work->pc = (pch << 8) | pcl;
}

static void
jmp
(cpu_8080_work *work)
{
  unsigned char pcl = work->load_8(work->pc++);
  unsigned char pch = work->load_8(work->pc++);
  work->pc = (pch << 8) | pcl;
}

static void
call
(cpu_8080_work *work)
{
  unsigned char pcl = work->load_8(work->pc++);
  unsigned char pch = work->load_8(work->pc++);
  work->store_8(--work->sp, work->pc >> 8);
  work->store_8(--work->sp, work->pc & 0xff);
  work->pc = (pch << 8) | pcl;
}

static void
rst
(cpu_8080_work *work, unsigned short addr)
{
  work->store_8(--work->sp, work->pc >> 8);
  work->store_8(--work->sp, work->pc & 0xff);
  work->pc = addr;
}

void
cpu_8080_reset
(cpu_8080_work *work)
{
  work->a = 0;
  work->f = 0;
  work->b = 0;
  work->c = 0;
  work->d = 0;
  work->e = 0;
  work->h = 0;
  work->l = 0;
  work->sp = 0xffff;
  work->pc = 0;
}

int
cpu_8080_step
(cpu_8080_work *work)
{
  unsigned char work_8;
  unsigned short work_16;
  work->op = work->load_8(work->pc++);
  switch (work->op) {
  case 0x00:	// NOP
    break;
  case 0x01:	// LXI B
    work->c = work->load_8(work->pc++);
    work->b = work->load_8(work->pc++);
    break;
  case 0x02:	// STAX B
    work->store_8((work->b << 8) | work->c, work->a);
    break;
  case 0x03:	// INX B
    work_16 = (work->b << 8) | work->c;
    work_16++;
    work->b = work_16 >> 8;
    work->c = work_16 & 0xff;
    break;
  case 0x04:	// INR B
    work->b = inr(work, work->b);
    break;
  case 0x05:	// DCR B
    work->b = dcr(work, work->b);
    break;
  case 0x06:	// MVI B
    work->b = work->load_8(work->pc++);
    break;
  case 0x07:	// RLC
    work_8 = work->a >> 7;
    work->f &= ~F_CY;
    work->f |= work_8;
    work->a = (work->a << 1) | work_8;
    break;
  case 0x09:	// DAD B
    dad(work, (work->b << 8) | work->c);
    break;
  case 0x0a:	// LDAX B
    work->a = work->load_8((work->b << 8) | work->c);
    break;
  case 0x0b:	// DCX B
    work_16 = (work->b << 8) | work->c;
    work_16--;
    work->b = work_16 >> 8;
    work->c = work_16 & 0xff;
    break;
  case 0x0c:	// INR C
    work->c = inr(work, work->c);
    break;
  case 0x0d:	// DCR C
    work->c = dcr(work, work->c);
    break;
  case 0x0e:	// MVI C
    work->c = work->load_8(work->pc++);
    break;
  case 0x0f:	// RRC
    work_8 = work->a & 0x01;
    work->f &= ~F_CY;
    work->f |= work_8;
    work->a = (work_8 << 7) | (work->a >> 1);
    break;
  case 0x11:	// LXI D
    work->e = work->load_8(work->pc++);
    work->d = work->load_8(work->pc++);
    break;
  case 0x12:	// STAX D
    work->store_8((work->d << 8) | work->e, work->a);
    break;
  case 0x13:	// INX D
    work_16 = (work->d << 8) | work->e;
    work_16++;
    work->d = work_16 >> 8;
    work->e = work_16 & 0xff;
    break;
  case 0x14:	// INR D
    work->d = inr(work, work->d);
    break;
  case 0x15:	// DCR D
    work->d = dcr(work, work->d);
    break;
  case 0x16:	// MVI D
    work->d = work->load_8(work->pc++);
    break;
  case 0x17:	// RAL
    work_8 = work->f & F_CY;
    work->f &= ~F_CY;
    work->f |= work->a >> 7;
    work->a = (work->a << 1) | work_8;
    break;
  case 0x19:	// DAD D
    dad(work, (work->d << 8) | work->e);
    break;
  case 0x1a:	// LDAX D
    work->a = work->load_8((work->d << 8) | work->e);
    break;
  case 0x1b:	// DCX D
    work_16 = (work->d << 8) | work->e;
    work_16--;
    work->d = work_16 >> 8;
    work->e = work_16 & 0xff;
    break;
  case 0x1c:	// INR E
    work->e = inr(work, work->e);
    break;
  case 0x1d:	// DCR E
    work->e = dcr(work, work->e);
    break;
  case 0x1e:	// MVI E
    work->e = work->load_8(work->pc++);
    break;
  case 0x1f:	// RAR
    work_8 = work->f & F_CY;
    work->f &= ~F_CY;
    work->f |= work->a & F_CY;
    work->a = (work_8 << 7) | (work->a >> 1);
    break;
  case 0x21:	// LXI H
    work->l = work->load_8(work->pc++);
    work->h = work->load_8(work->pc++);
    break;
  case 0x22:	// SHLD
    work_16 = work->load_8(work->pc++);
    work_16 |= work->load_8(work->pc++) << 8;
    work->store_8(work_16++, work->l);
    work->store_8(work_16, work->h);
    break;
  case 0x23:	// INX H
    work_16 = get_hl(work);
    work_16++;
    work->h = work_16 >> 8;
    work->l = work_16 & 0xff;
    break;
  case 0x24:	// INR H
    work->h = inr(work, work->h);
    break;
  case 0x25:	// DCR H
    work->h = dcr(work, work->h);
    break;
  case 0x26:	// MVI H
    work->h = work->load_8(work->pc++);
    break;
  case 0x27:	// DAA
    work_8 = work->a >> 4;
    work->a &= 0x0f;
    if ((work->a > 9) | (0 != (work->f & F_AC))) {
      work->a += 6;
      work->f |= F_AC;
    } else {
      work->f &= ~F_AC;
    }
    if ((work_8 > 9) | (0 != (work->f & F_CY))) {
      work_8 += 6;
      work->f |= F_CY;
    } else {
      work->f &= ~F_CY;
    }
    work->a = (work_8 << 4) + work->a;
    work->f &= ~(F_Z | F_S | F_P);
    flag_zsp(work, work->a);
    break;
  case 0x29:	// DAD H
    dad(work, get_hl(work));
    break;
  case 0x2a:	// LHLD
    work_16 = work->load_8(work->pc++);
    work_16 |= work->load_8(work->pc++) << 8;
    work->l = work->load_8(work_16++);
    work->h = work->load_8(work_16);
    break;
  case 0x2b:	// DCX H
    work_16 = get_hl(work);
    work_16--;
    work->h = work_16 >> 8;
    work->l = work_16 & 0xff;
    break;
  case 0x2c:	// INR L
    work->l = inr(work, work->l);
    break;
  case 0x2d:	// DCR L
    work->l = dcr(work, work->l);
    break;
  case 0x2e:	// MVI L
    work->l = work->load_8(work->pc++);
    break;
  case 0x2f:	// CMA
    work->a = ~work->a;
    break;
  case 0x31:	// LXI SP
    work_8 = work->load_8(work->pc++);
    work->sp = (work->load_8(work->pc++) << 8) | work_8;
    break;
  case 0x32:	// STA
    work_8 = work->load_8(work->pc++);
    work_16 = (work->load_8(work->pc++) << 8) | work_8;
    work->store_8(work_16, work->a);
    break;
  case 0x33:	// INX SP
    work->sp++;
    break;
  case 0x34:	// INR M
    work_16 = get_hl(work);
    work_8 = work->load_8(work_16);
    work_8 = inr(work, work_8);
    work->store_8(work_16, work_8);
    break;
  case 0x35:	// DCR M
    work_16 = get_hl(work);
    work_8 = work->load_8(work_16);
    work_8 = dcr(work, work_8);
    work->store_8(work_16, work_8);
    break;
  case 0x36:	// MVI M
    work_8 = work->load_8(work->pc++);
    work->store_8(get_hl(work), work_8);
    break;
  case 0x37:	// STC
    work->f |= F_CY;
    break;
  case 0x39:	// DAD SP
    dad(work, work->sp);
    break;
  case 0x3a:	// LDA
    work_8 = work->load_8(work->pc++);
    work_16 = (work->load_8(work->pc++) << 8) | work_8;
    work->a = work->load_8(work_16);
    break;
  case 0x3b:	// DCX SP
    work->sp--;
    break;
  case 0x3c:	// INR A
    work->a = inr(work, work->a);
    break;
  case 0x3d:	// DCR A
    work->a = dcr(work, work->a);
    break;
  case 0x3e:	// MVI A
    work->a = work->load_8(work->pc++);
    break;
  case 0x3f:	// CMC
    work->f ^= F_CY;
    break;
  case 0x40:	// MOV B, B
    break;
  case 0x41:	// MOV B, C
    work->b = work->c;
    break;
  case 0x42:	// MOV B, D
    work->b = work->d;
    break;
  case 0x43:	// MOV B, E
    work->b = work->e;
    break;
  case 0x44:	// MOV B, H
    work->b = work->h;
    break;
  case 0x45:	// MOV B, L
    work->b = work->l;
    break;
  case 0x46:	// MOV B, M
    work->b = work->load_8(get_hl(work));
    break;
  case 0x47:	// MOV B, A
    work->b = work->a;
    break;
  case 0x48:	// MOV C, B
    work->c = work->b;
    break;
  case 0x49:	// MOV C, C
    break;
  case 0x4a:	// MOV C, D
    work->c = work->d;
    break;
  case 0x4b:	// MOV C, E
    work->c = work->e;
    break;
  case 0x4c:	// MOV C, H
    work->c = work->h;
    break;
  case 0x4d:	// MOV C, L
    work->c = work->l;
    break;
  case 0x4e:	// MOV C, M
    work->c = work->load_8(get_hl(work));
    break;
  case 0x4f:	// MOV C, A
    work->c = work->a;
    break;
  case 0x50:	// MOV D, B
    work->d = work->b;
    break;
  case 0x51:	// MOV D, C
    work->d = work->c;
    break;
  case 0x52:	// MOV D, D
    break;
  case 0x53:	// MOV D, E
    work->d = work->e;
    break;
  case 0x54:	// MOV D, H
    work->d = work->h;
    break;
  case 0x55:	// MOV D, L
    work->d = work->l;
    break;
  case 0x56:	// MOV D, M
    work->d = work->load_8(get_hl(work));
    break;
  case 0x57:	// MOV D, A
    work->d = work->a;
    break;
  case 0x58:	// MOV E, B
    work->e = work->b;
    break;
  case 0x59:	// MOV E, C
    work->e = work->c;
    break;
  case 0x5a:	// MOV E, D
    work->e = work->d;
    break;
  case 0x5b:	// MOV E, E
    break;
  case 0x5c:	// MOV E, H
    work->e = work->h;
    break;
  case 0x5d:	// MOV E, L
    work->e = work->l;
    break;
  case 0x5e:	// MOV E, M
    work->e = work->load_8(get_hl(work));
    break;
  case 0x5f:	// MOV E, A
    work->e = work->a;
    break;
  case 0x60:	// MOV H, B
    work->h = work->b;
    break;
  case 0x61:	// MOV H, C
    work->h = work->c;
    break;
  case 0x62:	// MOV H, D
    work->h = work->d;
    break;
  case 0x63:	// MOV H, E
    work->h = work->e;
    break;
  case 0x64:	// MOV H, H
    break;
  case 0x65:	// MOV H, L
    work->h = work->l;
    break;
  case 0x66:	// MOV H, M
    work->h = work->load_8(get_hl(work));
    break;
  case 0x67:	// MOV H, A
    work->h = work->a;
    break;
  case 0x68:	// MOV L, B
    work->l = work->b;
    break;
  case 0x69:	// MOV L, C
    work->l = work->c;
    break;
  case 0x6a:	// MOV L, D
    work->l = work->d;
    break;
  case 0x6b:	// MOV L, E
    work->l = work->e;
    break;
  case 0x6c:	// MOV L, H
    work->l = work->h;
    break;
  case 0x6d:	// MOV L, L
    break;
  case 0x6e:	// MOV L, M
    work->l = work->load_8(get_hl(work));
    break;
  case 0x6f:	// MOV L, A
    work->l = work->a;
    break;
  case 0x70:	// MOV M, B
    work->store_8(get_hl(work), work->b);
    break;
  case 0x71:	// MOV M, C
    work->store_8(get_hl(work), work->c);
    break;
  case 0x72:	// MOV M, D
    work->store_8(get_hl(work), work->d);
    break;
  case 0x73:	// MOV M, E
    work->store_8(get_hl(work), work->e);
    break;
  case 0x74:	// MOV M, H
    work->store_8(get_hl(work), work->h);
    break;
  case 0x75:	// MOV M, L
    work->store_8(get_hl(work), work->l);
    break;
  case 0x76:	// HALT
    //work->pc--;
    return 1;
  case 0x77:	// MOV M, A
    work->store_8(get_hl(work), work->a);
    break;
  case 0x78:	// MOV A, B
    work->a = work->b;
    break;
  case 0x79:	// MOV A, C
    work->a = work->c;
    break;
  case 0x7a:	// MOV A, D
    work->a = work->d;
    break;
  case 0x7b:	// MOV A, E
    work->a = work->e;
    break;
  case 0x7c:	// MOV A, H
    work->a = work->h;
    break;
  case 0x7d:	// MOV A, L
    work->a = work->l;
    break;
  case 0x7e:	// MOV A, M
    work->a = work->load_8(get_hl(work));
    break;
  case 0x7f:	// MOV A, A
    break;
  case 0x80:	// ADD B
    work->a = add(work, work->b);
    break;
  case 0x81:	// ADD C
    work->a = add(work, work->c);
    break;
  case 0x82:	// ADD D
    work->a = add(work, work->d);
    break;
  case 0x83:	// ADD E
    work->a = add(work, work->e);
    break;
  case 0x84:	// ADD H
    work->a = add(work, work->h);
    break;
  case 0x85:	// ADD L
    work->a = add(work, work->l);
    break;
  case 0x86:	// ADD M
    work_8 = work->load_8(get_hl(work));
    work->a = add(work, work_8);
    break;
  case 0x87:	// ADD A
    work->a = add(work, work->a);
    break;
  case 0x88:	// ADC B
    work->a = adc(work, work->b);
    break;
  case 0x89:	// ADC C
    work->a = adc(work, work->c);
    break;
  case 0x8a:	// ADC D
    work->a = adc(work, work->d);
    break;
  case 0x8b:	// ADC E
    work->a = adc(work, work->e);
    break;
  case 0x8c:	// ADC H
    work->a = adc(work, work->h);
    break;
  case 0x8d:	// ADC L
    work->a = adc(work, work->l);
    break;
  case 0x8e:	// ADC M
    work_8 = work->load_8(get_hl(work));
    work->a = adc(work, work_8);
    break;
  case 0x8f:	// ADC A
    work->a = adc(work, work->a);
    break;
  case 0x90:	// SUB B
    work->a = sub(work, work->b);
    break;
  case 0x91:	// SUB C
    work->a = sub(work, work->c);
    break;
  case 0x92:	// SUB D
    work->a = sub(work, work->d);
    break;
  case 0x93:	// SUB E
    work->a = sub(work, work->e);
    break;
  case 0x94:	// SUB H
    work->a = sub(work, work->h);
    break;
  case 0x95:	// SUB L
    work->a = sub(work, work->l);
    break;
  case 0x96:	// SUB M
    work_8 = work->load_8(get_hl(work));
    work->a = sub(work, work_8);
    break;
  case 0x97:	// SUB A
    work->a = sub(work, work->a);
    break;
  case 0x98:	// SBB B
    work->a = sbb(work, work->b);
    break;
  case 0x99:	// SBB C
    work->a = sbb(work, work->c);
    break;
  case 0x9a:	// SBB D
    work->a = sbb(work, work->d);
    break;
  case 0x9b:	// SBB E
    work->a = sbb(work, work->e);
    break;
  case 0x9c:	// SBB H
    work->a = sbb(work, work->h);
    break;
  case 0x9d:	// SBB L
    work->a = sbb(work, work->l);
    break;
  case 0x9e:	// SBB M
    work_8 = work->load_8(get_hl(work));
    work->a = sbb(work, work_8);
    break;
  case 0x9f:	// SBB A
    work->a = sbb(work, work->a);
    break;
  case 0xa0:	// ANA B
    work->a = ana(work, work->b);
    break;
  case 0xa1:	// ANA C
    work->a = ana(work, work->c);
    break;
  case 0xa2:	// ANA D
    work->a = ana(work, work->d);
    break;
  case 0xa3:	// ANA E
    work->a = ana(work, work->e);
    break;
  case 0xa4:	// ANA H
    work->a = ana(work, work->h);
    break;
  case 0xa5:	// ANA L
    work->a = ana(work, work->l);
    break;
  case 0xa6:	// ANA M
    work_8 = work->load_8(get_hl(work));
    work->a = ana(work, work_8);
    break;
  case 0xa7:	// ANA A
    work->a = ana(work, work->a);
    break;
  case 0xa8:	// XRA B
    work->a = xra(work, work->b);
    break;
  case 0xa9:	// XRA C
    work->a = xra(work, work->c);
    break;
  case 0xaa:	// XRA D
    work->a = xra(work, work->d);
    break;
  case 0xab:	// XRA E
    work->a = xra(work, work->e);
    break;
  case 0xac:	// XRA H
    work->a = xra(work, work->h);
    break;
  case 0xad:	// XRA L
    work->a = xra(work, work->l);
    break;
  case 0xae:	// XRA M
    work_8 = work->load_8(get_hl(work));
    work->a = xra(work, work_8);
    break;
  case 0xaf:	// XRA A
    work->a = xra(work, work->a);
    break;
  case 0xb0:	// ORA B
    work->a = ora(work, work->b);
    break;
  case 0xb1:	// ORA C
    work->a = ora(work, work->c);
    break;
  case 0xb2:	// ORA D
    work->a = ora(work, work->d);
    break;
  case 0xb3:	// ORA E
    work->a = ora(work, work->e);
    break;
  case 0xb4:	// ORA H
    work->a = ora(work, work->h);
    break;
  case 0xb5:	// ORA L
    work->a = ora(work, work->l);
    break;
  case 0xb6:	// ORA M
    work_8 = work->load_8(get_hl(work));
    work->a = ora(work, work_8);
    break;
  case 0xb7:	// ORA A
    work->a = ora(work, work->a);
    break;
  case 0xb8:	// CMP B
    sub(work, work->b);
    break;
  case 0xb9:	// CMP C
    sub(work, work->c);
    break;
  case 0xba:	// CMP D
    sub(work, work->d);
    break;
  case 0xbb:	// CMP E
    sub(work, work->e);
    break;
  case 0xbc:	// CMP H
    sub(work, work->h);
    break;
  case 0xbd:	// CMP L
    sub(work, work->l);
    break;
  case 0xbe:	// CMP M
    work_8 = work->load_8(get_hl(work));
    sub(work, work_8);
    break;
  case 0xbf:	// CMP A
    sub(work, work->a);
    break;
  case 0xc0:	// RNZ
    if (0 == (work->f & F_Z)) ret(work);
    break;
  case 0xc1:	// POP B
    work->c = work->load_8(work->sp++);
    work->b = work->load_8(work->sp++);
    break;
  case 0xc2:	// JNZ
    if (0 == (work->f & F_Z)) jmp(work);
    else work->pc += 2;
    break;
  case 0xc3:	// JMP
    jmp(work);
    break;
  case 0xc4:	// CNZ
    if (0 == (work->f & F_Z)) call(work);
    else work->pc += 2;
    break;
  case 0xc5:	// PUSH B
    work->store_8(--work->sp, work->b);
    work->store_8(--work->sp, work->c);
    break;
  case 0xc6:	// ADI
    work_8 = work->load_8(work->pc++);
    work->a = add(work, work_8);
    break;
  case 0xc7:	// RST 00H
    rst(work, 0x0000);
    break;
  case 0xc8:	// RZ
    if (0 != (work->f & F_Z)) ret(work);
    break;
  case 0xc9:	// RET
    ret(work);
    break;
  case 0xca:	// JZ
    if (0 != (work->f & F_Z)) jmp(work);
    else work->pc += 2;
    break;
  case 0xcc:	// CZ
    if (0 != (work->f & F_Z)) call(work);
    else work->pc += 2;
    break;
  case 0xcd:	// CALL
    call(work);
    break;
  case 0xce:	// ACI
    work_8 = work->load_8(work->pc++);
    work->a = adc(work, work_8);
    break;
  case 0xcf:	// RST 08H
    rst(work, 0x0008);
    break;
  case 0xd0:	// RNC
    if (0 == (work->f & F_CY)) ret(work);
    break;
  case 0xd1:	// POP D
    work->e = work->load_8(work->sp++);
    work->d = work->load_8(work->sp++);
    break;
  case 0xd2:	// JNC
    if (0 == (work->f & F_CY)) jmp(work);
    else work->pc += 2;
    break;
  case 0xd3:	// OUT
    work_8 = work->load_8(work->pc++);
    work->out(work_8, work->a);
    break;
  case 0xd4:	// CNC
    if (0 == (work->f & F_CY)) call(work);
    else work->pc += 2;
    break;
  case 0xd5:	// PUSH D
    work->store_8(--work->sp, work->d);
    work->store_8(--work->sp, work->e);
    break;
  case 0xd6:	// SUI
    work_8 = work->load_8(work->pc++);
    work->a = sub(work, work_8);
    break;
  case 0xd7:	// RST 10H
    rst(work, 0x0010);
    break;
  case 0xd8:	// RC
    if (0 != (work->f & F_CY)) ret(work);
    break;
  case 0xda:	// JC
    if (0 != (work->f & F_CY)) jmp(work);
    else work->pc += 2;
    break;
  case 0xdb:	// IN
    work_8 = work->load_8(work->pc++);
    work->a = work->in(work_8);
    break;
  case 0xdc:	// CC
    if (0 != (work->f & F_CY)) call(work);
    else work->pc += 2;
    break;
  case 0xde:	// SBI
    work_8 = work->load_8(work->pc++);
    work->a = sbb(work, work_8);
    break;
  case 0xdf:	// RST 18H
    rst(work, 0x0018);
    break;
  case 0xe0:	// RPO
    if (0 == (work->f & F_P)) ret(work);
    break;
  case 0xe1:	// POP H
    work->l = work->load_8(work->sp++);
    work->h = work->load_8(work->sp++);
    break;
  case 0xe2:	// JPO
    if (0 == (work->f & F_P)) jmp(work);
    else work->pc += 2;
    break;
  case 0xe3:	// XTHL
    work_8 = work->load_8(work->sp);
    work->store_8(work->sp, work->l);
    work->l = work_8;
    work_8 = work->load_8(work->sp + 1);
    work->store_8(work->sp + 1, work->h);
    work->h = work_8;
    break;
  case 0xe4:	// CPO
    if (0 == (work->f & F_P)) call(work);
    else work->pc += 2;
    break;
  case 0xe5:	// PUSH H
    work->store_8(--work->sp, work->h);
    work->store_8(--work->sp, work->l);
    break;
  case 0xe6:	// ANI
    work_8 = work->load_8(work->pc++);
    work->a = ana(work, work_8);
    break;
  case 0xe7:	// RST 20H
    rst(work, 0x0020);
    break;
  case 0xe8:	// RPE
    if (0 != (work->f & F_P)) ret(work);
    break;
  case 0xe9:	// PCHL
    work->pc = get_hl(work);
    break;
  case 0xea:	// JPE
    if (0 != (work->f & F_P)) jmp(work);
    else work->pc += 2;
    break;
  case 0xeb:	// XCHG
    work_16 = get_hl(work);
    work->h = work->d;
    work->l = work->e;
    work->d = work_16 >> 8;
    work->e = work_16 & 0xff;
    break;
  case 0xec:	// CPE
    if (0 != (work->f & F_P)) call(work);
    else work->pc += 2;
    break;
  case 0xee:	// XRI
    work_8 = work->load_8(work->pc++);
    work->a = xra(work, work_8);
    break;
  case 0xef:	// RST 28H
    rst(work, 0x0028);
    break;
  case 0xf0:	// RP
    if (0 == (work->f & F_S)) ret(work);
    break;
  case 0xf1:	// POP PSW
    work->f = work->load_8(work->sp++);
    work->a = work->load_8(work->sp++);
    break;
  case 0xf2:	// JP
    if (0 == (work->f & F_S)) jmp(work);
    else work->pc += 2;
    break;
  case 0xf3:	// DI
    break;
  case 0xf4:	// CP
    if (0 == (work->f & F_S)) call(work);
    else work->pc += 2;
    break;
  case 0xf5:	// PUSH PSW
    work->store_8(--work->sp, work->a);
    work->store_8(--work->sp, work->f | 0x02);
    break;
  case 0xf6:	// ORI
    work_8 = work->load_8(work->pc++);
    work->a = ora(work, work_8);
    break;
  case 0xf7:	// RST 30H
    rst(work, 0x0030);
    break;
  case 0xf8:	// RM
    if (0 != (work->f & F_S)) ret(work);
    break;
  case 0xf9:	// SPHL
    work->sp = get_hl(work);
    break;
  case 0xfa:	// JM
    if (0 != (work->f & F_S)) jmp(work);
    else work->pc += 2;
    break;
  case 0xfb:	// EI
    break;
  case 0xfc:	// CM
    if (0 != (work->f & F_S)) call(work);
    else work->pc += 2;
    break;
  case 0xfe:	// CPI
    work_8 = work->load_8(work->pc++);
    sub(work, work_8);
    break;
  case 0xff:	// RST 38H
    rst(work, 0x0038);
    break;
  default:	// Illegal Op.
    return ((-1) & ~0xff) | work->op;
  }
  return 0;
}
