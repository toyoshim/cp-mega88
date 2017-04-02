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

#if !defined(TEST)
# include <avr/io.h>
#endif // !defined(TEST)
#if defined(EFI)
# include <efi/efi.h>
# include <efi/efilib.h>
#endif // defined(EFI)
#include "uart.h"
#include "sram.h"
#include "sdcard.h"
#include "fat.h"
#include "eeprom.h"
#include "config.h"
#include "platform.h"

#if defined(CPU_EMU_C)
# include "cpu_8080.h"
#else // if defined(CPU_EMU_A)
# include "i8080.h"
#endif // defined(CPU_EMU_C)

static char wr_prt = 0;
#if defined(EFI)
static char vt_cnv = 2;
#else
static char vt_cnv = 1;
#endif
static char sd_fat = 0;

#if defined(CPU_EMU_C)
static cpu_8080_work work;
#endif // defined(CPU_EMU_C)

#if defined(EFI)
extern EFI_HANDLE *efi_image;
extern EFI_SYSTEM_TABLE *efi_systab;
#endif // defined(EFI)

#if !defined(TEST)
extern uint8_t _end;
extern uint8_t __stack;
#endif // defined(TEST)

void
disk_read
(unsigned long blk)
{
  if (0 == sd_fat) sdcard_fetch(blk);
#if defined(USE_FAT)
  else {
    fat_seek(blk);
    fat_read();
  }
#endif // defined(USE_FAT)
}

void
boot
(void)
{
#if defined(CPU_EMU_C)
  cpu_8080_reset(&work);
#else // if defined(CPU_EMU_A)
  i8080_reset();
#endif // defined(CPU_EMU_C)
  int i, j;
  for (i = 0; i < 13; i++) {
    disk_read(i << 9);
    unsigned short addr = 0xe380 + i * 512;
    for (j = 0; j < 512; j++) sram_write(addr + j, sdcard_read(j));
  }
  for (i = 0; i < 0x100; i++) sram_write(i, 0);
  sram_write(0x0005, 0xc3);
  sram_write(0x0006, 0x06);
  sram_write(0x0007, 0x3c + 0xb0);
#if defined(CPU_EMU_C)
  do {
# if defined(CPM_DEBUG)
    unsigned char op = sram_read(work.pc);
    uart_puts("pc: ");
    uart_puthex(work.pc >> 8);
    uart_puthex(work.pc);
    uart_puts(" / ");
    uart_puthex(work.a);
    uart_puthex(work.f);
    uart_puthex(work.b);
    uart_puthex(work.c);
    uart_puthex(work.d);
    uart_puthex(work.e);
    uart_puthex(work.h);
    uart_puthex(work.l);
    uart_puthex(work.pc >> 8);
    uart_puthex(work.pc);
    uart_puthex(work.sp >> 8);
    uart_puthex(work.sp);
    uart_puts(" (");
    uart_puthex(op);
    uart_putsln(")");
    cpu_8080_step(&work);
  } while (1);
# else // defined(CPM_DEBUG)
  } while (0 == cpu_8080_step(&work));
# endif // defined(CPM_DEBUG)
#else // if defined(CPU_EMU_A)
# if defined(CPM_DEBUG)
  uart_putsln("            A F B C D E H LPhPlShSl");
  char r = 0;
# endif // defined(CPM_DEBUG)
  do {
# if defined(CPM_DEBUG)
    unsigned char op = sram_read((i8080_work.reg_pc_h << 8) | i8080_work.reg_pc_l);
    uart_puts("pc: ");
    uart_puthex(i8080_work.reg_pc_h);
    uart_puthex(i8080_work.reg_pc_l);
    uart_puts(" / ");
    uart_puthex(i8080_work.reg_a);
    uart_puthex(i8080_work.reg_f);
    uart_puthex(i8080_work.reg_b);
    uart_puthex(i8080_work.reg_c);
    uart_puthex(i8080_work.reg_d);
    uart_puthex(i8080_work.reg_e);
    uart_puthex(i8080_work.reg_h);
    uart_puthex(i8080_work.reg_l);
    uart_puthex(i8080_work.reg_pc_h);
    uart_puthex(i8080_work.reg_pc_l);
    uart_puthex(i8080_work.reg_sp_h);
    uart_puthex(i8080_work.reg_sp_l);
    uart_puts(" (");
    uart_puthex(op);
    uart_putsln(")");
    //if (0 != r) break;
    r = i8080_run();
  } while (1);
