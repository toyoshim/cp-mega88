/*
 * Copyright (c) 2010, Takashi TOYOSHIMA <toyoshim@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * - Neither the name of the authors nor the names of its contributors may be
 *   used to endorse or promote products derived from this software with out
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUE
 * NTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

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
