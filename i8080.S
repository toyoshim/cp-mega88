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
	.include "sram_mac.s"
//	.include "sram_test.s"

#define reg_a r17
#define reg_f r16
#define reg_b r15
#define reg_c r14
#define reg_d r13
#define reg_e r12
#define reg_h r11
#define reg_l r10
#define reg_pc_h r25
#define reg_pc_l r24
#define reg_sp_h r27
#define reg_sp_l r26

#define reg_mem_h r9
#define reg_mem_l r8
#define reg_mem_r r7

#define reg_tmp1 r18
#define reg_tmp2 r19

.equ	REG_A, 0
.equ	REG_F, 1
.equ	REG_B, 2
.equ	REG_C, 3
.equ	REG_D, 4
.equ	REG_E, 5
.equ	REG_H, 6
.equ	REG_L, 7
.equ	REG_PC_H, 8
.equ	REG_PC_L, 9
.equ	REG_SP_H, 10
.equ	REG_SP_L, 11
.equ	REG_SIZE, 12

.equ	F_CY, 0x01
.equ	F_P , 0x04
.equ	F_AC, 0x10
.equ	F_Z , 0x40
.equ	F_S , 0x80

.equ	B_CY, 0
.equ	B_P , 2
.equ	B_AC, 4
.equ	B_Z , 6
.equ	B_S , 7

.equ	SREG, 0x3f
	
.macro push_regs
	push r24
	push r25
	push r26
	push r27
.endm

.macro pop_regs
	pop r27
	pop r26
	pop r25
	pop r24
.endm

.global i8080_work
.lcomm	i8080_work, REG_SIZE

	.text
load:
	.type load, @function
	_sram_read reg_mem_l, reg_mem_h, reg_mem_r, reg_tmp1
	ret
	.size load, .-load

load_hl:
	.type load_hl, @function
	_sram_read reg_l, reg_h, reg_mem_r, reg_tmp1
	ret
	.size load_hl, .-load_hl

load_sp:
	.type load_sp, @function
	_sram_read reg_sp_l, reg_sp_h, reg_mem_r, reg_tmp1
	ret
	.size load_sp, .-load_sp

store:
	.type store, @function
	_sram_write reg_mem_l, reg_mem_h, reg_mem_r, reg_tmp1
	ret
	.size store, .-store

store_hl:
	.type store_hl, @function
	_sram_write reg_l, reg_h, reg_mem_r, reg_tmp1
	ret
	.size store_hl, .-store_hl

store_sp:
	.type store_sp, @function
	_sram_write reg_sp_l, reg_sp_h, reg_mem_r, reg_tmp1
	ret
	.size store_sp, .-store_sp

.global i8080_reset
	/*
	 * void i8080_reset(void)
	 */
	.type i8080_reset, @function
i8080_reset:
	ldi r30, lo8(i8080_work)
	ldi r31, hi8(i8080_work)
	ldi reg_tmp1, REG_SIZE-2
1:	st z+, r1
	dec reg_tmp1
	brne 1b
	ldi reg_tmp1, 0xff
	st z+, reg_tmp1
	st z+, reg_tmp1
	ret
	.size i8080_reset, .-i8080_reset

.global i8080_run
	/*
	 * char i8080_run(void)
	 */
	.type	i8080_run, @function
i8080_run:
	push r7
	push r8
	push r9
	push r10
	push r11
	push r12
	push r13
	push r14
	push r15
	push r16
	push r17
	ldi r30, lo8(i8080_work)
	ldi r31, hi8(i8080_work)
	ldd reg_a, z+REG_A
	ldd reg_f, z+REG_F
	ldd reg_b, z+REG_B
	ldd reg_c, z+REG_C
	ldd reg_d, z+REG_D
	ldd reg_e, z+REG_E
	ldd reg_h, z+REG_H
	ldd reg_l, z+REG_L
	ldd reg_pc_h, z+REG_PC_H
	ldd reg_pc_l, z+REG_PC_L
	ldd reg_sp_h, z+REG_SP_H
	ldd reg_sp_l, z+REG_SP_L
	rcall fetch
	ldi r30, lo8(i8080_work)
	ldi r31, hi8(i8080_work)
	std z+REG_A, reg_a
	std z+REG_F, reg_f
	std z+REG_B, reg_b
	std z+REG_C, reg_c
	std z+REG_D, reg_d
	std z+REG_E, reg_e
	std z+REG_H, reg_h
	std z+REG_L, reg_l
	std z+REG_PC_H, reg_pc_h
	std z+REG_PC_L, reg_pc_l
	std z+REG_SP_H, reg_sp_h
	std z+REG_SP_L, reg_sp_l
	mov r24, reg_tmp1
	pop r17
	pop r16
	pop r15
	pop r14
	pop r13
	pop r12
	pop r11
	pop r10
	pop r9
	pop r8
	pop r7
	ret
	.size i8080_run, .-i8080_run

	.type fetch_op, @function
fetch_op:
	_sram_read reg_pc_l, reg_pc_h, reg_mem_r, reg_tmp1
	inc reg_pc_l
	breq 1f
	ret
1:	inc reg_pc_h
	ret
	.size fetch_op, .-fetch_op

	.type fetch, @function
fetch_next:
//  	mov reg_tmp1, r1
//	ret
fetch:
	ldi r30, lo8(pm(decode))
	ldi r31, hi8(pm(decode))
	_sram_read reg_pc_l, reg_pc_h, reg_mem_r, reg_tmp1
	inc reg_pc_l
	breq 2f
1:	add r30, reg_mem_r
	adc r31, r1
	ijmp
2:	inc reg_pc_h
	rjmp 1b
