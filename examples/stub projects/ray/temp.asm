	ERR

	ADRM H, @height
	ADRL H, @height
	LDRW B, H
	SUB  B, #1
	@loop_y
		ADRL H, @width
		ADRM H, @width
		LDRW A, H
		SUB  A, #1

		@loop_x
			PSHW  A
			PSHW  B

			ADRL H, @width_f24
			ADRM H, @width_f24
			LDRS D, H+
			LDRS E, H  ;height

			FSUB D, A
			FSUB E, B

			ADRL H, @vp_delta_u
			ADRM H, @vp_delta_u
			LD3S ABC, H
			FMUL A, D
			FMUL B, D
			FMUL C, D
			ADRL H, @scratchpad
			ST3S ABC, H

			ADRL H, @vp_delta_v
			ADRM H, @vp_delta_v
			LD3S ABC, H
			FMUL A, E
			FMUL B, E
			FMUL C, E

			ADRL H, @pix00loc
			ADRM H, @pix00loc
			LD3S DEF, H
			FADD A, D
			FADD B, E
			FADD C, F

			ADRL H, @scratchpad
			ADRM H, @scratchpad
			LD3S DEF, H
			FADD A, D
			FADD B, E
			FADD C, F

			FNEG A, A
			FNEG B, B
			FNEG C, C

			ADRL H, @vp_arg3
			ADRM H, @vp_arg3
			ST3S ABC, H

			MOV  A, #0
			MOV  D, #0
			MOV  E, #0
			

			ADRL A, @vp_arg1
			ADRL D, @vp_arg2
			ADRL E, @vp_arg3
			ADRM A, @vp_arg1
			ADRM D, @vp_arg2
			ADRM E, @vp_arg3

			ADRL H, @hit_sphere
			ADRM H, @hit_sphere

			FC2  G

			JLR  H

			; currently we discard result

			CEQ    A, #1
			SWPR.P B
			ADD.P  B, #1
			SWPR.P B

			POPW  B
			POPW  A

			SUB     A, #1
			CNE     A, #0
			ADRL.P  H, @loop_x
			ADRM.P  H, @loop_x
			JMR.P   H

		SUB     B, #1
		CNE     B, #0
		ADRL.P  H, @loop_y
		ADRM.P  H, @loop_y
		JMR.P   H

SWPR B
DBGS B

ERR