#include <stdio.h>
#include "sdcard.h"
#include "fat.h"

static FILE *
sdcard_file;

static char
sdcard_buffer[512];

int
sdcard_fetch
(unsigned long blk_addr)
{
  if (0 != (blk_addr & 0x1ff)) perror("invalid block addr");
  if (0 != fseek(sdcard_file, blk_addr, SEEK_SET)) return -1;
  if (512 != fread(sdcard_buffer, 1, 512, sdcard_file)) return -2;
  return 0;
}

unsigned char
sdcard_read
(unsigned short offset)
{
  return sdcard_buffer[offset];
}

void
chdir
(char *name)
{
  fat_rewind();
  for (;;) {
    int rc = fat_next();
    if (rc < 0) break;
    char buf[8+1+3+1];
    fat_name(buf);
    if (0 != strcmp(buf, name)) continue;
    fat_chdir();
    break;
  }
}

void
ls
(void)
{
  fat_rewind();
  for (;;) {
    int rc = fat_next();
    if (rc < 0) break;
    char name[8+1+3+1];
    fat_name(name);
    printf("%d: %s\n", rc, name);
  }
}

void
dump
(void)
{
  fat_rewind();
  for (;;) {
    int rc = fat_next();
    if (rc < 0) return;
    char buf[8+1+3+1];
    if (0 != (0x10 & fat_attr())) continue;
    fat_name(buf);
    fat_open();
    unsigned long size = fat_size();
    printf("size: %d\n", size);
    unsigned long i;
    FILE *fp = fopen(buf, "w");
    for (i = 0; i < size; i += 512) {
      fat_seek(i);
      fat_read();
      fwrite(sdcard_buffer, 1, ((size - i) > 512)? 512: (size - i), fp);
    }
    fclose(fp);
  }
}

int
main
(int argc, char **argv)
{
  sdcard_file = fopen("disk5.backup", "r");
  if (NULL == sdcard_file) perror("sdcard open");
  if (fat_init() < 0) perror("fat_init");
  ls();
  chdir("DCIM");
  ls();
  chdir("100CANON");
  ls();
  dump();
  return 0;
}
