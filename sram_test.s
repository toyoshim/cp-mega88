.macro _sram_read adr_l=r24, adr_h=r25, ret=r24, work=r23
	push r24
	push r25
	push r26
	push r27
	rcall load
	mov \ret, r24
	pop r27
	pop r26
	pop r25
	pop r24
.endm

.macro _sram_write adr_l=r24, adr_h=r25, data=r22, work=r23
.endm
