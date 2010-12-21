	.arch atmega88
	.include "sram_mac.s"
	.text

.global sram_read
	.type	sram_read, @function
sram_read:
	/*
	 * r25: address high
	 * r24: address low
	 *   18 cycles + rcall(3 cycles)
	 */
	_sram_read
	ret
	.size sram_read, .-sram_read

.global sram_write
	.type	sram_write, @function
sram_write:
	/*
	 * r25: address high
	 * r24: address low
	 * r22: data
	 *   14 cycles + rcall(3 cycles)
	 */
	_sram_write
	ret
	.size sram_write, .-sram_write
