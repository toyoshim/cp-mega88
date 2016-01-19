#
# Copyright (c) 2010, Takashi TOYOSHIMA <toyoshim@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# - Redistributions of source code must retain the above copyright notice,
#   this list of conditions and the following disclaimer.
#
# - Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
#
# - Neither the name of the authors nor the names of its contributors may be
#   used to endorse or promote products derived from this software with out
#   specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUE
# NTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
# GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
#

CC	= avr-gcc
#CC	= avr-gcc
OBJCOPY	= avr-objcopy
OBJDUMP	= avr-objdump
SIZE	= avr-size
HIDSPX	= hidspx
MCU	= atmega88
CFLAGS	= -Os -Wall -mmcu=$(MCU) -mtiny-stack
LIBS	=
LDFLAGS	=
SFLAGS	= --mcu=$(MPU)
HFLAGS	= -d2
TARGET	= cpmega88
OBJS	= main.o uart.o uart_asm.o sram.o sram_asm.o sdcard.o fat.o eeprom.o i8080.o

all: $(TARGET).hex

size: $(TARGET).elf
	$(SIZE) $(SFLAGS) $(TARGET).elf

spx: all
	$(HIDSPX) $(HFLAGS) $(TARGET).hex

%.hex: %.elf
#	$(OBJCOPY) -j .text -j .data -O ihex $< $@
	$(OBJCOPY) -O ihex $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

%.o: %.s
	$(CC) -c -o $@ $<

%.o: %.S
	$(CC) -c -o $@ $<

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf *.o *.hex *.elf