decode:
	rjmp fetch_next	// 0x00 nop
	rjmp lxi_b_op	// 0x01 lxi b
	rjmp stax_b_op	// 0x02 stax b
	rjmp inx_b_op	// 0x03 inx b
	rjmp inr_b_op	// 0x04 inr b
	rjmp dcr_b_op	// 0x05 dcr b
	rjmp mvi_b_op	// 0x06 mvi b
	rjmp rlc_op	// 0x07 rlc
	rjmp inv_op	// 0x08
	rjmp dad_b_op	// 0x09 dad b
	rjmp ldax_b_op	// 0x0a ldax b
	rjmp dcx_b_op	// 0x0b dcx_b
	rjmp inr_c_op	// 0x0c inr c
	rjmp dcr_c_op	// 0x0d dcr c
	rjmp mvi_c_op	// 0x0e mvi c
	rjmp rrc_op	// 0x0f rrc
	rjmp inv_op	// 0x10
	rjmp lxi_d_op	// 0x11 lxi d
	rjmp stax_d_op	// 0x12 stax d
	rjmp inx_d_op	// 0x13 inx d
	rjmp inr_d_op	// 0x14 inr d
	rjmp dcr_d_op	// 0x15 dcr d
	rjmp mvi_d_op	// 0x16 mvi d
	rjmp ral_op	// 0x17 ral
	rjmp inv_op	// 0x18
	rjmp dad_d_op	// 0x19 dad d
	rjmp ldax_d_op	// 0x1a ldax d
	rjmp dcx_d_op	// 0x1b dcx d
	rjmp inr_e_op	// 0x1c inr e
	rjmp dcr_e_op	// 0x1d dcr e
	rjmp mvi_e_op	// 0x1e mvi e
	rjmp rar_op	// 0x1f rar
	rjmp inv_op	// 0x20 inv
	rjmp lxi_h_op	// 0x21 lxi h
	rjmp shld_op	// 0x22 shld
	rjmp inx_h_op	// 0x23 inx h
	rjmp inr_h_op	// 0x24 inr h
	rjmp dcr_h_op	// 0x25 dcr h
	rjmp mvi_h_op	// 0x26 mvi h
	rjmp daa_op	// 0x27 daa
	rjmp inv_op	// 0x28
	rjmp dad_h_op	// 0x29 dad h
	rjmp lhld_op	// 0x2a lhld
	rjmp dcx_h_op	// 0x2b dcx h
	rjmp inr_l_op	// 0x2c inr l
	rjmp dcr_l_op	// 0x2d dcr l
	rjmp mvi_l_op	// 0x2e mvi l
	rjmp cma_op	// 0x2f cma
	rjmp inv_op	// 0x30
	rjmp lxi_sp_op	// 0x31 lxi sp
	rjmp sta_op	// 0x32 sta
	rjmp inx_sp_op	// 0x33 inx sp
	rjmp inr_m_op	// 0x34 inr m
	rjmp dcr_m_op	// 0x35 dcr m
	rjmp mvi_m_op	// 0x36 mvi m
	rjmp stc_op	// 0x37 stc
	rjmp inv_op	// 0x38
	rjmp dad_sp_op	// 0x39 dad sp
	rjmp lda_op	// 0x3a lda
	rjmp dcx_sp_op	// 0x3b dcx sp
	rjmp inr_a_op	// 0x3c inr a
	rjmp dcr_a_op	// 0x3d dcr a
	rjmp mvi_a_op	// 0x3e mvi a
	rjmp cmc_op	// 0x3f cmc
	rjmp fetch_next	// 0x40 mov b, b
	rjmp mov_bc_op	// 0x41 mov b, c
	rjmp mov_bd_op	// 0x42 mov b, d
	rjmp mov_be_op	// 0x43 mov b, e
	rjmp mov_bh_op	// 0x44 mov b, h
	rjmp mov_bl_op	// 0x45 mov b, l
	rjmp mov_bm_op	// 0x46 mov b, m
	rjmp mov_ba_op	// 0x47 mov b, a
	rjmp mov_cb_op	// 0x48 mov c, b
	rjmp fetch_next	// 0x49 mov c, c
	rjmp mov_cd_op	// 0x4a mov c, d
	rjmp mov_ce_op	// 0x4b mov c, e
	rjmp mov_ch_op	// 0x4c mov c, h
	rjmp mov_cl_op	// 0x4d mov c, l
	rjmp mov_cm_op	// 0x4e mov c, m
	rjmp mov_ca_op	// 0x4f mov c, a
	rjmp mov_db_op	// 0x50 mov d, b
	rjmp mov_dc_op	// 0x51 mov d, c
	rjmp fetch_next	// 0x52 mov d, d
	rjmp mov_de_op	// 0x53 mov d, e
	rjmp mov_dh_op	// 0x54 mov d, h
	rjmp mov_dl_op	// 0x55 mov d, l
	rjmp mov_dm_op	// 0x56 mov d, m
	rjmp mov_da_op	// 0x57 mov d, a
	rjmp mov_eb_op	// 0x58 mov e, b
	rjmp mov_ec_op	// 0x59 mov e, c
	rjmp mov_ed_op	// 0x5a mov e, d
	rjmp fetch_next	// 0x5b mov e, e
	rjmp mov_eh_op	// 0x5c mov e, h
	rjmp mov_el_op	// 0x5d mov e, l
	rjmp mov_em_op	// 0x5e mov e, m
	rjmp mov_ea_op	// 0x5f mov e, a
	rjmp mov_hb_op	// 0x60 mov h, b
	rjmp mov_hc_op	// 0x61 mov h, c
	rjmp mov_hd_op	// 0x62 mov h, d
	rjmp mov_he_op	// 0x63 mov h, e
	rjmp fetch_next	// 0x64 mov h, h
	rjmp mov_hl_op	// 0x65 mov h, l
	rjmp mov_hm_op	// 0x66 mov h, m
	rjmp mov_ha_op	// 0x67 mov h, a
	rjmp mov_lb_op	// 0x68 mov l, b
	rjmp mov_lc_op	// 0x69 mov l, c
	rjmp mov_ld_op	// 0x6a mov l, d
	rjmp mov_le_op	// 0x6b mov l, e
	rjmp mov_lh_op	// 0x6c mov l, h
	rjmp fetch_next	// 0x6d mov l, l
	rjmp mov_lm_op	// 0x6e mov l, m
	rjmp mov_la_op	// 0x6f mov l, a
	rjmp mov_mb_op	// 0x70 mov m, b
	rjmp mov_mc_op	// 0x71 mov m, c
	rjmp mov_md_op	// 0x72 mov m, d
	rjmp mov_me_op	// 0x73 mov m, e
	rjmp mov_mh_op	// 0x74 mov m, h
	rjmp mov_ml_op	// 0x75 mov m, l
	rjmp halt_op	// 0x76 halt
	rjmp mov_ma_op	// 0x77 mov m, a
	rjmp mov_ab_op	// 0x78 mov a, b
	rjmp mov_ac_op	// 0x79 mov a, c
	rjmp mov_ad_op	// 0x7a mov a, d
	rjmp mov_ae_op	// 0x7b mov a, e
	rjmp mov_ah_op	// 0x7c mov a, h
	rjmp mov_al_op	// 0x7d mov a, l
	rjmp mov_am_op	// 0x7e mov a, m
	rjmp fetch_next	// 0x7f mov a, a
	rjmp add_b_op	// 0x80 add b
	rjmp add_c_op	// 0x81 add c
	rjmp add_d_op	// 0x82 add d
	rjmp add_e_op	// 0x83 add e
	rjmp add_h_op	// 0x84 add h
	rjmp add_l_op	// 0x85 add l
	rjmp add_m_op	// 0x86 add m
	rjmp add_a_op	// 0x87 add a
	rjmp adc_b_op	// 0x88 adc b
	rjmp adc_c_op	// 0x89 adc c
	rjmp adc_d_op	// 0x8a adc d
	rjmp adc_e_op	// 0x8b adc e
	rjmp adc_h_op	// 0x8c adc h
	rjmp adc_l_op	// 0x8d adc l
	rjmp adc_m_op	// 0x8e adc m
	rjmp adc_a_op	// 0x8f adc a
	rjmp sub_b_op	// 0x90 sub b
	rjmp sub_c_op	// 0x91 sub c
	rjmp sub_d_op	// 0x92 sub d
	rjmp sub_e_op	// 0x93 sub e
	rjmp sub_h_op	// 0x94 sub h
	rjmp sub_l_op	// 0x95 sub l
	rjmp sub_m_op	// 0x96 sub m
	rjmp sub_a_op	// 0x97 sub a
	rjmp sbb_b_op	// 0x98 sbb b
	rjmp sbb_c_op	// 0x99 sbb c
	rjmp sbb_d_op	// 0x9a sbb d
	rjmp sbb_e_op	// 0x9b sbb e
	rjmp sbb_h_op	// 0x9c sbb h
	rjmp sbb_l_op	// 0x9d sbb l
	rjmp sbb_m_op	// 0x9e sbb m
	rjmp sbb_a_op	// 0x9f sbb a
	rjmp ana_b_op	// 0xa0 ana b
	rjmp ana_c_op	// 0xa1 ana c
	rjmp ana_d_op	// 0xa2 ana d
	rjmp ana_e_op	// 0xa3 ana e
	rjmp ana_h_op	// 0xa4 ana h
	rjmp ana_l_op	// 0xa5 ana l
	rjmp ana_m_op	// 0xa6 ana m
	rjmp ana_a_op	// 0xa7 ana a
	rjmp xra_b_op	// 0xa8 xra b
	rjmp xra_c_op	// 0xa9 xra c
	rjmp xra_d_op	// 0xaa xra d
	rjmp xra_e_op	// 0xab xra e
	rjmp xra_h_op	// 0xac xra h
	rjmp xra_l_op	// 0xad xra l
	rjmp xra_m_op	// 0xae xra m
	rjmp xra_a_op	// 0xaf xra a
	rjmp ora_b_op	// 0xb0 ora b
	rjmp ora_c_op	// 0xb1 ora c
	rjmp ora_d_op	// 0xb2 ora d
	rjmp ora_e_op	// 0xb3 ora e
	rjmp ora_h_op	// 0xb4 ora h
	rjmp ora_l_op	// 0xb5 ora l
	rjmp ora_m_op	// 0xb6 ora m
	rjmp ora_a_op	// 0xb7 ora a
	rjmp cmp_b_op	// 0xb8 cmp b
	rjmp cmp_c_op	// 0xb9 cmp c
	rjmp cmp_d_op	// 0xba cmp d
	rjmp cmp_e_op	// 0xbb cmp e
	rjmp cmp_h_op	// 0xbc cmp h
	rjmp cmp_l_op	// 0xbd cmp l
	rjmp cmp_m_op	// 0xbe cmp m
	rjmp cmp_a_op	// 0xbf cmp a
	rjmp rnz_op	// 0xc0 rnz
	rjmp pop_b_op	// 0xc1 pop b
	rjmp jnz_op	// 0xc2 jnz
	rjmp jmp_op	// 0xc3 jmp
	rjmp cnz_op	// 0xc4 cnz
	rjmp push_b_op	// 0xc5 push b
	rjmp adi_op	// 0xc6 adi
	rjmp rst_00_op	// 0xc7 rst 00h
	rjmp rz_op	// 0xc8 rz
	rjmp ret_op	// 0xc9 ret
	rjmp jz_op	// 0xca jz
	rjmp inv_op	// 0xcb
	rjmp cz_op	// 0xcc cz
	rjmp call_op	// 0xcd call
	rjmp aci_op	// 0xce aci
	rjmp rst_08_op	// 0xcf rst 08h
	rjmp rnc_op	// 0xd0 rnc
	rjmp pop_d_op	// 0xd1 pop d
	rjmp jnc_op	// 0xd2 jnc
	rjmp out_op	// 0xd3 out
	rjmp cnc_op	// 0xd4 cnc
	rjmp push_d_op	// 0xd5 push d
	rjmp sui_op	// 0xd6 sui
	rjmp rst_10_op	// 0xd7 rst 10h
	rjmp rc_op	// 0xd8 rc
	rjmp inv_op	// 0xd9
	rjmp jc_op	// 0xda jc
	rjmp in_op	// 0xdb in
	rjmp cc_op	// 0xdc cc
	rjmp inv_op	// 0xdd
	rjmp sbi_op	// 0xde sbi
	rjmp rst_18_op	// 0xdf rst 18h
	rjmp rpo_op	// 0xe0 rpo
	rjmp pop_h_op	// 0xe1 pop h
	rjmp jpo_op	// 0xe2 jpo
	rjmp xthl_op	// 0xe3 xthl
	rjmp cpo_op	// 0xe4 cpo
	rjmp push_h_op	// 0xe5 push h
	rjmp ani_op	// 0xe6 ani
	rjmp rst_20_op	// 0xe7 rst 20h
	rjmp rpe_op	// 0xe8 rpe
	rjmp pchl_op	// 0xe9 pchl
	rjmp jpe_op	// 0xea jpe
	rjmp xchg_op	// 0xeb xchg
	rjmp cpe_op	// 0xec cpe
	rjmp inv_op	// 0xed
	rjmp xri_op	// 0xee xri
	rjmp rst_28_op	// 0xef rst 28h
	rjmp rp_op	// 0xf0 rp
	rjmp pop_psw_op	// 0xf1 pop psw
	rjmp jp_op	// 0xf2 jp
	rjmp di_op	// 0xf3 di
	rjmp cp_op	// 0xf4 cp
	rjmp push_psw_op// 0xf5 push psw
	rjmp ori_op	// 0xf6 ori
	rjmp rst_30_op	// 0xf7 rst 30h
	rjmp rm_op	// 0xf8 rm
	rjmp sphl_op	// 0xf9 sphl
	rjmp jm_op	// 0xfa jm
	rjmp ei_op	// 0xfb ei
	rjmp cm_op	// 0xfc cm
	rjmp inv_op	// 0xfd
	rjmp cpi_op	// 0xfe cpi
	rjmp rst_38_op	// 0xff rst 38h

