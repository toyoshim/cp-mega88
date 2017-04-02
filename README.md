# CP/Mega88

## What is this?
This is an i8080 emulator running on ATmega88, and on which CP/M can run. Also, it can be built as a UEFI application that can run on x86_64 platforms without any operating system.

## Specification
to be written

## Software Test
You can try running CP/M on POSIX environment, or EFI firmware.

### Build for POSIX
```
% git clone https://github.com/toyoshim/cp-mega88.git
% cd cp-mega88
% make posix
```

### Boot CP/M on POSIX
For booting, you need a CP/M disk image named 'sdcard.img' in current directory. i8080 I/O configuration of CP/Mega88 is compatible with z80pack. So you can use the image for z80pack (see following external resources section).
```
% cd obj.posix
% tar zxvf z80pack-1.17.tgz
% ln -s z80pack-1.17/cpmsim/disks/library/cpm2-1.dsk sdcard.img
% ./cpmega88

booting CP/Mega88 done.
memory write test: 65535/65535
memory address test: 65535/65535
SDC: ok
FAT: error(02)
EEPROM: init
CP/Mega88>b

A>_
```
In addition to CP/M raw disk images, you can use FAT formated SD CARD images which include CP/M disk images. In that case, you should mount CP/M disk images before the boot command.
```
% ./cpmega88

booting CP/Mega88 done.
memory write test: 65535/65535
memory address test: 65535/65535
SDC: ok
FAT: FAT16
EEPROM: load
CP/Mega88>ls
 -rw- DISK1.IMG
CP/Mega88>m DISK1.IMG
A: DISK1.IMG ok
CP/Mega88>b

A>_
```

### As an UEFI application
If you want, you can build it as an UEFI application so to run it without any operating system. Here is how to
build it on Ubuntu 14.04.
```
% sudo apt-get install gnu-efi
% make uefi
```
You can place a CP/M disk image at EFI/cpmega88/sdcard.img, and run
cpmega88.efi from UEFI Shell, or boot it directly by e.g., placing it to
EFI/Boot/bootx64.efi and disabling the Secure Boot.

On Mac, you can also boot it directly by pushing option key on booting.

Here is an example step to create a bootable media.
```
% sudo parted -a optimal /dev/sdb  # assuming the /dev/sdb is a target device.
(parted) mklabel gpt
(parted) mkpart primary fat32 0 100%
(parted) quit
% mkdir efi
% sudo mkfs.vfat /dev/sdb1
% sudo mount /dev/sdb1 efi
% cd obj.uefi
% make -f ../makefiles/Makefile.uefi install
% sudo cp -r fat/EFI efi
% sudo cp $(somewhere)/sdcard.img efi/EFI/cpmega88/
% sudo umount efi
% rmdir efi
```

If you want to try it on QEMU, here is a step.
```
% make make run_uefi
```
You need QEMU of course, and will need OVMF.fd that adds UEFI support for QEMU.

You would like the CP/Mega88 auto boot mode that could be enabled by "a on"
from the monitor. Configuration will be stored to EFI/cpmega/eeprom.img

### As an U-Boot application that runs on AZ/05M
You can build an U-Boot application that can boot directly without any operating
system through the U-Boot on Toshiba AZ/05M.

Here is a step to build the bootable image.

```
% sudo apt-get install gcc-arm-none-eabi  # for debian and variants
% git clone https://github.com/toyoshim/cp-mega88.git
% cd cp-mega88
% make uboot_ac100
```

And here is a sequence to boot it from the U-Boot.

```
# ext2load mmc 0:1 0x0c100000 /boot/cp-mega88.bin
# ext2load mmc 0:1 0x0c200000 /boot/sdcard.img
# go 0x0c100000
```

Note that you need to setup U-Boot correctly so that the AZ/05M boots from the
U-Boot to choose the system, and this instruction does not cover it.
You could use bootmenu command to automate the boot sequence.

### As an U-Boot application that runs on Raspberry Pi
You can build the one for Raspberry Pi, instead. Please make it with the target
`uboot_rpi`.

You should prepare Rasberry Pi ready SD-card, and copy sdcard.img,
obj.uboot_rpi/cpmeta88.bin and third_party/u-boot/u-boot.bin to the boot
partition that is formatted in fat.

Also you should overwrite config.txt as follows.

```
kernel=u-boot.bin
```

With this SD-card, Raspberry Pi should be able to boot with U-Boot.
Once Das U-Boot shows a prompt, following commands will boot CP/M.

```
# fatload mmc 0:1 0x00100000 /cpmega88.bin
# fatload mmc 0:1 0x00200000 /sdcard.img
# go 0x00100000
```

Note that sdcard image works like a ram disk, and the system does not write
back stored data to the original image file.

### Monitor Commands
```
CP/Mega88>help
monitor commands
 r                : reset
 b                : boot CP/M 2.2
 wp <on/off>      : file system write protection
 a <on/off>       : auto boot
 mr <addr>        : memory read from <addr>
 mw <addr>,<data> : memory write <data> to <addr>
 so               : sdcard open
 sd               : sdcard dump block buffer
 sf <addr>        : sdcard fetch block
 ss <addr>        : sdcard store block
 ls               : file listing
 cd               : change directory
 m <filename>     : mount image disk
 vt <on/off>      : vt100 compatible mode
```

## Hardware Test
to be written

## Schematic
<a href="https://raw.github.com/wiki/toyoshim/cp-mega88/data/mega88_sch.png"><img src="https://raw.github.com/wiki/toyoshim/cp-mega88/data/mega88_sch.png" width="800"/></a>

## Prebuilt Firmware Download
 * [Dec/26/2010 version](https://raw.github.com/wiki/toyoshim/cp-mega88/data/cpmega88-20101226.hex "cpmega88-20101226.hex")

## Internal Resources
 * [pin assignment](https://github.com/toyoshim/cp-mega88/blob/wiki/Hardware.md)
 * [development log slides](https://docs.google.com/present/view?id=d6f82bz_5n23p4jc6)
 * [demo movie](http://www.sprasia.com/channel/toyoshim/20100127012926.html)
 * [LT slides](http://prezi.com/jgletspfbwa3/cpmega88/)

## External Resources
 * [z80pack](http://www.autometer.de/unix4fun/z80pack/) ... Follow the first link in the Downloads section to find a z80pack-1.xx.tgz.
