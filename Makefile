CC	= avr-gcc-3.4.6
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

fat:
	gcc -o fat_test fat_test.c fat.c