inv_op:
	ldi reg_tmp1, 0xff
	ret

lxi_b_op:	// 0x01
	rcall fetch_op
	mov reg_c, reg_mem_r
	rcall fetch_op
	mov reg_b, reg_mem_r
	rjmp fetch_next

stax_b_op:	// 0x02
	mov reg_mem_l, reg_c
	mov reg_mem_h, reg_b
	mov reg_mem_r, reg_a
	rcall store
	rjmp fetch_next

inx_b_op:	// 0x03
	ldi reg_tmp1, 1
	add reg_c, reg_tmp1
	adc reg_b, r1
	rjmp fetch_next

inr_b_op:	// 0x04
	inc reg_b
	mov reg_tmp1, reg_b
	rjmp inr_comp

dcr_b_op:	// 0x05
	dec reg_b
	mov reg_tmp1, reg_b
	rjmp dcr_comp

mvi_b_op:	// 0x06
	rcall fetch_op
	mov reg_b, reg_mem_r
	rjmp fetch_next

rlc_op:		// 0x07
	clc
	sbrc reg_a, 7
	sec
	rol reg_a
	brcc 1f
	sbr reg_f, F_CY
	rjmp 2f
1:	cbr reg_f, F_CY
2:	rjmp fetch_next

dad_b_op:	// 0x09
	add reg_l, reg_c
	adc reg_h, reg_b
	cbr reg_f, F_CY
	brcc 1f
	sbr reg_f, F_CY
1:	rjmp fetch_next
	
ldax_b_op:	// 0x0a
	mov reg_mem_l, reg_c
	mov reg_mem_h, reg_b
	rcall load
	mov reg_a, reg_mem_r
	rjmp fetch_next

dcx_b_op:	// 0x0b
	ldi reg_tmp1, 1
	sub reg_c, reg_tmp1
	sbc reg_b, r1
	rjmp fetch_next

inr_c_op:	// 0x0c
	inc reg_c
	mov reg_tmp1, reg_c
	rjmp inr_comp

