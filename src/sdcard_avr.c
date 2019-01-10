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

#include "sdcard.h"

#include <avr/io.h>

#define PORT PORTC
#define DDR  DDRC
#define PIN  PINC

#define P_CK _BV(PINC2)
#define P_PU _BV(PINC4)
#define P_DO _BV(PINC4)

#if defined(MCU_88)
# define P_DI _BV(PINC3)
# define P_CS _BV(PINC5)
#elif defined(MCU_32U2)
# define P_DI _BV(PINC5)
# define P_CS _BV(PINC6)
#else
# error
#endif

#if defined(MCU_88)
# define PIN_HIGH(x) PORT |= (x)
# define PIN_LOW(x) PORT &= ~(x)
#elif defined(MCU_32U2)  // emulate open-drain
# define PIN_HIGH(x) DDR &= ~(x)
# define PIN_LOW(x) DDR |= (x)
#else
# error
#endif

static unsigned char
buffer[512];

static unsigned short
crc = 0xffff;

static unsigned long
cur_blk = 0;

static unsigned char
ccs = 0;

static unsigned char
sdc_version = 2;

static void
sd_out
(char c)
{
  int i;
  for (i = 7; i >= 0; i--) {
    PIN_LOW(P_CK);
    if (c & (1 << i)) PIN_HIGH(P_DI);
    else PIN_LOW(P_DI);
    PIN_HIGH(P_CK);
  }
}

static int
sd_busy
(char f)
{
  unsigned long timeout = 0xffff;
  for (; 0 != timeout; timeout--) {
    char c;
    PIN_LOW(P_CK);
    c = PIN;
    PIN_HIGH(P_CK);
    if ((f & P_DO) == (c & P_DO)) return 0;
  }
  return -1;
}

static unsigned long
sd_in
(char n)
{
  int i;
  unsigned long rc = 0;
  for (i = 0; i < n; i++) {
    char c;
    PIN_LOW(P_CK);
    c = PIN;
    PIN_HIGH(P_CK);
    rc <<= 1;
    if (c & P_DO) rc |= 1;
  }
  return rc;
}

static unsigned long
sd_cmd
(char cmd, char arg0, char arg1, char arg2, char arg3, char crc)
{
  PIN_HIGH(P_DI);
  PIN_LOW(P_CS);

  sd_out(cmd);
  sd_out(arg0);
  sd_out(arg1);
  sd_out(arg2);
  sd_out(arg3);
  sd_out(crc);
  PIN_HIGH(P_DI);
  if (sd_busy(0) < 0) return 0xffffffff;
  // The first response bit was already consumed by sd_busy.
  // Read remaining response bites.
  if (0x48 == cmd) return sd_in(39);  // R7
  if (0x7a == cmd) return sd_in(39);  // R3
  return sd_in(7);  // R1
}

void
sdcard_init
(void)
{
  /*
   * CK/DI/CS: out, low
   * DO: in
   */
  // Port Settings
  DDR |=  (P_CK | P_DI | P_CS);
  PORT &= ~(P_CK | P_DI | P_CS);

  DDR &= ~P_DO;
  PORT |= P_PU;

  cur_blk = 0;
}

int
sdcard_open
(void)
{
  // initial clock
  PIN_HIGH(P_DI | P_CS);
  int i;
  for (i = 0; i < 80; i++) {
    PIN_HIGH(P_CK);
    PIN_LOW(P_CK);
  }
  unsigned long rc;
  // cmd0 - GO_IDLE_STATE (response R1)
  rc = sd_cmd(0x40, 0x00, 0x00, 0x00, 0x00, 0x95);
  if (1 != rc) return -1;

  // cmd8 - SEND_IF_COND (response R7)
  sdc_version = 2;
  rc = sd_cmd(0x48, 0x00, 0x00, 0x01, 0x0aa, 0x87);
  if ((rc & 0x00000fff) != 0x1aa) {
    // SDC v2 initialization failed, try legacy command.
    // cmd1 - SEND_OP_COND (response R1)
    sdc_version = 1;
    for (;;) {
      rc = sd_cmd(0x41, 0x00, 0x00, 0x00, 0x00, 0x00);
      if (0 == rc) break;
      if (0xffffffff == rc) return -2;
    }
    if (0 != rc) return -3;
  } else {
    do {
      // cmd55 - APP_CMD (response R1)
      rc = sd_cmd(0x77, 0x00, 0x00, 0x00, 0x00, 0x01);
      // acmd41 - SD_SEND_OP_COND (response R1)
      rc = sd_cmd(0x69, 0x40, 0x00, 0x00, 0x00, 0x77);
    } while (0 != rc);  // no errors, no idle
    do {
      // cmd58 - READ_OCR (response R3)
      rc = sd_cmd(0x7a, 0x00, 0x00, 0x00, 0x00, 0xfd);
    } while (0 == (rc & 0x80000000));  // card powerup not completed
    ccs = (rc & 0x40000000) ? 1 : 0; // ccs bit high means SDHC card
  }
  PIN_LOW(P_CK);
  return 0;
}

int
sdcard_fetch
(unsigned long blk_addr)
{
  if (0 != ccs) blk_addr >>= 9; // SDHC cards use block addresses
  // cmd17
  unsigned long rc =
    sd_cmd(0x51, blk_addr >> 24, blk_addr >> 16, blk_addr >> 8, blk_addr, 0x00);
  if (0 != rc) return -1;
  if (sd_busy(0) < 0) return -2;
  int i;
  for (i = 0; i < 512; i++) buffer[i] = sd_in(8);
  rc = sd_in(8);
  crc = (rc << 8) | sd_in(8);

  // XXX: rc check

  PIN_LOW(P_CK);

  cur_blk = blk_addr;
  return 0;
}

int
sdcard_store
(unsigned long blk_addr)
{
  if (0 == ccs) blk_addr >>= 9; // SDHC cards use block addresses
  // cmd24
  unsigned long rc =
    sd_cmd(0x58, blk_addr >> 24, blk_addr >> 16, blk_addr >> 8, blk_addr, 0x00);
  if (0 != rc) return -1;
  sd_out(0xff); // blank 1B
  sd_out(0xfe); // Data Token
  int i;
  for (i = 0; i < 512; i++) sd_out(buffer[i]); // Data Block
  sd_out(0xff); // CRC dummy 1/2
  sd_out(0xff); // CRC dummy 2/2
  if (sd_busy(0) < 0) return -3;
  rc = sd_in(4);
  if (sd_busy(~0) < 0) return -4;

  // XXX: rc check
  // 0 0101: accepted
  // 0 1011: CRC error
  // 0 1101: write error

  PIN_LOW(P_CK);

  return rc; // test result code
}

unsigned short
sdcard_crc
(void)
{
  return crc;
}

int
sdcard_flush
(void)
{
  return sdcard_store(cur_blk);
}

void *
sdcard_buffer
(void)
{
  return buffer;
}