# else // defined(CPM_DEBUG)
  } while (0 == i8080_run());
# endif // defined(CPM_DEBUG)
#endif // defined(CPU_EMU_C)
#if !defined(MSG_MIN)
  uart_putsln("quit vm");
#if defined(CPU_EMU_C)
  unsigned char op = sram_read(work.pc);
  uart_puts("pc: ");
  uart_puthex(work.pc >> 8);
  uart_puthex(work.pc);
  uart_puts(" (");
  uart_puthex(op);
  uart_putsln(")");
#endif // defined(CPU_EMU_C)
#endif // !defined(MSG_MIN)
#if !defined(MONITOR)
  uart_putsln("press any key to reboot");
  while (-1 == uart_getchar());
  platform_reset();
#endif // !defined(MONITOR)
}

#if defined(MONITOR)
int
strdcmp
(char *s1, char *s2, char s)
{
  for (;;) {
    if (((0 == *s1) || (s == *s1)) && 
        ((0 == *s2) || (s == *s2))) return 0;
    if (*s1++ != *s2++) return -1;
  }
}

#if defined(MON_MEM) | defined(MON_SDC)
int
strndcmp
(char *s1, char *s2, int n, char s)
{
  int i;
  for (i = 0; i < n; i++) {
    if (((0 == *s1) || (s == *s1)) ||
        ((0 == *s2) || (s == *s2))) return -1;
    if (*s1++ != *s2++) return -1;
  }
  return 0;
}

int
getnum
(char *s)
{
  int rc = 0;
  if (0 == strndcmp(s, "0x", 2, 0)) {
    s += 2;
    for (;;) {
      if (0 == *s) return rc;
      rc *= 16;
      if (('0' <= *s) && (*s <= '9')) rc += (*s - '0');
      else if (('a' <= *s) && (*s <= 'f')) rc += (*s - 'a' + 10);
      else if (('A' <= *s) && (*s <= 'F')) rc += (*s - 'A' + 10);
      else return rc / 16;
      s++;
    }
  } else {
    for (;;) {
      if (0 == *s) return rc;
      rc *= 10;
      if (('0' <= *s) && (*s <= '9')) rc += (*s - '0');
      else return rc / 10;
      s++;
    }
  }
  return rc;
}
#endif // defined(MON_MEM) | defined(MON_SDC)

#if defined(USE_FAT)
void
mount
(char *name)
{
  uart_puts("A: ");
  uart_puts(name);
  fat_rewind();
  for (;;) {
    if (fat_next() < 0) break;
    char buf[8 + 1 + 3 + 1];
    //char attr = fat_attr();
    //if (0 != (0x10 & attr)) continue;
    fat_name(buf);
    if (0 != strdcmp(name, buf, 0)) continue;
    fat_open();
    uart_putsln(" ok");
    sd_fat = 1;
    eeprom_write(16, 0);
    eeprom_write_string(17, name);
    eeprom_write(16, 0x88);
    return;
  }
  uart_putsln(" err");
  sd_fat = 0;
  return;
}
#endif // defined(USE_FAT)

char *
split
(char *s, char c)
{
  for (; 0 != *s; s++) if (c == *s) return ++s;
  return NULL;
}

