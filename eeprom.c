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
#if defined(EFI)
# include <efi.h>
#elif defined(TEST) // defined(EFI)
# if !defined(UBOOT)
#  include <stdio.h>
# endif // !defined(UBOOT)
#else // defined(TEST)
# include <avr/io.h>
#endif // defined(TEST)

#if defined(TEST)

# if defined(EFI)
extern EFI_FILE_HANDLE efi_fs;
EFI_FILE_HANDLE eep_fp = NULL;
# elif defined(UBOOT)
# else // defined(TEST)
FILE *eep_fp = NULL;
# endif

int
map
(void)
{
#if defined(EFI)
  UINT64 mode = EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE;
  EFI_STATUS status = uefi_call_wrapper(
      efi_fs->Open, 5, efi_fs, &eep_fp, L"EFI\\cpmega88\\eeprom.img", mode, 0);
  if (!EFI_ERROR(status)) return 0;
  mode = EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE;
  status = uefi_call_wrapper(
      efi_fs->Open, 5, efi_fs, &eep_fp, L"eeprom.img", mode, 0);
  return EFI_ERROR(status) ? -1 : 0;
#elif defined(UBOOT)
  return 0;
#else
  if (NULL != eep_fp) return 0;
  eep_fp = fopen("eeprom.img", "r+");
  if (NULL == eep_fp) eep_fp = fopen("eeprom.img", "w");
  if (NULL == eep_fp) eep_fp = fopen("eeprom.img", "r");
  if (NULL == eep_fp) return -1;
  return 0;
#endif
}
#endif // defined(TEST)

void
eeprom_write
(unsigned short addr, unsigned char data)
{
  if (0 != map()) return;
#if defined(EFI)
  EFI_STATUS status = uefi_call_wrapper(efi_fs->SetPosition, 2, eep_fp, addr);
  if (EFI_ERROR(status)) return;
  UINTN size = 1;
  uefi_call_wrapper(eep_fp->Write, 3, eep_fp, &size, &data);
#elif defined(UBOOT)
#elif defined(TEST) // defined(EFI)
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
  if (0 != map()) return 0xff;
#if defined(EFI)
  EFI_STATUS status = uefi_call_wrapper(efi_fs->SetPosition, 2, eep_fp, addr);
  if (EFI_ERROR(status)) return 0xff;
  UINTN size = 1;
  unsigned char data;
  status = uefi_call_wrapper(eep_fp->Read, 3, eep_fp, &size, &data);
  if (EFI_ERROR(status)) return 0xff;
  return data;
#elif defined(UBOOT)
  return 0;
#elif defined(TEST) // defined(EFI)
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
