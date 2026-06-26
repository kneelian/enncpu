; -----------------------
; all routines here take
; a v3 input in ABC / xyz
; and return either a v3
; in ABC / xyz order, or
; a scalar in A
;
; those that take a scalar
; take it in D
; those that take another v3
; take it in DEF / xyz order
;
; those that take both v3 and
; a scalar take them in
; DEF(v3) G(f)
;
; routine list:
;	- v3_add_v3
;	- v3_sub_v3
;	- v3_mul_sca
;	- v3_div_sca
;	- v3_dot_v3
;	- v3_cx_v3    <-- cross
;	- v3_length
;	- v3_unitvec
;	- v3_pixellise
;	- v3_greyify  <-- turn into 8bit
;	- v3_to_zero
;
; routines do not aim to
; preserve any of the regs
; so back them up if needed
; -------------------------

.ORG 0x0d00
.SEC %vec3ops

@v3_add_v3
	FADD A, D
	FADD B, E
	FADD C, F
	RET

@v3_sub_v3
	FSUB A, D
	FSUB B, E
	FSUB C, F
	RET

@v3_mul_scal
	FMUL A, D
	FMUL B, D
	FMUL C, D
	RET

@v3_div_scal
	FREC D, D
	FMUL A, D
	FMUL B, D
	FMUL C, D
	RET

@v3_dot_v3
	FMUL A, D
	FMUL B, E
	FMUL C, F
	FADD A, B
	FADD A, C
	RET
	
@v3_cx_v3
	MOV  G, B
	FMUL G, F ; y1z2
	MOV  H, C
	FMUL H, E ; z1y2
	FSUB G, H
	PSHS G

	MOV  G, C
	FMUL G, D ; z1x2
	MOV  H, A
	FMUL H, F ; x1z2
	FSUB G, H
	PSHS G

	MOV  G, A
	FMUL G, E ; x1y2
	MOV  H, B
	FMUL H, D ; y1x2
	FSUB G, H

	MOV  C, G
	POPS B
	POPS A
	RET

@v3_length
	FMUL A, A
	FMUL B, B
	FMUL C, C
	FADD A, B
	FADD A, C
	FSQT A, A
	RET

@v3_unitvec
	MOV  D, A
	FMUL D, D
	MOV  E, B
	FMUL E, E
	FADD D, E
	MOV  E, C
	FMUL E, E
	FADD D, E
	FSQT D, D
	FDIV A, D
	FDIV B, D
	FDIV C, D ; unrolled div_sca
	RET

@v3_pixellise
	ADRM H, @v3_consts
	ADRL H, @v3_consts
	LDRS D, H
	FMUL A, D
	FMUL B, D
	FMUL C, D
	FCST A, A
	FCST B, B
	FCST C, C
	RET

@v3_to_zero
	MOV A, #0
	MOV B, #0
	MOV C, #0
	RET

@v3_to_unit_xyz
	FC1 A
	FC1 B
	FC1 C
	JLO @v3_unitvec
	RET
	
@v3_greyify	
	JLO  @v3_length
	ADRM H, @v3_consts
	ADRL H, @v3_consts
	ADD  H, #3
	LDRS H, H
	FMUL A, H
	FCST A, A	
	RET

@v3_consts
.FP24 64.999
.FP24 255.999
.PAD

%vec3ops
