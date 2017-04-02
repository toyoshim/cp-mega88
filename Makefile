#
# Copyright (c) 2016, Takashi TOYOSHIMA <toyoshim@gmail.com>
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

.PHONY: all distclean clean usage nacl run_uefi
.PRECIOUS: obj.%
usage:
	@echo "Usage: make <target>"
	@echo "  Target:"
	@echo "    all            ... build all following targets"
	@echo "    avr            ... build firmware for ATmega88"
	@echo "    posix          ... build application for posix compatible host"
	@echo "    nacl           ... build for both i686-nacl and x86_64-nacl"
	@echo "    nacl32         ... build for i686-nacl"
	@echo "    nacl64         ... build for x86_64-nacl"
	@echo "    uboot_ac100    ... build U-Boot application for AZ/05M"
	@echo "    uboot_qemu_arm ... build U-Boot application for QEMU/arm"
	@echo "    uboot_rpi      ... build U-Boot application for Raspberry Pi"
	@echo "    uefi           ... build UEFI application for x86_64"
	@echo "    run_uefi       ... run built UEFI application on QEMU"
	@echo "    clean          ... delete all generated files"
	@echo "    distclean      ... delete all generated and downloaded files"

all: avr posix nacl32 nacl64 uboot_ac100 uboot_qemu_arm uboot_rpi uefi

nacl: nacl32 nacl64

%: obj.%
	make -C $< -f ../makefiles/Makefile.$@

obj.%:
	mkdir $@

nacl32:: third_party/naclfs

nacl64:: third_party/naclfs

uboot_ac100:: third_party/u-boot-ac100-exp

uboot_rpi:: third_party/u-boot

third_party/naclfs: third_party
	@if [ ! -d $@ ]; then \
		cd third_party && \
		git clone https://github.com/toyoshim/naclfs.git && \
		cd naclfs && \
		make newlib; \
	fi

third_party/u-boot-ac100-exp: third_party
	@if [ ! -d $@ ]; then \
		cd third_party && \
		git clone https://github.com/ac100-ru/u-boot-ac100-exp.git && \
		cd u-boot-ac100-exp && \
		make paz00_config CROSS_COMPILE=arm-none-eabi- && \
		make all CROSS_COMPILE=arm-none-eabi-; \
	fi

third_party/u-boot: third_party
	@if [ ! -d $@ ]; then \
		cd third_party && \
		git clone git://git.denx.de/u-boot.git && \
		cd u-boot && \
		make rpi_defconfig CROSS_COMPILE=arm-none-eabi- && \
		make all CROSS_COMPILE=arm-none-eabi-; \
	fi

third_party/OVMF.fd: third_party third_party/OVMF-X64-r15214.zip
	@if [ ! -f $@ ]; then \
		cd third_party && \
		unzip OVMF-X64-r15214.zip; \
	fi

third_party/OVMF-X64-r15214.zip:
	cd third_party && \
	wget --prefer-family=ipv4 --trust-server-names \
		http://sourceforge.net/projects/edk2/files/OVMF/OVMF-X64-r15214.zip/download

third_party/z80pack-1.27: third_party/z80pack-1.27.tgz
	cd third_party && \
	tar zxf z80pack-1.27.tgz

third_party/z80pack-1.27.tgz:
	cd third_party && \
	wget http://www.autometer.de/unix4fun/z80pack/ftp/z80pack-1.27.tgz

third_party:
	mkdir third_party

run_uefi: uefi third_party/OVMF.fd third_party/z80pack-1.27
	make -C obj.uefi -f ../makefiles/Makefile.uefi run

install_packages: third_party
	sudo apt-get update && \
	sudo apt-get install avr-libc device-tree-compiler gcc \
		gcc-arm-none-eabi gcc-avr git gnu-efi libc6-i386 make python unzip wget && \
	cd third_party && \
	wget https://storage.googleapis.com/nativeclient-mirror/nacl/nacl_sdk/nacl_sdk.zip && \
	unzip nacl_sdk.zip && \
	cd nacl_sdk && \
	./naclsdk install pepper_47

distclean: clean
	rm -rf third_party

clean:
	rm -rf obj.* native_client/*.nexe
