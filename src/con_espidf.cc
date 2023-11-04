/*
 * Copyright 2023 Takashi TOYOSHIMA <toyoshim@gmail.com>
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

extern "C" {
#include "con.h"
}

#include <stdio.h>

#include "freertos/freertos.h"
#include "freertos/task.h"

#define LGFX_AUTODETECT
#include "LovyanGFX.hpp"

static LGFX lcd;

static void update(void) {
  // TODO: implement software keyboard.
  int32_t x, y;
  if (lcd.getTouchRaw(&x, &y)) {
    printf("touch (%ld, %ld)\n", x, y);
  }
}

extern "C" void con_init(void) {
  lcd.init();
  lcd.fillScreen(lcd.color888(0, 0, 0));
  lcd.setTextColor(lcd.color888(0, 255, 0), lcd.color888(0, 0, 0));
  lcd.setFont(&fonts::lgfxJapanGothic_8);
  lcd.setTextWrap(false);
  lcd.setCursor(0, 0);

  lgfx::lgfxPinMode(39, lgfx::pin_mode_t::input);
}

extern "C" void con_putchar(unsigned char c) {
  // TODO: handle ADM-3A compatible escape sequences.
  if (c == '\r') {
    lcd.setCursor(0, lcd.getCursorY());
  } else {
    lcd.write(c);
  }
}

extern "C" int con_getchar(void) {
  if (con_peek() == 0) {
    return -1;
  }
  // TODO: handle software keyboard.
  return -1;
}

extern "C" int con_peek(void) {
  update();

  vTaskDelay(1);
  return 0;
}