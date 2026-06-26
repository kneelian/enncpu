.ORG 0
.SEC %main
@start
	MOVM A, #0xff
	WSP  A
	; set up stack

	MOV  A, #0
	MOVH A, #0x01

	MOVM B, #0x01
	MOVL B, #0x74

	MOVM C, #0x01
	MOVL C, #0x08

	MOV  D, #63

	STRW B, A+
	STRW C, A+
	STRW D, A+
	STRB D, A
	
	SUB  A, A

	ADRL A, @vp_arg1
	ADRM A, @vp_arg1

	ADRL D, @vp_arg2
	ADRM D, @vp_arg2

	ADRL E, @vp_arg3
	ADRM E, @vp_arg3

	FC2  G

	ADRL H, @hit_sphere
	ADRM H, @hit_sphere

	JLR  H

	MOVH B, #0
	MOVM B, #0x01
	MOVL B, #0x08  ; number of rows
	
	MOVH A, #0x00
	MOVM A, #0x01
	MOVL A, #0x74  ; pixels per row

	@loopy

		MOVH E, #0
		ADRL E, @vp_arg3_default 
		ADRM E, @vp_arg3_default
		LDRS H, E+		 ; ray start x
		LDRS C, E 		 ; ray start y

		MOVH F, #0
		ADRM F, @vp_delta_u
		ADRL F, @vp_delta_u
		LDRS F, F        ; delta_u->x

		FCNV D, A
		MOVH G, #0x00
		MOVM G, #0x01
		MOVL G, #0x74
		FCNV G, G        ; iteration nr

		FSUB G, D
		FMUL F, D

		FADD H, F

		MOVH F, #0
		ADRM F, @vp_delta_v
		ADRL F, @vp_delta_v
		ADD  F, #3		
		LDRS F, F        ; delta_v->y

		FCNV D, B
		MOVH G, #0x00
		MOVM G, #0x01
		MOVL G, #0x08
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

			ADRL A, @vp_arg1
			ADRL D, @vp_arg2
			ADRL E, @vp_arg3
			ADRM A, @vp_arg1
			ADRM D, @vp_arg2
			ADRM E, @vp_arg3
			ADRH A, @vp_arg1
			ADRH D, @vp_arg2
			ADRH E, @vp_arg3
			FC2  G

			ADRL H, @hit_sphere
			ADRM H, @hit_sphere
			ADRH H, @hit_sphere

			JLR  H

			MOV C, #0

			CEQ A, #1

			MOV.P C, #1

			POPS A
			POPS B
			PSHS B
			PSHS A

			ADRH E, @put_pixel
			ADRM E, @put_pixel
			ADRL E, @put_pixel

			JLR  E

			SWPR.P B
			ADD.P  B, #1
			SWPR.P B

		POPS  A
		POPS  B

		SUB   A, #1
		CEQ   A, #0

		SUB.P B, #1

		MVM.P A, #0x01
		MVL.P A, #0x74
		CNE   B, #0

		MVH.P E, @loopy
		MVM.P E, @loopy
		MVL.P E, @loopy

		JMR.P E

	SWPR B
	DBGS B
	SWPR B

	ERR

@test_temp
	.FP24 0.0 ; spherec
	.FP24 0.0
	.FP24 5.0
	.FP24 0.0 ; rayorig
	.FP24 0.0
	.FP24 0.0
	.FP24 0.0 ; raydir
	.FP24 0.0
	.FP24 1.0

@scratchpad
	.FP24 0
	.FP24 0
	.FP24 0

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
	.FP24 0.0
	.FP24 5.0
@eye_point
@vp_arg2
	.FP24 0.0 ; rayorigin
	.FP24 0.0
	.FP24 0.0
@vp_arg3
	.FP24 0.0 ; raydirec
	.FP24 0.0
	.FP24 1.0
@vp_arg3_default
	.FP24  -1.4090909 ; raydirec start
	.FP24  1.0
	.FP24  1.0
@vp_arg4
	.FP24 2.0 ; sphere_radius

@put_pixel
	;DBGW A
	;DBGW B
	;ERR
	
	MOV  D, #0
	MVM  D, #0x01

	MVL  D, #0x74

	MOV  E, D
	SUB  E, A
	MOV  A, E

	MVL  D, #0x08

	MOV  E, D
	SUB  E, B
	MOV  B, E

	MVH  F, #0x01
	MVM  F, #0x00
	MVL  F, #0x08

	MVL  D, #0x75

	MOV  E, D
	SUB  E, A
	MOV  A, E
	
	MULA D, A
	ADD  D, B
	ADD  F, D

	MOV  D, #16
	MULA C, D

	STRB C, F

	RET

%main

$include rayops.asm

.ORG 0x010000
@gfx_info
