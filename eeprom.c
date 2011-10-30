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

#include "eeprom.h"
#if defined(TEST)
# include <stdio.h>
#else // defined(TEST)
# include <avr/io.h>
#endif // defined(TEST)

#if defined(TEST)
FILE *eep_fp = NULL;

int
map
(void)
{
  if (NULL != eep_fp) return 0;
  eep_fp = fopen("eeprom.img", "r+");
  if (NULL == eep_fp) eep_fp = fopen("eeprom.img", "w");
  if (NULL == eep_fp) eep_fp = fopen("eeprom.img", "r");
  if (NULL == eep_fp) return -1;
  return 0;
}
#endif // defined(TEST)

void
eeprom_write
(unsigned short addr, unsigned char data)
{
#if defined(TEST)
  if (0 != map()) return;
  if (0 != fseek(eep_fp, addr, SEEK_SET)) return;
  fwrite(&data, 1, 1, eep_fp);
#else // defined(TEST)
  while (0 != (EECR & _BV(EEPE)));
  EEAR = addr;
  EEDR = data;
  EECR |= _BV(EEMPE);
  EECR |= _BV(EEPE);
#endif // defined(TEST)
}

unsigned char
eeprom_read
(unsigned short addr)
{
#if defined(TEST)
  if (0 != map()) return 0xff;
  if (0 != fseek(eep_fp, addr, SEEK_SET)) return 0xff;
  unsigned char rc;
  if (1 != fread(&rc, 1, 1, eep_fp)) return 0xff;
  return rc;
#else // defined(TEST)
  while (0 != (EECR & _BV(EEPE)));
  EEAR = addr;
  EECR |= _BV(EERE);
  return EEDR;
#endif // defined(TEST)
}

void
eeprom_write_string
(unsigned short addr, char *str)
{
  do {
    eeprom_write(addr++, *str);
  } while (0 != *str++);
}

void
eeprom_read_string
(unsigned short addr, char *str)
{
  do {
    *str = eeprom_read(addr++);
  } while (0 != *str++);
}