dcr_c_op:	// 0x0d
	dec reg_c
	mov reg_tmp1, reg_c
	rjmp dcr_comp

mvi_c_op:	// 0x0e
	rcall fetch_op
	mov reg_c, reg_mem_r
	rjmp fetch_next

rrc_op:		// 0x0f
	cbr reg_f, F_CY
	lsr reg_a
	brcc 1f
	sbr reg_f, F_CY
	sbr reg_a, 0x80
1:	rjmp fetch_next

lxi_d_op:	// 0x11
	rcall fetch_op
	mov reg_e, reg_mem_r
	rcall fetch_op
	mov reg_d, reg_mem_r
	rjmp fetch_next

stax_d_op:	// 0x12
	mov reg_mem_l, reg_e
	mov reg_mem_h, reg_d
	mov reg_mem_r, reg_a
	rcall store
	rjmp fetch_next
	
inx_d_op:	// 0x13
	ldi reg_tmp1, 1
	add reg_e, reg_tmp1
	adc reg_d, r1
	rjmp fetch_next

inr_d_op:	// 0x14
	inc reg_d
	mov reg_tmp1, reg_d
	rjmp inr_comp

dcr_d_op:	// 0x15
	dec reg_d
	mov reg_tmp1, reg_d
	rjmp dcr_comp

mvi_d_op:	// 0x16
	rcall fetch_op
	mov reg_d, reg_mem_r
	rjmp fetch_next

ral_op:		// 0x17
	clc
	sbrc reg_f, B_CY
	sec
	rol reg_a
	cbr reg_f, F_CY
	brcc 1f
	sbr reg_f, F_CY
1:	rjmp fetch_next

dad_d_op:	// 0x19
	add reg_l, reg_e
	adc reg_h, reg_d
	cbr reg_f, F_CY
	brcc 1f
	sbr reg_f, F_CY
1:	rjmp fetch_next
	
ldax_d_op:	// 0x1a
	mov reg_mem_l, reg_e
	mov reg_mem_h, reg_d
	rcall load
	mov reg_a, reg_mem_r
	rjmp fetch_next

dcx_d_op:	// 0x1b
	ldi reg_tmp1, 1
	sub reg_e, reg_tmp1
	sbc reg_d, r1
	rjmp fetch_next

inr_e_op:	// 0x1c
	inc reg_e
	mov reg_tmp1, reg_e
	rjmp inr_comp

dcr_e_op:	// 0x1d
	dec reg_e
	mov reg_tmp1, reg_e
	rjmp dcr_comp

mvi_e_op:	// 0x1e
	rcall fetch_op
	mov reg_e, reg_mem_r
	rjmp fetch_next

rar_op:		// 0x1f
	clc
	sbrc reg_f, B_CY
	sec
	ror reg_a
	cbr reg_f, F_CY
	brcc 1f
	sbr reg_f, F_CY
1:	rjmp fetch_next

lxi_h_op:	// 0x21
	rcall fetch_op
	mov reg_l, reg_mem_r
	rcall fetch_op
	mov reg_h, reg_mem_r
	rjmp fetch_next

shld_op:	// 0x22
	rcall fetch_op
	mov reg_mem_l, reg_mem_r
	rcall fetch_op
	mov reg_mem_h, reg_mem_r
	mov reg_mem_r, reg_l
	rcall store
	ldi reg_tmp1, 1
	add reg_mem_l, reg_tmp1
	adc reg_mem_h, r1
	mov reg_mem_r, reg_h
	rcall store
	rjmp fetch_next

inx_h_op:	// 0x23
	ldi reg_tmp1, 1
	add reg_l, reg_tmp1
	adc reg_h, r1
	rjmp fetch_next

inr_h_op:	// 0x24
	inc reg_h
	mov reg_tmp1, reg_h
	rjmp inr_comp

dcr_h_op:	// 0x25
	dec reg_h
	mov reg_tmp1, reg_h
	rjmp dcr_comp

mvi_h_op:	// 0x26
	rcall fetch_op
	mov reg_h, reg_mem_r
	rjmp fetch_next

daa_op:		// 0x27
	mov reg_tmp1, reg_a
	andi reg_tmp1, 0x0f
	cpi reg_tmp1, 10
	brcc 1f
	sbrc reg_f, B_AC
	rjmp 1f
	cbr reg_f, F_AC
	rjmp 2f
1:	subi reg_a, -6
	sbr reg_f, F_AC
2:	cpi reg_a, 160
	brcc 3f
	sbrc reg_f, B_CY
	rjmp 3f
	cbr reg_f, F_CY
	rjmp 4f
3:	subi reg_a, -96
	sbr reg_f, F_CY
4:	andi reg_f, ~(F_Z | F_S | F_P)
	mov reg_tmp1, reg_a
	rjmp flag_zsp

dad_h_op:	// 0x29
	add reg_l, reg_l
	adc reg_h, reg_h
	cbr reg_f, F_CY
	brcc 1f
	sbr reg_f, F_CY
1:	rjmp fetch_next
	
lhld_op:	// 0x2a
	rcall fetch_op
	mov reg_mem_l, reg_mem_r
	rcall fetch_op
	mov reg_mem_h, reg_mem_r
	rcall load
	mov reg_l, reg_mem_r
	ldi reg_tmp1, 1
	add reg_mem_l, reg_tmp1
	adc reg_mem_h, r1
	rcall load
	mov reg_h, reg_mem_r
	rjmp fetch_next

dcx_h_op:	// 0x2b
	ldi reg_tmp1, 1
	sub reg_l, reg_tmp1
	sbc reg_h, r1
	rjmp fetch_next

inr_l_op:	// 0x2c
	inc reg_l
	mov reg_tmp1, reg_l
	rjmp inr_comp

dcr_l_op:	// 0x2d
	dec reg_l
	mov reg_tmp1, reg_l
	rjmp dcr_comp

mvi_l_op:	// 0x2e
	rcall fetch_op
	mov reg_l, reg_mem_r
	rjmp fetch_next

cma_op:		// 0x2f
	com reg_a
	rjmp fetch_next

lxi_sp_op:	// 0x31
	rcall fetch_op
	mov reg_sp_l, reg_mem_r
	rcall fetch_op
	mov reg_sp_h, reg_mem_r
	rjmp fetch_next

sta_op:		// 0x32
	rcall fetch_op
	mov reg_mem_l, reg_mem_r
	rcall fetch_op
	mov reg_mem_h, reg_mem_r
	mov reg_mem_r, reg_a
	rcall store
	rjmp fetch_next

inx_sp_op:	// 0x33
	ldi reg_tmp1, 1
	add reg_sp_l, reg_tmp1
	adc reg_sp_h, r1
	rjmp fetch_next

inr_m_op:	// 0x34
	rcall load_hl
	inc reg_mem_r
	rcall store_hl
	mov reg_tmp1, reg_mem_r
	rjmp inr_comp

dcr_m_op:	// 0x35
	rcall load_hl
	dec reg_mem_r
	rcall store_hl
	mov reg_tmp1, reg_mem_r
	rjmp dcr_comp

mvi_m_op:	// 0x36
	rcall fetch_op
	rcall store_hl
	rjmp fetch_next

stc_op:		// 0x37
	sbr reg_f, F_CY
	rjmp fetch_next

