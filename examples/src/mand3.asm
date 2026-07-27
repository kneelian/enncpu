.ORG 0x000
.SEC %PGA

	MASK

	MOVL A, #0
	MOVM A, #8
	WSP  A

	ADRL A, @EX_HANDLER
	ADRM A, @EX_HANDLER
	WXV  A

	ADRL A, @START
	ADRM A, @START

	; ERR

	JMR  A ;IP = RA;

@EX_HANDLER

	PSHS A
	PSHS B
	PSHS C
	RPS  C
	RXS  A

	CNE  A, #4

	WPS.P  C
	POPS.P C
	POPS.P B
	POPS.P A
	ERET.P

	MOVL B, #0x00
	MOVM B, #0xf4
	MOVH B, #0x87
	LDRB A, B

	DBGB A

	WPS  C
	MOV  C, #0
	WXS  C
	POPS C
	POPS B
	POPS A

	ERET

%PGA

.ORG 0x400
.SEC %main
@START

MOVH H, #0x80
MOVL E, #0x89
MOVM E, #0x02
MOVL F, #0x89
MOVM F, #0x01

MOVL B, #96

STRW E, H+
STRW F, H+
STRW B, H+

MOV  A, #2
STRW A, H+

DBGS H

SWAP

ADRL A, @CONST
ADRM A, @CONST

; D: stepx = (maxre - minre) / xsize (60)
; E: stepy = (maxim - minim) / ysize (20)

LDRS D, A+
LDRS H, A+
FSUB D, H
SWPR E
FCNV H, E
SWPR E
FDIV D, H

LDRS E, A+
LDRS H, A
FSUB E, H
SWPR F
FCNV H, F
SWPR F
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
		PSHW B
		PSHW C

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

			PSHW  A
			FC2   A
			FADD  A, A
			FCGT  H, A ; > 4.0?
			POPW  A
			JMO.P @LoopNEnd ; escape condition

			FMUL D, E ; zi := zi*zr
			FADD D, D ; *2
			FADD D, F

			MOV  E, C
			FSUB E, B
			FADD E, G
		SWPR  B
		CLT   A, B
		SWPR  B
		ADD.P A, #1
		JMO.P @LoopN
		@LoopNEnd

		SWPR H
		INV  A, A
		STRW A, H+
		SWPR H
	
		POPW C
		POPW B
		POPW A
		POPS E
		POPS D
	SWPR E
	CLT  C, E
	
	SWPR E
	ADD.P C, #1
	JMO.P @LoopY

SWPR   F
CLT    B, F
SWPR   F
ADD.P  B, #1
JMO.P  @LoopX

SWPR H

ERR

@CONST
.FP24 1.25 ; MaxRE
.FP24 -2.25 ; MinRE
.FP24 1.25  ; MaxIM
.FP24 -1.25 ; MinIM

%main
