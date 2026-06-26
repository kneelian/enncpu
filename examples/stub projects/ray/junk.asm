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

			; currently we discard result

			CEQ    A, #1
			SWPR.P B
			ADD.P  B, #1
			SWPR.P B

		POPS  A
		POPS  B