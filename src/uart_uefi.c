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

#include <efi.h>
#include <wchar.h>

extern EFI_SYSTEM_TABLE* efi_systab;

void
uart_init
(void)
{
}

void
uart_putchar
(unsigned char c)
{
  wchar_t s[2];
  s[0] = c;
  s[1] = 0;
  uefi_call_wrapper(efi_systab->ConOut->OutputString, 2, efi_systab->ConOut, s);
}

int
uart_getchar
(void)
{
  EFI_EVENT event[1];
  UINTN index;
  EFI_INPUT_KEY key;
  event[0] = efi_systab->ConIn->WaitForKey;
  uefi_call_wrapper(
      efi_systab->BootServices->WaitForEvent, 3, 1, event, &index);
  uefi_call_wrapper(
      efi_systab->ConIn->ReadKeyStroke, 2, efi_systab->ConIn, &key);
  return key.UnicodeChar;
}

int
uart_peek
(void)
{
  EFI_EVENT event[2];
  UINTN index;
  event[0] = efi_systab->ConIn->WaitForKey;
  EFI_STATUS status = uefi_call_wrapper(
      efi_systab->BootServices->CreateEvent, 5,
      EVT_TIMER, 0, NULL, NULL, &event[1]);
  if (EFI_ERROR(status)) return 0;
  status = uefi_call_wrapper(
      efi_systab->BootServices->SetTimer, 3, event[1], TimerRelative, 0);
  if (!EFI_ERROR(status)) {
    status = uefi_call_wrapper(
        efi_systab->BootServices->WaitForEvent, 3, 2, event, &index);
  }
  uefi_call_wrapper(
      efi_systab->BootServices->CloseEvent, 1, event[1]);
  if (EFI_ERROR(status) || index == 1) return 0;
  return 1;
}
