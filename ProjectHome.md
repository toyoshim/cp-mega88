# What is this? #
This is an i8080 emulator running on ATmega88, and on which CP/M can run.

# Specification #
to be written

# Software Test #
You can try to run CP/M on POSIX environment.

## Build ##
```
% hg clone https://cp-mega88.googlecode.com/hg/ cp-mega88
% cd cp-mega88
% make -f Makefile.posix
```

## Boot CP/M ##
For booting, you need a CP/M disk image named 'sdcard.img' in current directory. i8080 I/O configuration of CP/Mega88 is compatible with z80pack. So you can use the image for z80pack (see following external resources section).
```
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

## Monitor Commands ##
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

# Hardware Test #
to be written

# Schematic #
<a href='http://cp-mega88.googlecode.com/files/mega88_sch.png'><img src='http://cp-mega88.googlecode.com/files/mega88_sch.png' width='800' /></a>

# Internal Resources #
  * [pin assignment](http://code.google.com/p/cp-mega88/wiki/Hardware)
  * [development log slides](https://docs.google.com/present/view?id=d6f82bz_5n23p4jc6)
  * [demo movie](http://www.sprasia.com/channel/toyoshim/20100127012926.html)
  * [LT slides](http://prezi.com/jgletspfbwa3/cpmega88/)
# External Resources #
  * [z80pack](http://www.unix4fun.org/z80pack/) ... Unfortunately, official site is offline now. Please check [this thread](http://groups.google.com/group/comp.os.cpm/browse_thread/thread/5c4e450309d661c4) of comp.os.cpm.