dad_sp_op:	// 0x39
	add reg_l, reg_sp_l
	adc reg_h, reg_sp_h
	cbr reg_f, F_CY
	brcc 1f
	sbr reg_f, F_CY
1:	rjmp fetch_next

lda_op:		// 0x3a
	rcall fetch_op
	mov reg_mem_l, reg_mem_r
	rcall fetch_op
	mov reg_mem_h, reg_mem_r
	rcall load
	mov reg_a, reg_mem_r
	rjmp fetch_next

dcx_sp_op:	// 0x3b
	ldi reg_tmp1, 1
	sub reg_sp_l, reg_tmp1
	sbc reg_sp_h, r1
	rjmp fetch_next

inr_a_op:	// 0x3c
	inc reg_a
	mov reg_tmp1, reg_a
	rjmp inr_comp

dcr_a_op:	// 0x3d
	dec reg_a
	mov reg_tmp1, reg_a
	rjmp dcr_comp

mvi_a_op:	// 0x3e
	rcall fetch_op
	mov reg_a, reg_mem_r
	rjmp fetch_next

cmc_op:		// 0x3f
	ldi reg_tmp1, F_CY
	eor reg_f, reg_tmp1
	rjmp fetch_next

mov_bc_op:	// 0x41
	mov reg_b, reg_c
	rjmp fetch_next

mov_bd_op:	// 0x42
	mov reg_b, reg_d
	rjmp fetch_next

mov_be_op:	// 0x43
	mov reg_b, reg_e
	rjmp fetch_next

mov_bh_op:	// 0x44
	mov reg_b, reg_h
	rjmp fetch_next

mov_bl_op:	// 0x45
	mov reg_b, reg_l
	rjmp fetch_next

mov_bm_op:	// 0x46
	rcall load_hl
	mov reg_b, reg_mem_r
	rjmp fetch_next

mov_ba_op:	// 0x47
	mov reg_b, reg_a
	rjmp fetch_next

mov_cb_op:	// 0x48
	mov reg_c, reg_b
	rjmp fetch_next

mov_cd_op:	// 0x4a
	mov reg_c, reg_d
	rjmp fetch_next

mov_ce_op:	// 0x4b
	mov reg_c, reg_e
	rjmp fetch_next

mov_ch_op:	// 0x4c
	mov reg_c, reg_h
	rjmp fetch_next

mov_cl_op:	// 0x4d
	mov reg_c, reg_l
	rjmp fetch_next

mov_cm_op:	// 0x4e
	rcall load_hl
	mov reg_c, reg_mem_r
	rjmp fetch_next

mov_ca_op:	// 0x4f
	mov reg_c, reg_a
	rjmp fetch_next

mov_db_op:	// 0x50
	mov reg_d, reg_b
	rjmp fetch_next

mov_dc_op:	// 0x51
	mov reg_d, reg_c
	rjmp fetch_next

mov_de_op:	// 0x53
	mov reg_d, reg_e
	rjmp fetch_next

mov_dh_op:	// 0x54
	mov reg_d, reg_h
	rjmp fetch_next

mov_dl_op:	// 0x55
	mov reg_d, reg_l
	rjmp fetch_next

mov_dm_op:	// 0x56
	rcall load_hl
	mov reg_d, reg_mem_r
	rjmp fetch_next

mov_da_op:	// 0x57
	mov reg_d, reg_a
	rjmp fetch_next

mov_eb_op:	// 0x58
	mov reg_e, reg_b
	rjmp fetch_next

mov_ec_op:	// 0x59
	mov reg_e, reg_c
	rjmp fetch_next

mov_ed_op:	// 0x5a
	mov reg_e, reg_d
	rjmp fetch_next

mov_eh_op:	// 0x5c
	mov reg_e, reg_h
	rjmp fetch_next

mov_el_op:	// 0x5d
	mov reg_e, reg_l
	rjmp fetch_next

mov_em_op:	// 0x5e
	rcall load_hl
	mov reg_e, reg_mem_r
	rjmp fetch_next

mov_ea_op:	// 0x5f
	mov reg_e, reg_a
	rjmp fetch_next

mov_hb_op:	// 0x60
	mov reg_h, reg_b
	rjmp fetch_next

mov_hc_op:	// 0x61
	mov reg_h, reg_c
	rjmp fetch_next

mov_hd_op:	// 0x62
	mov reg_h, reg_d
	rjmp fetch_next

mov_he_op:	// 0x63
	mov reg_h, reg_e
	rjmp fetch_next

mov_hl_op:	// 0x65
	mov reg_h, reg_l
	rjmp fetch_next

mov_hm_op:	// 0x66
	rcall load_hl
	mov reg_h, reg_mem_r
	rjmp fetch_next

mov_ha_op:	// 0x67
	mov reg_h, reg_a
	rjmp fetch_next

mov_lb_op:	// 0x68
	mov reg_l, reg_b
	rjmp fetch_next

mov_lc_op:	// 0x69
	mov reg_l, reg_c
	rjmp fetch_next

mov_ld_op:	// 0x6a
	mov reg_l, reg_d
	rjmp fetch_next

mov_le_op:	// 0x6b
	mov reg_l, reg_e
	rjmp fetch_next

mov_lh_op:	// 0x6c
	mov reg_l, reg_h
	rjmp fetch_next

mov_lm_op:	// 0x6e
	rcall load_hl
	mov reg_l, reg_mem_r
	rjmp fetch_next

mov_la_op:	// 0x6f
	mov reg_l, reg_a
	rjmp fetch_next

mov_mb_op:	// 0x70
	mov reg_mem_r, reg_b
	rcall store_hl
	rjmp fetch_next

mov_mc_op:	// 0x71
	mov reg_mem_r, reg_c
	rcall store_hl
	rjmp fetch_next

mov_md_op:	// 0x72
	mov reg_mem_r, reg_d
	rcall store_hl
	rjmp fetch_next

mov_me_op:	// 0x73
	mov reg_mem_r, reg_e
	rcall store_hl
	rjmp fetch_next

mov_mh_op:	// 0x74
	mov reg_mem_r, reg_h
	rcall store_hl
	rjmp fetch_next

mov_ml_op:	// 0x75
	mov reg_mem_r, reg_l
	rcall store_hl
	rjmp fetch_next

halt_op:	// 0x76
	ldi reg_tmp1, 1
	sub reg_pc_l, reg_tmp1
	sbc reg_pc_h, r1
	ret

mov_ma_op:	// 0x77
	mov reg_mem_r, reg_a
	rcall store_hl
	rjmp fetch_next

mov_ab_op:	// 0x78
	mov reg_a, reg_b
	rjmp fetch_next

mov_ac_op:	// 0x79
	mov reg_a, reg_c
	rjmp fetch_next

mov_ad_op:	// 0x7a
	mov reg_a, reg_d
	rjmp fetch_next

mov_ae_op:	// 0x7b
	mov reg_a, reg_e
	rjmp fetch_next

mov_ah_op:	// 0x7c
	mov reg_a, reg_h
	rjmp fetch_next

mov_al_op:	// 0x7d
	mov reg_a, reg_l
	rjmp fetch_next

mov_am_op:	// 0x7e
	rcall load_hl
	mov reg_a, reg_mem_r
	rjmp fetch_next

