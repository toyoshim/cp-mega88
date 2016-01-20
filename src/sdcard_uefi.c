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

#include <efi.h>

static unsigned char
buffer[512];

static unsigned short
crc = 0xffff;

static unsigned long
cur_blk = 0;

extern EFI_FILE_HANDLE efi_fs;
EFI_FILE_HANDLE fp = NULL;

void
sdcard_init
(void)
{
  fp = NULL;
}

int
sdcard_open
(void)
{
  UINT64 mode = EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE;
  EFI_STATUS status = uefi_call_wrapper(
      efi_fs->Open, 5, efi_fs, &fp, L"EFI\\cpmega88\\sdcard.img", mode, 0);
  if (!EFI_ERROR(status)) return 0;
  status = uefi_call_wrapper(
      efi_fs->Open, 5, efi_fs, &fp, L"sdcard.img", mode, 0);
  return EFI_ERROR(status) ? -1 : 0;
}

int
sdcard_fetch
(unsigned long blk_addr)
{
  if (NULL == fp) return -1;
  if (0 != (blk_addr & 0x1ff)) return -2;
  EFI_STATUS status = uefi_call_wrapper(fp->SetPosition, 2, fp, blk_addr);
  if (EFI_ERROR(status)) return -3;
  UINTN size = 512;
  status = uefi_call_wrapper(fp->Read, 3, fp, &size, buffer);
  if (EFI_ERROR(status) || 512 != size) return -4;
  cur_blk = blk_addr;
  return 0;
}

int
sdcard_store
(unsigned long blk_addr)
{
  if (NULL == fp) return -1;
  if (0 != (blk_addr & 0x1ff)) return -2;
  EFI_STATUS status = uefi_call_wrapper(fp->SetPosition, 2, fp, blk_addr);
  if (EFI_ERROR(status)) return -3;
  UINTN size = 512;
  status = uefi_call_wrapper(fp->Write, 3, fp, &size, buffer);
  if (EFI_ERROR(status) || 512 != size) return -4;
  return 0;
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