void
prompt
(void)
{
  char buffer[MAX_PROMPT + 1];
  unsigned char size = 0;
  uart_puts("CP/Mega88>");
  for (;;) {
    int c;
    do {
      c = uart_getchar();
    } while (c < 0);
    if ((0x08 == c) || (0x7f == c)) {
      if (0 != size) {
        size--;
        uart_putchar(0x08);
        uart_putchar(0x20);
        uart_putchar(0x08);
      }
    } else if (0x0a == c) {
    } else if (0x0d == c) {
      uart_putsln("");
      break;
    } else if ((0 == size) && (0x10 == c)) {
      char *p;
      for (p = buffer; 0 != *p; p++) size++;
      uart_puts(buffer);
    } else {
      if (MAX_PROMPT != size) {
        buffer[size++] = c;
        uart_putchar(c);
      }
    }
  }
  buffer[size] = 0;
  char *cmd = buffer;
  char *arg = split(cmd, ' ');
  if (0 == *cmd) {
    return;
  } else if (0 == strdcmp("r", cmd, 0)) {
    platform_reset();
  } else if (0 == strdcmp("b", cmd, 0)) {
    boot();
    return;
  } else if (0 == strdcmp("a", cmd, ' ')) {
    if (NULL == arg) goto usage;
    if (0 == strdcmp("on", arg, 0)) {
      uart_putsln(" on");
      eeprom_write(8, 0x88);
    } else {
      uart_putsln(" off");
      eeprom_write(8, 0);
    }
    return;
  } else if (0 == strdcmp("wp", cmd, ' ')) {
    if (NULL == arg) goto usage;
#if !defined(MSG_MIN)
    uart_puts("<write protection");
#endif // !defined(MSG_MIN)
    if (0 == strdcmp("on", arg, 0)) {
      uart_puts(" on");
      wr_prt = 1;
    } else {
      uart_puts(" off");
      wr_prt = 0;
    }
#if defined(MSG_MIN)
    uart_putsln("");
#else // defined(MSG_MIN)
    uart_putsln(">");
#endif // defined(MSG_MIN)
    return;
#if defined(MON_MEM)
  } else if (0 == strdcmp("mr", cmd, ' ')) {
    if (NULL == arg) goto usage;
    unsigned short n;
    n = getnum(arg);
# if !defined(MSG_MIN)
    uart_puts("<memory read 0x");
    uart_puthex(n >> 8);
    uart_puthex(n);
    uart_putsln("> ");
# endif // !defined(MSG_MIN)
    unsigned char c = sram_read(n);
    uart_puthex(c);
    uart_putsln("");
    return;
  } else if (0 == strdcmp("mw", cmd, ' ')) {
    if (NULL == arg) goto usage;
    char *addr = arg;
    char *data = split(addr, ',');
    if (NULL == data) goto usage;
    unsigned short n_addr;
    unsigned short n_data;
    n_addr = getnum(addr);
    n_data = getnum(data);
# if !defined(MSG_MIN)
    uart_puts("<memory write 0x");
    uart_puthex(n_addr >> 8);
    uart_puthex(n_addr);
    uart_puts(",0x");
    uart_puthex(n_data);
    uart_putsln(">");
# endif // !defined(MSG_MIN)
    sram_write(n_addr, n_data);
    return;
#endif // defined(MON_MEM)
#if defined (MON_SDC)
  } else if (0 == strdcmp("so", cmd, 0)) {
# if !defined(MSG_MIN)
    uart_putsln("<sdcard open>");
# endif // !defined(MSG_MIN)
    char rc = sdcard_open();
    if (rc >= 0) uart_putsln(" detect");
    else {
      uart_puts(" not detect(");
      uart_puthex(-rc);
      uart_putsln(")");
    }
    return;
  } else if (0 == strdcmp("sd", cmd, 0)) {
# if !defined(MSG_MIN)
    uart_putsln("<sdcard dump block buffer>");
# endif // !defined(MSG_MIN)
    uart_puts(" ");
    int i;
    for (i = 0; i < 512; i++) {
      char buf[17];
      if (0 == (i & 3)) uart_putchar('_');
      unsigned char rc = sdcard_read(i);
      uart_puthex(rc);
      if ((rc < '!') || ('~' < rc)) rc = ' ';
      buf[i & 15] = rc;
      if (15 == (i & 15)) {
        buf[16] = 0;
        uart_puts(": ");
        uart_puts(buf);
        uart_putsln(" ");
      }
    }
    uart_puts("crc: ");
    unsigned short crc = sdcard_crc();
    uart_puthex(crc >> 8);
    uart_puthex(crc);
    uart_putsln("");
    return;
  } else if (0 == strdcmp("sf", cmd, ' ')) {
    if (NULL == arg) goto usage;
    unsigned long addr = getnum(arg);
# if !defined(MSG_MIN)
    uart_puts("<sdcard fetch block 0x");
    uart_puthex(addr >> 8);
    uart_puthex(addr);
    uart_putsln(">");
# endif // !defined(MSG_MIN)
    char rc = sdcard_fetch(addr << 9);
    if (rc >= 0) uart_putsln(" ok");
    else {
      uart_puts(" error(");
      uart_puthex(-rc);
      uart_putsln(")");
    }
    return;
  } else if (0 == strdcmp("ss", cmd, ' ')) {
    if (NULL == arg) goto usage;
    unsigned long addr = getnum(arg);
# if !defined(MSG_MIN)
    uart_puts("<sdcard store block 0x");
    uart_puthex(addr >> 8);
    uart_puthex(addr);
    uart_putsln(">");
# endif // !defined(MSG_MIN)
    char rc = sdcard_store(addr << 9);
    if (rc >= 0) uart_putsln(" ok");
    else {
      uart_puts(" error(");
      uart_puthex(-rc);
      uart_putsln(")");
    }
    return;
#endif // defined(MON_SDC)
#if defined(MON_FAT)
  } else if (0 == strdcmp("ls", cmd, 0)) {
    fat_rewind();
    for (;;) {
      if (fat_next() < 0) break;
      char name[8 + 1 + 3 + 1];
      char attr = fat_attr();
      fat_name(name);
      uart_putchar(' ');
      uart_putchar((0 != (0x10 & attr))? 'd': '-');
# if !defined(MSG_MIN)
      uart_putchar((0 == (0x04 & attr))? 'r': '-');
      uart_putchar((0 == (0x01 & attr))? 'w': '-');
      uart_putchar((0 != (0x10 & attr))? 'x': '-');
# endif // !defined(MSG_MIN)
      uart_putchar(' ');
      uart_puts(name);
      uart_putsln("");
    }
    return;
  } else if (0 == strdcmp("cd", cmd, ' ')) {
    if (NULL == arg) {
      fat_init();
      return;
    }
    fat_rewind();
    for (;;) {
      if (fat_next() < 0) break;
      char name[8 + 1 + 3 + 1];
      char attr = fat_attr();
      if (0 == (0x10 & attr)) continue;
      fat_name(name);
      if (0 != strdcmp(arg, name, 0)) continue;
      fat_chdir();
      break;
    }
    return;
  } else if (0 == strdcmp("m", cmd, ' ')) {
    if (NULL == arg) goto usage;
    mount(arg);
    return;
#endif // defined(MON_FAT)
#if defined(MON_CON)
  } else if (0 == strdcmp("vt", cmd, ' ')) {
    if (NULL == arg) goto usage;
# if !defined(MSG_MIN)
    uart_puts("<vt100 compatible mode");
# endif // !defined(MSG_MIN)
    if (0 == strdcmp("on", arg, 0)) {
      uart_puts(" on");
      vt_cnv = 1;
    } else {
      uart_puts(" off");
      vt_cnv = 0;
    }
# if defined(MSG_MIN)
    uart_putsln("");
# else // defined(MSG_MIN)
    uart_putsln(">");
# endif // defined(MSG_MIN)
    return;
# if defined(EFI)
  } else if (0 == strdcmp("efi", cmd, ' ')) {
    if (NULL == arg) goto usage;
#  if !defined(MSG_MIN)
    uart_puts("<EFI terminal mode");
#  endif // !defined(MSG_MIN)
    if (0 == strdcmp("on", arg, 0)) {
      uart_puts(" on");
      vt_cnv = 2;
    } else {
      uart_puts(" off");
      vt_cnv = 0;
    }
#  if defined(MSG_MIN)
    uart_putsln("");
#  else // defined(MSG_MIN)
    uart_putsln(">");
#  endif // defined(MSG_MIN)
    return;
# endif // defined(EFI)
#endif // defined(MON_CON)
  }
 usage:
#if defined(MON_HELP)
# if !defined(MSG_MIN)
  uart_putsln("monitor commands");
  uart_putsln(" r                : reset");
  uart_putsln(" b                : boot CP/M 2.2");
  uart_putsln(" wp <on/off>      : file system write protection");
  uart_putsln(" a <on/off>       : auto boot");
#  if defined(MON_MEM)
  uart_putsln(" mr <addr>        : memory read from <addr>");
  uart_putsln(" mw <addr>,<data> : memory write <data> to <addr>");
#  endif // defined(MON_MEM)
#  if defined(MON_SDC)
  uart_putsln(" so               : sdcard open");
  uart_putsln(" sd               : sdcard dump block buffer");
  uart_putsln(" sf <addr>        : sdcard fetch block");
  uart_putsln(" ss <addr>        : sdcard store block");
#  endif // defined(MON_SDC)
#  if defined(MON_FAT)
  uart_putsln(" ls               : file listing");
  uart_putsln(" cd               : change directory");
  uart_putsln(" m <filename>     : mount image disk");
#  endif // defined(MON_FAT)
#  if defined(MON_CON)
  uart_putsln(" vt <on/off>      : vt100 compatible mode");
#   if defined(EFI)
  uart_putsln(" efi <on/off>     : EFI terminal mode");
#   endif // defined(EFI)
#  endif // defined(MON_CON)
# else // !defined(MSG_MIN)
  uart_puts("  CMD R;B;WP t;A t");
#  if defined(MON_MEM)
  uart_puts(";MR a;MW a,d");
#  endif // defined(MON_MEM)
#  if defined(MON_SDC)
  uart_puts(";SO;SD;SF a;SS a");
#  endif // defined(MON_SDC)
#  if defined(MON_FAT)
  uart_puts(";LS;CD s;M s");
#  endif // defined(MON_FAT)
#  if defined(MON_CON)
  uart_puts(";VT t");
#  endif // defined(MON_CON)
  uart_putsln("");
# endif // !defined(MSG_MIN)
#endif // defined(MON_HELP)
  return;
}
#endif // defined(MONITOR)