add_b_op:	// 0x80
	add reg_a, reg_b
	rjmp add_comp

add_c_op:	// 0x81
	add reg_a, reg_c
	rjmp add_comp

add_d_op:	// 0x82
	add reg_a, reg_d
	rjmp add_comp

add_e_op:	// 0x83
	add reg_a, reg_e
	rjmp add_comp

add_h_op:	// 0x84
	add reg_a, reg_h
	rjmp add_comp

add_l_op:	// 0x85
	add reg_a, reg_l
	rjmp add_comp

add_m_op:	// 0x86
	rcall load_hl
	add reg_a, reg_mem_r
	rjmp add_comp

add_a_op:	// 0x87
	add reg_a, reg_a
	rjmp add_comp

adc_b_op:	// 0x88
	mov reg_tmp1, reg_b
	rjmp adc_comp

adc_c_op:	// 0x89
	mov reg_tmp1, reg_c
	rjmp adc_comp

adc_d_op:	// 0x8a
	mov reg_tmp1, reg_d
	rjmp adc_comp

adc_e_op:	// 0x8b
	mov reg_tmp1, reg_e
	rjmp adc_comp

adc_h_op:	// 0x8c
	mov reg_tmp1, reg_h
	rjmp adc_comp

adc_l_op:	// 0x8d
	mov reg_tmp1, reg_l
	rjmp adc_comp

adc_m_op:	// 0x8e
	rcall load_hl
	mov reg_tmp1, reg_mem_r
	rjmp adc_comp

adc_a_op:	// 0x8f
	mov reg_tmp1, reg_a
	rjmp adc_comp

sub_b_op:	// 0x90
	sub reg_a, reg_b
	rjmp sub_comp

sub_c_op:	// 0x91
	sub reg_a, reg_c
	rjmp sub_comp

sub_d_op:	// 0x92
	sub reg_a, reg_d
	rjmp sub_comp

sub_e_op:	// 0x93
	sub reg_a, reg_e
	rjmp sub_comp

sub_h_op:	// 0x94
	sub reg_a, reg_h
	rjmp sub_comp

sub_l_op:	// 0x95
	sub reg_a, reg_l
	rjmp sub_comp

sub_m_op:	// 0x96
	rcall load_hl
	sub reg_a, reg_mem_r
	rjmp sub_comp

sub_a_op:	// 0x97
	sub reg_a, reg_a
	rjmp sub_comp

sbb_b_op:	// 0x98
	mov reg_tmp1, reg_b
	rjmp sbb_comp

sbb_c_op:	// 0x99
	mov reg_tmp1, reg_c
	rjmp sbb_comp

sbb_d_op:	// 0x9a
	mov reg_tmp1, reg_d
	rjmp sbb_comp

sbb_e_op:	// 0x9b
	mov reg_tmp1, reg_e
	rjmp sbb_comp

sbb_h_op:	// 0x9c
	mov reg_tmp1, reg_h
	rjmp sbb_comp

sbb_l_op:	// 0x9d
	mov reg_tmp1, reg_l
	rjmp sbb_comp

sbb_m_op:	// 0x9e
	rcall load_hl
	mov reg_tmp1, reg_mem_r
	rjmp sbb_comp

sbb_a_op:	// 0x9f
	mov reg_tmp1, reg_a
	rjmp sbb_comp

ana_b_op:	// 0xa0
	and reg_a, reg_b
	rjmp ana_comp

ana_c_op:	// 0xa1
	and reg_a, reg_c
	rjmp ana_comp

ana_d_op:	// 0xa2
	and reg_a, reg_d
	rjmp ana_comp

ana_e_op:	// 0xa3
	and reg_a, reg_e
	rjmp ana_comp

ana_h_op:	// 0xa4
	and reg_a, reg_h
	rjmp ana_comp

ana_l_op:	// 0xa5
	and reg_a, reg_l
	rjmp ana_comp

ana_m_op:	// 0xa6
	rcall load_hl
	and reg_a, reg_mem_r
	rjmp ana_comp

ana_a_op:	// 0xa7
	and reg_a, reg_a
	rjmp ana_comp

xra_b_op:	// 0xa8
	eor reg_a, reg_b
	rjmp xra_comp

xra_c_op:	// 0xa9
	eor reg_a, reg_c
	rjmp xra_comp

xra_d_op:	// 0xaa
	eor reg_a, reg_d
	rjmp xra_comp

xra_e_op:	// 0xab
	eor reg_a, reg_e
	rjmp xra_comp

xra_h_op:	// 0xac
	eor reg_a, reg_h
	rjmp xra_comp

xra_l_op:	// 0xad
	eor reg_a, reg_l
	rjmp xra_comp

xra_m_op:	// 0xae
	rcall load_hl
	eor reg_a, reg_mem_r
	rjmp xra_comp

xra_a_op:	// 0xaf
	eor reg_a, reg_a
	rjmp xra_comp

ora_b_op:	// 0xb0
	or reg_a, reg_b
	rjmp ora_comp

ora_c_op:	// 0xb1
	or reg_a, reg_c
	rjmp ora_comp

ora_d_op:	// 0xb2
	or reg_a, reg_d
	rjmp ora_comp

ora_e_op:	// 0xb3
	or reg_a, reg_e
	rjmp ora_comp

ora_h_op:	// 0xb4
	or reg_a, reg_h
	rjmp ora_comp

ora_l_op:	// 0xb5
	or reg_a, reg_l
	rjmp ora_comp

ora_m_op:	// 0xb6
	rcall load_hl
	or reg_a, reg_mem_r
	rjmp ora_comp

ora_a_op:	// 0xb7
	or reg_a, reg_a
	rjmp ora_comp

cmp_b_op:	// 0xb8
	mov reg_tmp1, reg_a
	sub reg_tmp1, reg_b
	rjmp cmp_comp

cmp_c_op:	// 0xb9
	mov reg_tmp1, reg_a
	sub reg_tmp1, reg_c
	rjmp cmp_comp

cmp_d_op:	// 0xba
	mov reg_tmp1, reg_a
	sub reg_tmp1, reg_d
	rjmp cmp_comp

cmp_e_op:	// 0xbb
	mov reg_tmp1, reg_a
	sub reg_tmp1, reg_e
	rjmp cmp_comp

cmp_h_op:	// 0xbc
	mov reg_tmp1, reg_a
	sub reg_tmp1, reg_h
	rjmp cmp_comp

cmp_l_op:	// 0xbd
	mov reg_tmp1, reg_a
	sub reg_tmp1, reg_l
	rjmp cmp_comp

cmp_m_op:	// 0xbe
	rcall load_hl
	mov reg_tmp1, reg_a
	sub reg_tmp1, reg_mem_r
	rjmp cmp_comp

cmp_a_op:	// 0xbf
	mov reg_tmp1, reg_a
	sub reg_tmp1, reg_a
	rjmp cmp_comp

rnz_op:		// 0xc0
	sbrc reg_f, B_Z
	rjmp fetch_next
	rjmp ret_op

pop_b_op:	// 0xc1
	rcall pop_start
	mov reg_c, reg_mem_l
	mov reg_b, reg_mem_r
	rjmp fetch_next

