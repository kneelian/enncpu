@START

SUB A, A
SUB B, B
SUB C, C
SUB D, D
SUB E, E
SUB F, F
SUB G, G
SUB H, H

SUB  A, A
MOVM A, #4
WSP  A

ADRL A, @CONST
ADRM A, @CONST

; D: stepx = (maxre - minre) / xsize (60)
; E: stepy = (maxim - minim) / ysize (20)
LDRS D, A+
LDRS H, A+
FSUB D, H
MOV  H, #0
MOVL H, #96
FCNV H, H
FDIV D, H

LDRS E, A+
LDRS H, A
FSUB E, H
MOV  H, #32
FCNV H, H
FDIV E, H

; for(y = 0; y < YSize; y++)
SUB  B, B
@LoopX
	FCNV F, B ; fp(y)
	FMUL F, E ; *stepy
	LDRS H, A
	FADD F, H ; +MinIM = im

	; for(x = 0; x < XSize; x++)
	SUB  C, C
	@LoopY
		FCNV G, C ; fp(x)
		FMUL G, D ; *stepx
		SUB  A, #6
		LDRS H, A
		ADD  A, #6
		FADD G, H ; +MinRE = re

		PSHS D
		PSHS E
		PSHW A
		PSHB B
		PSHB C

		MOV  D, F ; Zi
		MOV  E, G ; Zr

		SUB  A, A
		@LoopN
			MOV  B, D
			FMUL B, B ; Zi^2
			MOV  C, E
			FMUL C, C ; Zr^2
			MOV  H, B
			FADD H, C ; Zi^2 + Zr^2

			PSHB  A
			FC2   A
			FADD  A, A
			FCGT  H, A ; > 4.0?
			POPB  A
			JMO.P @LoopNEnd ; escape condition

			FMUL D, E ; zi := zi*zr
			FADD D, D ; *2
			FADD D, F

			MOV  E, C
			FSUB E, B
			FADD E, G
		CLT   A, #30
		ADD.P A, #1
		JMO.P @LoopN
		@LoopNEnd

		MOV  B, #62
		SUB  B, A
	;	DBGC B
		
		POPB C
		POPB B
		POPW A
		POPS E
		POPS D
	MOV   H, #0
	MOVL  H, #96
	CLT   C, H
	ADD.P C, #1
	JMO.P @LoopY

	MOV  C, #10 ; newline
	;DBGC C
CLT   B, #32
ADD.P B, #1
JMO.P @LoopX

JMA @START

@CONST
.FP24 1.0  ; MaxRE
.FP24 -2.0 ; MinRE
.FP24 1.0  ; MaxIM
.FP24 -1.0 ; MinIM