#if defined(CLR_MEM)
void
mem_clr
(void)
{
  unsigned short addr;
  addr = 0;
  do {
    sram_write(addr++, 0);
  } while (0 != addr);
}
#endif // defined(CLR_MEM)

#if defined(MON_MEM) | defined(CHK_MEM)
void
mem_chk
(void)
{
  unsigned char test;
  unsigned short addr = 0;
  for (test = 0; test < 2; addr++) {
    unsigned char c;
    unsigned char err = 0;
    if (0 == test) {
      sram_write(addr, 0xaa);
      c = sram_read(addr);
      if (0xaa != c) err |= 1;
#if !defined(CHK_MIN)
      sram_write(addr, 0x55);
      c = sram_read(addr);
      if (0x55 != c) err |= 2;
#endif // !defined(CHK_MIN)
      if (0 == (addr & 1)) sram_write(addr, addr);
      else sram_write(addr, addr >> 8);
#if !defined(CHK_MIN)
      c = sram_read(addr);
      if ((0 == (addr & 1)) & ((addr & 0xff) != c)) err |= 4;
      if ((0 != (addr & 1)) & ((addr >> 8) != c)) err |= 4;
#endif // !defined(CHK_MIN)
    } else {
      c = sram_read(addr);
      if ((0 == (addr & 1)) & ((addr & 0xff) != c)) err |= 4;
      if ((0 != (addr & 1)) & ((addr >> 8) != c)) err |= 4;
    }
    if (0 != err) {
#if defined(CHK_MIN)
      uart_puts("ERR: ");
#else // defined(CHK_MIN)
      if (0 == test) uart_puts("write failed at 0x");
      else uart_puts("address failed at 0x");
#endif // defined(CHK_MIN)
      uart_puthex(addr >> 8);
      uart_puthex(addr);
#if defined(CHK_MIN)
      uart_putsln("");
#else // defined(CHK_MIN)
      uart_puts(" (");
      if (0 == test) uart_puthex(err);
      else uart_puthex(c);
      uart_putsln(")");
#endif // defined(CHK_MIN)
      return;
    }
    if (0xfff == (addr & 0xfff)) {
#if defined(MSG_MIN)
# if defined(CHK_MIN)
      if (0 != test) uart_puts("MEM: ");
# else // defined(CHK_MIN)
      if (0 == test) uart_puts("mem wrt: ");
      else uart_puts("mem adr: ");
# endif // defined(CHK_MIN)
#else // defined(MSG_MIN)
      if (0 == test) uart_puts("memory write test: ");
      else uart_puts("memory address test: ");
#endif // defined(MSG_MIN)
#if defined(CHK_MIN)
      if (0 != test) {
        uart_putnum_u16(addr, 5);
        uart_puts("/65535\r");
      }
#else // defined(CHK_MIN)
      uart_putnum_u16(addr, 5);
      uart_puts("/65535\r");
#endif // defined(CHK_MIN)
    }
    if (0xffff == addr) {
      test++;
#if defined(CHK_MIN)
      if (0 != test) uart_puts("\n");
#else // !defined(CHK_MIN)
      uart_puts("\n");
#endif // !defined(CHK_MIN)
    }
  }
}
#endif // defined(MON_MEM) | defined(CHK_MEM)

