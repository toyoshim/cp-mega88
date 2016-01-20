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

#include "platform.h"

#include <efi/efi.h>
#include <efi/efilib.h>

#include "machine.h"

EFI_HANDLE *efi_image;
EFI_SYSTEM_TABLE *efi_systab;
EFI_FILE_HANDLE efi_fs;

#if !defined(EFI_CONSOLE_CONTROL_PROTOCOL_GUID)
// Provides minimum definitions that are sufficient for our use case.
# define EFI_CONSOLE_CONTROL_PROTOCOL_GUID \
  { 0xf42f7782, 0x12e, 0x4c12, {0x99, 0x56, 0x49, 0xf9, 0x43, 0x4, 0xf7, 0x21} }
EFI_GUID ConsoleControlProtocol = EFI_CONSOLE_CONTROL_PROTOCOL_GUID;
typedef struct _EFI_CONSOLE_CONTROL_INTERFACE EFI_CONSOLE_CONTROL_INTERFACE;
typedef enum { EfiConsoleControlScreenText } EFI_CONSOLE_CONTROL_SCREEN_MODE;
typedef EFI_STATUS
(EFIAPI *EFI_CONSOLE_CONTROL_PROTOCOL_SET_MODE) (
  IN  EFI_CONSOLE_CONTROL_INTERFACE   *This,
  IN  EFI_CONSOLE_CONTROL_SCREEN_MODE Mode
  );
struct _EFI_CONSOLE_CONTROL_INTERFACE {
  VOID *GetMode;
  EFI_CONSOLE_CONTROL_PROTOCOL_SET_MODE SetMode;
  VOID *LockStdIn;
};
#endif

EFI_STATUS
efi_main
(EFI_HANDLE *image, EFI_SYSTEM_TABLE *systab)
{
  efi_image = image;
  efi_systab = systab;
  EFI_FILE_IO_INTERFACE *efi_fio;
  EFI_GUID LoadedImageProtocol = LOADED_IMAGE_PROTOCOL;
  EFI_LOADED_IMAGE *efi_li;
  EFI_CONSOLE_CONTROL_INTERFACE *efi_cc;
  EFI_STATUS status = uefi_call_wrapper(
      efi_systab->BootServices->HandleProtocol, 3,
      image, &LoadedImageProtocol, (void**)&efi_li);
  if (EFI_ERROR(status)) return 0;
  status = uefi_call_wrapper(
      efi_systab->BootServices->HandleProtocol, 3,
      efi_li->DeviceHandle, &FileSystemProtocol, (void**)&efi_fio);
  if (EFI_ERROR(status)) return 0;
  status = uefi_call_wrapper(efi_fio->OpenVolume, 2, efi_fio, &efi_fs);
  if (EFI_ERROR(status)) return 0;
  status = uefi_call_wrapper(
      efi_systab->BootServices->LocateProtocol, 3,
      &ConsoleControlProtocol, NULL, (void**)&efi_cc);
  if (!EFI_ERROR(status)) {
    // Switch console mode from graphics to text on Mac.
    uefi_call_wrapper(efi_cc->SetMode, 2, efi_cc, EfiConsoleControlScreenText);
  }
  machine_boot();
  return 0;
}

void
platform_reset
(void)
{
  uefi_call_wrapper(efi_systab->BootServices->Exit, 3,
                    efi_image, EFI_SUCCESS, 0, NULL);
}