jnz_op:		// 0xc2
	sbrs reg_f, B_Z
	rjmp jmp_op
	ldi reg_tmp1, 2
	add reg_pc_l, reg_tmp1
	adc reg_pc_h, r1
	rjmp fetch_next

jmp_op:		// 0xc3
  	rcall fetch_op
	mov reg_tmp2, reg_mem_r
	rcall fetch_op
	mov reg_pc_h, reg_mem_r
	mov reg_pc_l, reg_tmp2
	rjmp fetch_next

cnz_op:		// 0xc4
	sbrs reg_f, B_Z
	rjmp call_op
	ldi reg_tmp1, 2
	add reg_pc_l, reg_tmp1
	adc reg_pc_h, r1
	rjmp fetch_next

push_b_op:	// 0xc5
	mov reg_mem_l, reg_c
	mov reg_mem_r, reg_b
	rjmp push_comp

adi_op:		// 0xc6
	rcall fetch_op
	add reg_a, reg_mem_r
	rjmp add_comp

rst_00_op:	// 0xc7
	ldi reg_tmp1, 0x00
	rjmp rst_comp

rz_op:		// 0xc8
	sbrs reg_f, B_Z
	rjmp fetch_next
//	rjmp ret_op

ret_op:		// 0xc9
	rcall load_sp
	mov reg_pc_l, reg_mem_r
	ldi reg_tmp2, 1
	add reg_sp_l, reg_tmp2
	adc reg_sp_h, r1
	rcall load_sp
	mov reg_pc_h, reg_mem_r
	add reg_sp_l, reg_tmp2
	adc reg_sp_h, r1
	rjmp fetch_next

jz_op:		// 0xca
	sbrc reg_f, B_Z
	rjmp jmp_op
	ldi reg_tmp1, 2
	add reg_pc_l, reg_tmp1
	adc reg_pc_h, r1
	rjmp fetch_next

cz_op:		// 0xcc
	sbrc reg_f, B_Z
	rjmp call_op
	ldi reg_tmp1, 2
	add reg_pc_l, reg_tmp1
	adc reg_pc_h, r1
	rjmp fetch_next

call_op:	// 0xcd
	rcall fetch_op
	mov reg_mem_l, reg_mem_r
	rcall fetch_op
	mov reg_mem_h, reg_mem_r
	ldi reg_tmp2, 1
	sub reg_sp_l, reg_tmp2
	sbc reg_sp_h, r1
	mov reg_mem_r, reg_pc_h
	rcall store_sp
	sub reg_sp_l, reg_tmp2
	sbc reg_sp_h, r1
	mov reg_mem_r, reg_pc_l
	rcall store_sp
	mov reg_pc_l, reg_mem_l
	mov reg_pc_h, reg_mem_h
	rjmp fetch_next

aci_op:		// 0xce
	rcall fetch_op
	mov reg_tmp1, reg_mem_r
	rjmp adc_comp

rst_08_op:	// 0xcf
	ldi reg_tmp1, 0x08
	rjmp rst_comp

rnc_op:		// 0xd0
	sbrc reg_f, B_CY
	rjmp fetch_next
	rjmp ret_op

pop_d_op:	// 0xd1
	rcall pop_start
	mov reg_e, reg_mem_l
	mov reg_d, reg_mem_r
	rjmp fetch_next

jnc_op:		// 0xd2
	sbrs reg_f, B_CY
	rjmp jmp_op
	ldi reg_tmp1, 2
	add reg_pc_l, reg_tmp1
	adc reg_pc_h, r1
	rjmp fetch_next

out_op:		// 0xd3
	rcall fetch_op
	push_regs
	mov r24, reg_mem_r
	mov r22, reg_a
	rcall out
	pop_regs
	rjmp fetch_next

cnc_op:		// 0xd4
	sbrs reg_f, B_CY
	rjmp call_op
	ldi reg_tmp1, 2
	add reg_pc_l, reg_tmp1
	adc reg_pc_h, r1
	rjmp fetch_next

push_d_op:	// 0xd5
	mov reg_mem_l, reg_e
	mov reg_mem_r, reg_d
	rjmp push_comp

sui_op:		// 0xd6
	rcall fetch_op
	sub reg_a, reg_mem_r
	rjmp sub_comp

rst_10_op:	// 0xd7
	ldi reg_tmp1, 0x10
	rjmp rst_comp

rc_op:		// 0xd8
	sbrs reg_f, B_CY
	rjmp fetch_next
	rjmp ret_op

jc_op:		// 0xda
	sbrc reg_f, B_CY
	rjmp jmp_op
	ldi reg_tmp1, 2
	add reg_pc_l, reg_tmp1
	adc reg_pc_h, r1
	rjmp fetch_next

in_op:		// 0xdb
	rcall fetch_op
	push_regs
	mov r24, reg_mem_r
	rcall in
	mov reg_a, r24
	pop_regs
	rjmp fetch_next

cc_op:		// 0xdc
	sbrc reg_f, B_CY
	rjmp call_op
	ldi reg_tmp1, 2
	add reg_pc_l, reg_tmp1
	adc reg_pc_h, r1
	rjmp fetch_next

sbi_op:		// 0xde
	rcall fetch_op
	mov reg_tmp1, reg_mem_r
	rjmp sbb_comp

rst_18_op:	// 0xdf
	ldi reg_tmp1, 0x18
	rjmp rst_comp

rpo_op:		// 0xe0
	sbrc reg_f, B_P
	rjmp fetch_next
	rjmp ret_op

pop_h_op:	// 0xe1
	rcall pop_start
	mov reg_l, reg_mem_l
	mov reg_h, reg_mem_r
	rjmp fetch_next

jpo_op:		// 0xe2
	sbrs reg_f, B_P
	rjmp jmp_op
	ldi reg_tmp1, 2
	add reg_pc_l, reg_tmp1
	adc reg_pc_h, r1
	rjmp fetch_next

xthl_op:	// 0xe3
	mov reg_mem_l, reg_sp_l
	mov reg_mem_h, reg_sp_h
	rcall load
	mov reg_tmp1, reg_l
	mov reg_l, reg_mem_r
	mov reg_mem_r, reg_tmp1
	rcall store
	ldi reg_tmp1, 1
	add reg_mem_l, reg_tmp1
	adc reg_mem_h, r1
	rcall load
	mov reg_tmp1, reg_h
	mov reg_h, reg_mem_r
	mov reg_mem_r, reg_tmp1
	rcall store
	rjmp fetch_next

cpo_op:		// 0xe4
	sbrs reg_f, B_P
	rjmp call_op
	ldi reg_tmp1, 2
	add reg_pc_l, reg_tmp1
	adc reg_pc_h, r1
	rjmp fetch_next

push_h_op:	// 0xe5
	mov reg_mem_l, reg_l
	mov reg_mem_r, reg_h
	rjmp push_comp

ani_op:		// 0xe6
	rcall fetch_op
	and reg_a, reg_mem_r
	rjmp ana_comp

rst_20_op:	// 0xe7
	ldi reg_tmp1, 0x20
	rjmp rst_comp

rpe_op:		// 0xe8
	sbrc reg_f, B_P
	rjmp ret_op
	rjmp fetch_next

pchl_op:	// 0xe9
	mov reg_pc_l, reg_l
	mov reg_pc_h, reg_h
	rjmp fetch_next