void
out
(unsigned char port, unsigned char val)
{
  static unsigned char drive __attribute__ ((unused)) = 0;
  static unsigned char track = 0;
  static unsigned char sect = 0;
  static unsigned char dma_lo = 0;
  static unsigned char dma_hi = 0;
  static unsigned char esc = 0;
#if defined(EFI)
  static INT64 row;
#endif // defined(EFI)

  switch(port) {
  case 1:
    switch (esc) {
    case 1:
      if ('=' == val) esc = 2;
      else if (';' == val) {
        if (vt_cnv == 1) {
          uart_puts("\e[2J");
#if defined(EFI)
        } else {
          uefi_call_wrapper(efi_systab->ConOut->ClearScreen, 1,
                            efi_systab->ConOut);
#endif // defined(EFI)
        }
        esc = 0;
      } else esc = 0;
      break;
    case 2:
      if (vt_cnv == 1) {
        uart_puts("\e[");
        uart_putnum_u16(val - 0x20 + 1, -1);
#if defined(EFI)
      } else {
        row = val - 0x20;
#endif // defined(EFI)
      }
      esc = 3;
      break;
    case 3:
      if (vt_cnv == 1) {
        uart_putchar(';');
        uart_putnum_u16(val - 0x20 + 1, -1);
        uart_putchar('H');
#if defined(EFI)
      } else {
        INT64 column = val - 0x20;
        uefi_call_wrapper(efi_systab->ConOut->SetCursorPosition, 3,
                          efi_systab->ConOut, column, row);
#endif // defined(EFI)
      }
      esc = 0;
      break;
    default:
      if (0 == vt_cnv) uart_putchar(val);
      else {
        if (0x1a == val) {
          if (vt_cnv == 1) {
            uart_puts("\e[2J");
#if defined(EFI)
          } else {
            uefi_call_wrapper(efi_systab->ConOut->ClearScreen, 1,
                              efi_systab->ConOut);
#endif // defined(EFI)
          }
        } else if (0x1b == val) esc = 1;
        else uart_putchar(val);
      }
    }
    break;
  case 10:
    drive = val;
    break;
  case 11:
    track = val;
    break;
  case 12:
    sect = val;
    break;
  case 13: {
    unsigned long pos = ((unsigned long)track * 26 + sect - 1) * 128;
    unsigned long blk = pos & 0xfffffe00;
    unsigned short off = pos & 0x1ff;
    disk_read(blk);
    unsigned short i;
    unsigned short addr = (dma_hi << 8) | dma_lo;
    if (0 == val) {
      for (i = 0; i < 128; i++) sram_write(addr + i, sdcard_read(off + i));
    } else if (0 == wr_prt) {
      for (i = 0; i < 128; i++) sdcard_write(off + i, sram_read(addr + i));
      sdcard_flush();
    }
    break; }
  case 15:
    dma_lo = val;
    break;
  case 16:
    dma_hi = val;
    break;
  }
}

