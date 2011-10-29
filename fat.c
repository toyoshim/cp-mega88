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

#include "fat.h"
#include "sdcard.h"

#define OFF_FS_DESC 450
#define OFF_P1_1SCT 454
#define OFF_FS_TYPE 54
#define OFF_B_P_SCT 11
#define OFF_SCT_P_C 13
#define OFF_NUM_O_F 16
#define OFF_ROOTNUM 17
#define OFF_SCT_P_F 22

static unsigned char sectors_per_cluster;
static unsigned long fat_first_sect;

static unsigned long dir_sector;
static unsigned short dir_entries;
static unsigned short dir_offset;
static unsigned short dir_cluster;

static unsigned short file_cluster;
static unsigned long file_offset;
static unsigned long file_size;

static unsigned short last_cluster = 0xffff;
static unsigned long last_offset = 0;

static unsigned short
read2
(unsigned short off)
{
  unsigned short rc =
    (sdcard_read(off + 1) <<  8) |
    (sdcard_read(off + 0) <<  0);
  return rc;
}

static unsigned long
read4
(unsigned short off)
{
  unsigned long rc =
    ((unsigned long)sdcard_read(off + 3) << 24) |
    ((unsigned long)sdcard_read(off + 2) << 16) |
    (sdcard_read(off + 1) <<  8) |
    (sdcard_read(off + 0) <<  0);
  return rc;
}

static int
fetch_cluster
(unsigned short cluster, unsigned long offset)
{
  if ((cluster == last_cluster) &&
      ((offset & 0xfe00) == (last_offset & 0xfe00))) return 0;
  last_cluster = cluster;
  last_offset = offset;
  unsigned long sector;
  if (0 == cluster) sector = dir_sector + (offset >> 9);
  else {
    while (offset > (sectors_per_cluster << 9)) {
      unsigned long pos = ((fat_first_sect + 1) << 9) + (cluster << 1);
      if (sdcard_fetch(pos & 0xfffffe00) < 0) return -1;
      cluster = read2(pos & 0x000001ff);
      if ((cluster < 2) || (0xfff7 <= cluster)) return -2;
      offset -= (sectors_per_cluster << 9);
    }
    unsigned long cluster_sect = dir_sector + 32 + (unsigned long)(cluster - 2) * sectors_per_cluster;
    sector = cluster_sect + (offset >> 9);
  }
  return sdcard_fetch(sector << 9);
}

int
fat_init
(void)
{
  if (sdcard_fetch(0) < 0) return -1;
  char fs_desc = sdcard_read(OFF_FS_DESC);
  if ((0x55 != sdcard_read(510)) || (0xaa != sdcard_read(511))) return -2;
  if ((4 != fs_desc) && (6 != fs_desc)) return -80 - fs_desc;

  // BPB sector
  fat_first_sect = read4(OFF_P1_1SCT);

  if (sdcard_fetch(fat_first_sect << 9) < 0) return -3;
  if ((0x55 != sdcard_read(510)) || (0xaa != sdcard_read(511))) return -4;

  unsigned short bytes_per_sector = read2(OFF_B_P_SCT);
  unsigned char number_of_fats = sdcard_read(OFF_NUM_O_F);
  unsigned char sectors_per_fat = sdcard_read(OFF_SCT_P_F);
  sectors_per_cluster = sdcard_read(OFF_SCT_P_C);
  dir_entries = read2(OFF_ROOTNUM);
  dir_sector = fat_first_sect + 1 + sectors_per_fat * number_of_fats;

  if (512 != bytes_per_sector) return -5;

  dir_cluster = 0;
  fat_rewind();
  return fs_desc;
}

void
fat_rewind
(void)
{
  dir_offset = 0xffff;
}

int
fat_next
(void)
{
  for (dir_offset++; dir_offset < dir_entries; dir_offset++) {
    unsigned short off = dir_offset % 16;
    if (fetch_cluster(dir_cluster, ((unsigned long)dir_offset << 5)) < 0) return -2;
    unsigned char first_char = sdcard_read(off << 5);
    if (0 == first_char) continue;
    if (0 != (0x80 & first_char)) continue;
    if (0x0f == fat_attr()) continue;
    return dir_offset;
  }
  return -1;
}

void
fat_name
(char *namebuf)
{
  unsigned short off = dir_offset % 16;
  int i;
  for (i = 0; i < (8 + 3); i++) {
    unsigned char c = sdcard_read((off << 5) + i);
    if (0x20 != c) {
      if (8 == i) *namebuf++ = '.';
      *namebuf++ = c;
    }
  }
  *namebuf = 0;
}

char
fat_attr
(void)
{
  unsigned short off = dir_offset % 16;
  return sdcard_read((off << 5) + 11);
}

int
fat_chdir
(void)
{
  unsigned short off = dir_offset % 16;
  dir_cluster = read2((off << 5) + 26);
  fat_rewind();
  return 0;
}

unsigned long
fat_size
(void)
{
  unsigned short off = dir_offset % 16;
  return read4((off << 5) + 28);
}

int
fat_open
(void)
{
  unsigned short off = dir_offset % 16;
  file_cluster = read2((off << 5) + 26);
  file_size = fat_size();
  return fat_seek(0);
}

int
fat_seek
(unsigned long pos)
{
  if (file_size <= pos) {
    pos = file_size - 1;
    return -1;
  }
  file_offset = pos;
  return 0;
}

int
fat_read
(void)
{
  return fetch_cluster(file_cluster, file_offset);
}
