/*
 * Copyright 2023 Takashi TOYOSHIMA <toyoshim@gmail.com>
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "eeprom.h"

// TODO: Make data persistent.
static unsigned char rom[EEPROM_SIZE] = {
    0x88, 0, 0, 0, 0, 0, 0, 0, 0x88, 0, 0, 0, 0, 0, 0, 0, 0,
};

void eeprom_write(unsigned short addr, unsigned char data) {
  rom[addr] = data;
}

unsigned char eeprom_read(unsigned short addr) {
  return rom[addr];
}