jpe_op:		// 0xea
	sbrc reg_f, B_P
	rjmp jmp_op
	ldi reg_tmp1, 2
	add reg_pc_l, reg_tmp1
	adc reg_pc_h, r1
	rjmp fetch_next

xchg_op:	// 0xeb
	mov reg_tmp1, reg_h
	mov reg_h, reg_d
	mov reg_d, reg_tmp1
	mov reg_tmp1, reg_l
	mov reg_l, reg_e
	mov reg_e, reg_tmp1
	rjmp fetch_next

cpe_op:		// 0xec
	sbrc reg_f, B_P
	rjmp call_op
	ldi reg_tmp1, 2
	add reg_pc_l, reg_tmp1
	adc reg_pc_h, r1
	rjmp fetch_next

xri_op:		// 0xee
	rcall fetch_op
	eor reg_a, reg_mem_r
	rjmp xra_comp

rst_28_op:	// 0xef
	ldi reg_tmp1, 0x28
	rjmp rst_comp

rp_op:		// 0xf0
	sbrc reg_f, B_S
	rjmp fetch_next
	rjmp ret_op

pop_psw_op:	// 0xf1
	rcall pop_start
	mov reg_f, reg_mem_l
	mov reg_a, reg_mem_r
	rjmp fetch_next

jp_op:		// 0xf2
	sbrs reg_f, B_S
	rjmp jmp_op
	ldi reg_tmp1, 2
	add reg_pc_l, reg_tmp1
	adc reg_pc_h, r1
	rjmp fetch_next

di_op:		// 0xf3
	rjmp fetch_next

cp_op:		// 0xf4
	sbrs reg_f, B_S
	rjmp call_op
	ldi reg_tmp1, 2
	add reg_pc_l, reg_tmp1
	adc reg_pc_h, r1
	rjmp fetch_next

push_psw_op:	// 0xf5
	mov reg_mem_l, reg_f
	mov reg_mem_r, reg_a
	rjmp push_comp

ori_op:		// 0xf6
	rcall fetch_op
	or reg_a, reg_mem_r
	rjmp ora_comp

rst_30_op:	// 0xf7
	ldi reg_tmp1, 0x30
	rjmp rst_comp

rm_op:		// 0xf8
	sbrs reg_f, B_S
	rjmp fetch_next
	rjmp ret_op

sphl_op:	// 0xf9
	mov reg_sp_l, reg_l
	mov reg_sp_h, reg_h
	rjmp fetch_next

jm_op:		// 0xfa
	sbrc reg_f, B_S
	rjmp jmp_op
	ldi reg_tmp1, 2
	add reg_pc_l, reg_tmp1
	adc reg_pc_h, r1
	rjmp fetch_next

ei_op:		// 0xfb
	rjmp fetch_next

cm_op:		// 0xfc
	sbrc reg_f, B_S
	rjmp call_op
	ldi reg_tmp1, 2
	add reg_pc_l, reg_tmp1
	adc reg_pc_h, r1
	rjmp fetch_next

cpi_op:		// 0xfe
	rcall fetch_op
	mov reg_tmp1, reg_a
	sub reg_tmp1, reg_mem_r
	rjmp cmp_comp

rst_38_op:	// 0xff
	ldi reg_tmp1, 0x38
	rjmp rst_comp

adc_comp:
	clc
	sbrc reg_f, B_CY
	sec
	adc reg_a, reg_tmp1
//	rjmp add_comp

add_comp:
sub_comp:
	in reg_tmp1, SREG
	ldi reg_f, 0
	brcc 1f
	sbr reg_f, F_CY
	out SREG, reg_tmp1
1:	brhc 2f
	sbr reg_f, F_AC
	out SREG, reg_tmp1
2:	brne 3f
	sbr reg_f, F_Z
	out SREG, reg_tmp1
3:	brpl 4f
	sbr reg_f, F_S
4:	mov reg_tmp1, reg_a
	rjmp par_set

cmp_comp:
	in reg_tmp2, SREG
	ldi reg_f, 0
	brcc 1f
	sbr reg_f, F_CY
	out SREG, reg_tmp2
1:	brhc 2f
	sbr reg_f, F_AC
	out SREG, reg_tmp2
2:	brne 3f
	sbr reg_f, F_Z
	out SREG, reg_tmp2
3:	brpl 4f
	sbr reg_f, F_S
4:	rjmp par_set

sbb_comp:
	clc
	sbrc reg_f, B_CY
	sec
	sez
	sbc reg_a, reg_tmp1
	rjmp sub_comp

ana_comp:
	ldi reg_f, F_AC
	brne 1f
	sbr reg_f, F_Z
	cp reg_a, r1
1:	brpl 2f
	sbr reg_f, F_S
2:	mov reg_tmp1, reg_a
	rjmp par_set

xra_comp:
ora_comp:
	ldi reg_f, 0
	brne 1f
	sbr reg_f, F_Z
	cp reg_a, r1
1:	brpl 2f
	sbr reg_f, F_S
2:	mov reg_tmp1, reg_a
	rjmp par_set

push_comp:
	ldi reg_tmp2, 1
	sub reg_sp_l, reg_tmp2
	sbc reg_sp_h, r1
	rcall store_sp
	sub reg_sp_l, reg_tmp2
	sbc reg_sp_h, r1
	mov reg_mem_r, reg_mem_l
	rcall store_sp
	rjmp fetch_next

pop_start:
	rcall load_sp
	mov reg_mem_l, reg_mem_r
	ldi reg_tmp2, 1
	add reg_sp_l, reg_tmp2
	adc reg_sp_h, r1
	rcall load_sp
	add reg_sp_l, reg_tmp2
	adc reg_sp_h, r1
	ret

rst_comp:
	mov reg_mem_l, reg_pc_l
	mov reg_mem_r, reg_pc_h
	mov reg_pc_h, r1
	mov reg_pc_l, reg_tmp1
	rjmp push_comp

dcr_comp:
	andi reg_f, F_CY
	mov reg_tmp2, reg_tmp1
	andi reg_tmp2, 0x0f
	cpi reg_tmp2, 0x0f
	brne flag_zsp
	sbr reg_f, F_AC
	rjmp flag_zsp

inr_comp:
	andi reg_f, F_CY
	mov reg_tmp2, reg_tmp1
	andi reg_tmp2, 0x0f
	brne 1f
	sbr reg_f, F_AC
flag_zsp:
1:	cp reg_tmp1, r1
	brne 2f
	sbr reg_f, F_Z
2:	mov reg_tmp2, reg_tmp1
	andi reg_tmp2, F_S
	or reg_f, reg_tmp2
//	rjmp par_set

par_set:
	mov reg_tmp2, r1
	lsr reg_tmp1
	adc reg_tmp2, r1
	lsr reg_tmp1
	adc reg_tmp2, r1
	lsr reg_tmp1
	adc reg_tmp2, r1
	lsr reg_tmp1
	adc reg_tmp2, r1
	lsr reg_tmp1
	adc reg_tmp2, r1
	lsr reg_tmp1
	adc reg_tmp2, r1
	lsr reg_tmp1
	adc reg_tmp2, r1
	lsr reg_tmp1
	adc reg_tmp2, r1
	andi reg_tmp2, 1
	brne 1f
	sbr reg_f, F_P
1:	rjmp fetch_next

	.size fetch, .-fetch
