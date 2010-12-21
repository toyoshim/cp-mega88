.equ	DOUT,11
.equ	DDIR,10
.equ	DIN,9
.equ	CTRL,5

.equ	W_X,0
.equ	SEL,1
.equ	A16,2
.equ	C_L,4
.equ	C_H,5

.equ	C1,(1 << C_L)
.equ	C2,(1 << C_H)
.equ	RD,((1 << W_X) | (1 << SEL))
.equ	WR,(1 << SEL)

.equ	DATA_IN,0
.equ	DATA_OUT,0xff

.macro _sram_read adr_l=r24, adr_h=r25, ret=r24, work=r23
	out DOUT, \adr_l/* output address low   */
	ldi \work, C1
	out CTRL, \work	/* assert CLK for Latch */
	out DOUT, \adr_h/* output address high  */
	ldi \work, C2
	out CTRL, \work	/* assert CLK for Latch */
	out DDIR, r1	/* data port as input   */
	ldi \work, RD
	out CTRL, \work	/* assert /W and E2     */
	out DOUT, r1	/* disable pull-ups     */
	ldi \work, DATA_OUT
	out CTRL, r1	/* negate signals       */
	in \ret, DIN	/* read data            */
	out DDIR, \work	/* data port as output  */
.endm

.macro _sram_write adr_l=r24, adr_h=r25, data=r22, work=r23
	out DOUT, \adr_l/* output address low   */
	ldi \work, C1
	out CTRL, \work	/* assert CLK for Latch */
	out DOUT, \adr_h/* output address high  */
	ldi \work, C2
	out CTRL, \work	/* assert CLK for Latch */
	out DOUT, \data	/* output data          */
	ldi \work, WR
	out CTRL, \work	/* assert E2            */
	out CTRL, r1	/* negate signals       */
.endm
