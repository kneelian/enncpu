.ORG 0
.SEC %main
@start
	MOVM A, #0xff
	WSP  A
	; set up stack

	; going to keep location of
	; next pixel to write in reg
	; F and swap it in/out as
	; required out of shadow
	
	MOVL F, #0x8
	MOVH F, #0x1
	SWPR F
		
	SUB  A, A

	MOVL B, #0x08
	MOVM B, #0x01  ; number of rows + 1
	
	MOVL A, #0x74  ; pixels per row + 1
	MOVM A, #0x01

	@loopy
		ADRL E, @vp_arg3_default 
		ADRM E, @vp_arg3_default
		LDRS H, E+		 ; ray start x
		LDRS C, E 		 ; ray start y

		ADRL F, @vp_delta_u
		ADRM F, @vp_delta_u
		LDRS F, F        ; delta_u->x

		FCNV D, A
		MOVL G, #0x74
		MOVM G, #0x01
		FCNV G, G        ; iteration nr

		FSUB G, D
		FMUL F, D

		FADD H, F

		ADRL F, @vp_delta_v
		ADRM F, @vp_delta_v
		ADD  F, #3		
		LDRS F, F        ; delta_v->y

		FCNV D, B

		MOVL G, #0x09
		MOVM G, #0x01
		FCNV G, G        ; iteration nr

		FSUB G, D
		FMUL F, D

		FADD C, F        ; ray position rn

		ADRL E, @vp_arg3
		ADRM E, @vp_arg3
		ADRH E, @vp_arg3
		STRS H, E+
		STRS C, E

		PSHS B
		PSHS A

			ADRL  A, @vp_arg1
			ADRL  D, @vp_arg2
			ADRL  E, @vp_arg3

			ADRM  A, @vp_arg1
			ADRM  D, @vp_arg2
			ADRM  E, @vp_arg3

			ADRH  A, @vp_arg1
			ADRH  D, @vp_arg2
			ADRH  E, @vp_arg3
			FC2   G

			ADRL  H, @hit_sphere
			ADRM  H, @hit_sphere
			ADRH  H, @hit_sphere

			JLR   H

			ADRL E, @put_pixel
			ADRM E, @put_pixel
			ADRH E, @put_pixel

			JLR  E

		POPS  A
		POPS  B

		SUB   A, #1
		CEQ   A, #0

		SUB.P B, #1

		MVM.P A, #0x01
		MVL.P A, #0x74
		CNE   B, #0

		MVL.P E, @loopy
		MVM.P E, @loopy
		JMR.P E
		
	SWPR F
	ERR

@put_pixel
	JMNZO A, @get_color

	MOV   C, #0
	CNE   A, #0
	MOV.P C, #1

	SWPR F
	LSHL C, #5
	STRW C, F+
	SWPR F

	RET

; takes nonzero fp24
; in A
@get_color
	ADRL H, @hit_sphere_tempb
	ADRM H, @hit_sphere_tempb

	LD3S BCD, H+ ; ray origin
	LD3S EFG, H+ ; ray direction

	FMUL E, A
	FMUL F, A
	FMUL G, A

	FADD B, E ; hit is at point (B,C,D)
	FADD C, F
	FADD D, G

	FC1  E
	FADD D, E ; unexplained on page?

	MOV  E, B
	FMUL E, E
	MOV  F, C
	FMUL F, F
	FADD E, F
	MOV  F, D
	FMUL F, F
	FADD E, F
	FSQT E, E
	FDIV B, E
	FDIV C, E
	FDIV D, E ; unitvectorise

	FC1  E
	FADD B, E
	FADD C, E
	FADD D, E
	FC2  E
	FDIV B, E
	FDIV C, E
	FDIV D, E

	MOV  E, #0
	ADRL E, @pix_mul
	ADRM E, @pix_mul
	LDRS E, E

	FMUL B, E
	FMUL C, E
	FMUL D, E
	
	FCST B, B
	FCST C, C
	FCST D, D
	
	LSHR B, #3
	LSHR C, #2
	LSHR D, #3

	LSHL B, #11
	LSHL C, #5
	BOR  C, B
;	MOV  D, #0
	BOR  C, D

;	DBGW C

	SWPR F
	STRW C, F+
	SWPR F

;	ERR
	
	RET

@pix_mul
	.FP24 255.99
	
@width
.INT16 372
@height
.INT16 264

@width_f24
	.FP24 372.0
@height_f24
	.FP24 264.0

@aspect_ratio
	.FP24 1.4090909 ; 372/264

@vp_height
	.FP24 2.0
@vp_width
	.FP24 2.8181818 ; 2 * 372/264

@vp_uvec
	.FP24 2.8181818 ; width
	.FP24 0.0 		; 0
	.FP24 0.0 		; 0
@vp_delta_u
	.FP24 0.0075757 ; width / 372
	.FP24 0.0       ;
	.FP24 0.0       ;

@vp_vvec
	.FP24 0.0       ;
	.FP24 -2.0      ; height
	.FP24 0.0       ;
@vp_delta_v
	.FP24 0.0       ;
	.FP24 -0.007575 ; height / 264
	.FP24 0.0       ;

@vp_upperleft
	.FP24 -1.409090 ;
	.FP24 -1.0      ;
	.FP24 1.0       ;

@pix00loc
	.FP24 -1.405302 ;
	.FP24 -1.003787 ;
	.FP24 1.0       ;

@vp_focallength
	.FP24 1.0

@vp_arg1
	.FP24 0.0 ; sphere_centre
	.FP24 0.35
	.FP24 -3.5
@eye_point
@vp_arg2
	.FP24 0.0 ; rayorigin
	.FP24 0.0
	.FP24 0.0
@vp_arg3
	.FP24  0.0 ; raydirec
	.FP24  0.0
	.FP24 -1.0
@vp_arg3_default
	.FP24 -1.4090909 ; raydirec start
	.FP24  1.0
	.FP24 -1.0
@vp_arg4
	.FP24 2.0 ; sphere_radius

%main

$include rayops.asm

.ORG 0x0fff8
.SEC %gfx

	; set up framebuffer
	; with the right magic
	;
	; first the width and height
.INT16 372
.INT16 264
	; set legacy graphic mode
	; to 63 = greyscale 6-bit
	; or
	; to 1 = 16bpp
.INT16 1
	; set scaling factor to 
	; either 1 (default)
	; or to 2 (currently the
	; only possible upscale)
.INT8  2

.INT8  0

%gfx
@gfx_info
