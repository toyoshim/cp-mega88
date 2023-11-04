# Copyright 2023 Takashi TOYOSHIMA <toyoshim@gmail.com>
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/../src/*_espidf.*
../../src/machine.c ../../src/cpu_8080.c ../../src/sram_base.c
../../src/io_base.c ../../src/led_base.c ../../src/con_base.c
../../src/eeprom_base.c ../../src/sdcard_base.c ../../src/fat.c
)
idf_component_register(SRCS ${app_sources})
add_definitions(
  -D__ESPIDF__
  -D__config_h__
  -DCPU_EMU_C
  -DCHK_MEM
  -DCHK_MIN
  -DMONITOR
  -DMON_MEM
  -DMON_SDC
  -DMON_CON
  -DMON_HELP
  -DNULL=0
  -DMAX_PROMPT=80
  )
set_source_files_properties(${app_sources} PROPERTIES COMPILE_FLAGS -fsigned-char)