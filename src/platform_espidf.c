/*
 * Copyright 2023 Takashi TOYOSHIMA <toyoshim@gmail.com>
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "machine.h"

#include <stddef.h>
#include <stdio.h>

#include "freertos/freertos.h"
#include "freertos/task.h"

#include "esp_task_wdt.h"

static void task(void* arguments) {
  machine_boot();
}

static void launch() {
  // Start emulation at the app core.
  xTaskCreatePinnedToCore(task, "CP/Mega Core", 8192, 0, 1, 0, APP_CPU_NUM);
}

void app_main(void) {
  launch();

  // The main core goes to the event loop.
  for (;;) {
    vTaskDelay(1);
  }
}

void platform_reset(void) {
  vTaskDelete(0);
  launch();
}