unsigned char
in
(unsigned char port)
{
  if (0 == port) {
    if (0 != uart_peek()) return 0xff;
    return 0;
  } else if (1 == port) {
    int c;
    do {
      c = uart_getchar();
    } while (-1 == c);
    return c;
  } else if (14 == port) {
    return 0;
  }
  return 0;
}

int
machine_boot
(void)
{
  uart_init();
  sram_init();
  sdcard_init();
#if defined(MSG_MIN)
  uart_putsln("\r\nCP/Mega88");
#else // defined(MSG_MIN)
  uart_putsln("\r\nbooting CP/Mega88 done.");
#endif // defined(MSG_MIN)
#if defined(CLR_MEM)
  mem_clr();
#endif // defined(CLR_MEM)
#if defined(MON_MEM) || defined(CHK_MEM)
  mem_chk();
#endif // defined(MON_MEM) || defined(CHK_MEM)
  char rc = sdcard_open();
  if (rc >= 0) uart_putsln("SDC: ok");
#if !defined(MSG_MIN)
  else {
    uart_puts("SDC: err(");
    uart_puthex(-rc);
    uart_putsln(")");
  }
#endif // !defined(MSG_MIN);
#if defined(USE_FAT)
  rc = fat_init();
  if (rc >= 0) {
    uart_puts("FAT: ");
    if ((4 == rc) || (6 == rc)) uart_puts("FAT16");
    else if (0xb == rc) uart_puts("FAT32");
    else uart_puthex(rc);
    uart_putsln("");
    sd_fat = 1;
  } else {
#if !defined(MSG_MIN)
    uart_puts("FAT: ");
    uart_puthex(-rc);
    uart_putsln("");
    sd_fat = 0;
#endif // !defined(MSG_MIN);
  }
#endif // defined(USE_FAT)
#if defined(CPU_EMU_C)
  work.load_8 = &sram_read;
  work.store_8 = &sram_write;
  work.in = &in;
  work.out = &out;
#endif // defined(CPU_EMU_C)
  if (0x88 != eeprom_read(0)) {
    uart_putsln("EEPROM: init");
    int i;
    for (i = 0; i < 256; i++) eeprom_write(i, 0);
    eeprom_write(0, 0x88);
  } else {
    uart_putsln("EEPROM: load");
    if (0x88 == eeprom_read(16)) {
      char buf[8 + 1 + 3 + 1];
      eeprom_read_string(17, buf);
      if (0 != sd_fat) mount(buf);
    }
    if (0x88 == eeprom_read(8)) boot();
  }
#if defined(MONITOR)
  for (;;) prompt();
#else // defined(MONITOR)
  uart_putsln("starting CP/M 2.2 for ATmega88 with i8080 emulation");
#if defined(CPM_DEBUG)
  while (-1 == uart_getchar());
#endif // defined(CPM_DEBUG)
  boot();
#endif // defined(MONITOR)
  return 0;
}
