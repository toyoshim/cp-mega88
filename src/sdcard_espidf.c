/*
 * Copyright 2023 Takashi TOYOSHIMA <toyoshim@gmail.com>
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "sdcard.h"

#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>

#include "esp_vfs_fat.h"

static bool initialized = false;
static int fd = -1;
static unsigned char buffer[512];
static unsigned long cur_blk = 0;

static bool lazy_init(void) {
  if (initialized) {
    return true;
  }
  sdspi_device_config_t device_config = SDSPI_DEVICE_CONFIG_DEFAULT();
  device_config.host_id = VSPI_HOST;
  device_config.gpio_cs = (gpio_num_t)4;

  sdmmc_host_t host = SDSPI_HOST_DEFAULT();
  host.slot = device_config.host_id;

  esp_vfs_fat_mount_config_t mount_config = {.format_if_mount_failed = false,
                                             .max_files = 1,
                                             .allocation_unit_size = 16 * 1024};

  sdmmc_card_t* card;
  esp_err_t result = esp_vfs_fat_sdspi_mount("/sdcard", &host, &device_config,
                                             &mount_config, &card);
  if (result == 0) {
    initialized = true;
    return true;
  }
  return false;
}

void sdcard_init(void) {
  if (fd >= 0) {
    close(fd);
    fd = -1;
  }
}

int sdcard_open(void) {
  if (fd >= 0) {
    return -1;
  }
  // Initialize here as the SPI setup should run after the LCD initiation that
  // could be performed after sdcard_init() in con_init().
  if (!lazy_init()) {
    return -1;
  }

  fd = open("/sdcard/cpmega88.img", O_RDWR);
  return (fd >= 0) ? 0 : -1;
}

int sdcard_fetch(unsigned long blk_addr) {
  if (fd < 0) {
    return -1;
  }
  if (0 != (blk_addr & 0x1ff)) {
    return -2;
  }
  if (blk_addr != lseek(fd, blk_addr, SEEK_SET)) {
    return -3;
  }
  if (512 != read(fd, buffer, 512)) {
    return -4;
  }
  cur_blk = blk_addr;
  return 0;
}

int sdcard_store(unsigned long blk_addr) {
  if (fd < 0) {
    return -1;
  }
  if (0 != (blk_addr & 0x1ff)) {
    return -2;
  }
  if (blk_addr != lseek(fd, blk_addr, SEEK_SET)) {
    return -3;
  }
  if (512 != write(fd, buffer, 512)) {
    return -4;
  }
  return 0;
}

unsigned short sdcard_crc(void) {
  return 0xffff;
}

int sdcard_flush(void) {
  return sdcard_store(cur_blk);
}

void* sdcard_buffer(void) {
  return buffer;
}