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

	.arch atmega88
	.text

.equ	PORT,8
.equ	PIN,6
.equ	RX,0
.equ	TX,1

	.type	wait, @function
wait:
	/*
	 * (521 - 59) / 4 = 116 cycle wait
	 *  1 + (3 x 34 - 1) + 10 + 4
	 */
	ldi r23, 34	/* 1   */
1:	dec r23		/* 1   */
	brne 1b		/* 1/2 */
	cp r19, r20	/* 1   */
	breq 2f		/* 1/2 */
	nop		/* 1   */
	nop		/* 1   */
	nop		/* 1   */
	nop		/* 1   */
	nop		/* 1   */
	nop		/* 1   */
	rjmp 3f		/* 2   */
2:	in r0, PIN	/* 1   */
	clc		/* 1   */
	sbrc r0, RX	/* 1/2 */
	sec		/* 1   */
	ror r21		/* 1   */
	dec r22		/* 1   */
	nop		/* 1   */
3:	ret		/* 4   */
	.size wait, .-wait

	.type	x4wait, @function
x4wait:
	/*
	 * 521 - 11 = 510 cycle wait
	 *  1 + (3 x 168 - 1) + 1 + 4
	 */
	ldi r23, 168	/* 1   */
1:	dec r23		/* 1   */
	brne 1b		/* 1/2 */
	nop		/* 1   */
	ret		/* 4   */
	.size x4wait, .-x4wait

.global uart_tx
	.type	uart_tx, @function
uart_tx:
	/*
	 * r24: tx c
	 * r18: tx count
	 * r19: tx phase
	 * r20: rx mode (0: none / 1-4: phase0-3)
	 * r21: rx c
	 * r22: rx count
	 * r23:	wait count
	 */
	ldi r18, 9
	mov r20, r1
	ldi r22, 9
	clc
	/* 59/56 cycle/loop */
1:	brcs 3f		/* 1/2 ---       */
2:	/* output 0 */  /*      |        */
	nop		/* 1    |        */
	cbi PORT, TX	/* 2    |        */
	rjmp 4f		/* 2    |        */
3:	/* output 1 */  /*      | 7t     */
	sbi PORT, TX	/* 2    |        */
	nop		/* 1    |        */
	nop		/* 1    |        */
4:	ldi r19, 1	/* 1   ---       */
5:	cp r20, r1	/* 1   ---       */
	brne 10f	/* 1/2  |        */
	in r0, PIN	/* 1    |        */
	sbrs r0, RX	/* 1/2  |        */
	mov r20, r19	/* 1    |        */
11:	rcall wait	/* 3    |12t/11t */
	inc r19		/* 1    | =47t   */
	cpi r19, 5	/* 1    |        */
	brne 5b		/* 1/2 ---       */
	dec r18		/* 1   ---       */
	breq 6f		/* 1/2  | 5t/2t  */
	ror r24		/* 1    |        */
	rjmp 1b		/* 2   ---       */
6:	cp r20, r1	/* 1   */
	brne 7f		/* 1/2 */
	nop		/* 1   */
	nop		/* 1   */
	sbi PORT, TX	/* STOP bit */
	rcall x4wait
	mov r24, r1
	mov r25, r1
	ret
7:	nop
	sbi PORT, TX	/* STOP bit */
	cp r22, r1
	breq 9f
	ldi r19, 1
8:	nop
	nop
	nop
	nop
	nop
	rcall wait
	inc r19
	cpi r19, 5
	brne 8b
	nop
	nop
	nop
	nop
	rjmp 7b
9:	mov r24, r21
	ldi r25, 0xff
	rcall x4wait
	ret
10:	rjmp 11b
	.size uart_tx, .-uart_tx

.global uart_rx
	.type	uart_rx, @function
uart_rx:
	rcall x4wait
	ldi r18, 8
1:	in r0, PIN	/* 1   */
	clc		/* 1   */
	sbrc r0, RX	/* 1/2 */
   /* input 1 */
	sec		/* 1   */
2: /* input 0 */
	ror r24		/* 1   */
	rcall x4wait	/* 3   */
	dec r18		/* 1   */
	brne 1b		/* 1/2 */
	ret
	.size uart_rx, .-uart_rx
