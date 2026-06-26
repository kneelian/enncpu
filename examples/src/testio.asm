.SEC %main
@start

MOVM H, #0xbf
WSP  H

MOVM H, #0xd0

MOV  G, H
SUB  G, #32

MOV  B, #16

@loopy
	DBGINC  A
	CEQ     A, #10
	MOV.P   B, #0
	CINV
	STRB    A, G+
	SUB.P   B, #1
	JMNZO B, @loopy
MOV   G, H
SUB   G, #32

JLA  @parse_fp24

MOV  B, #0
ADRL B, @string_a
ADRM B, @string_a
@stringloopa
	LDRB  C, B+
	DBGC  C
	JMNZO C, @stringloopa
MOV  B, #32
DBGC B

PSHS A

ADRL B, @print_fp24
ADRM B, @print_fp24
JLR  B

MOV  B, #32
DBGC B

ADRL B, @string_b
ADRM B, @string_b
@stringloopb
	LDRB  C, B+
	DBGC  C
	JMNZO C, @stringloopb
MOV  B, #32
DBGC B

POPS A
FLN  A

JLA  @print_fp24

MOV  B, #10
DBGC B

ERR

@parse_fp24
; G holds pointer to first
; H holds pointer to end of span
; returning fp24 in A
	MOV   G, H
	SUB   G, #32	
	MOV   D, #10
	FCNV  D, D
	MOV   A, #0
	MOV   B, #0
	MOV   E, #0
	MOV   F, #0

	FC1     E
	LDRB    B, G     ; check sign
	CEQ     B, #45
	FNEG.P  E, E
	ADD.P   G, #1
	
@parse_loop	
	LDRB    B, G+
	CEQ     B, #46
	JMO.P   @decimal
	CEQ     B, #10
	JMO.P   @newline
	CEQ     G, H
	JMO.P   @toofar
	CLT     B, #46
	JMO.P   @toofar

	SUB   B, #48    ; ascii of '0'
	FCNV  B, B
	FMUL  A, D      ; 10
	FADD  A, B
	JMO   @parse_loop

@decimal
	MOV   C, D       ; will keep powers of 10
@decimal_loop
	LDRB  B, G+
	CLT   B, #48
	JMO.P @newline
	CEQ   G, H
	JMO.P @toofar
	SUB   B, #48
	FCNV  B, B
	FDIV  B, C       ; / d ( == 10^n)
	FMUL  C, D       ; d * 10
	FADD  F, B
	JMO   @decimal_loop
@newline
	FADD   A, F
	FMUL   A, E
	RET
@toofar
	JUNKB
	FC1   A
	RET

; ----------
; input in A
; ----------
@print_fp24
	MOV   H, A
	MOV   B, #0
	FC0   C
	FCLT  A, C
	MOV.P B, #45
	DBGC  B
	FABS  A, A

	FCST  B, A
	MOV   C, #10
	PSHB  C

	@_L01
		MOV   D, B
		MOD   D, C      ; % 10
		PSHB  D
		DIV   B, C      ; / 10
		JMNZO B, @_L01
	@_L02
		POPB  D
		CEQ   D, #10
		JMO.P @_L02e
		ADD   D, #48
		DBGC  D
		JMO   @_L02
	@_L02e

	MOV   D, #46     ; decimal point
	DBGC  D
	FCST  B, A
	FCNV  B, B
	FSUB  A, B       ; trim whole part
	FCNV  C, C
	MOV   E, #5
	@_L03
		FMUL  A, C
		FCST  B, A
		ADD   B, #48
		DBGC  B
		SUB   B, #48
		FCNV  B, B
		FSUB  A, B
		SUB   E, #1
		JMNZO E, @_L03
	RET

@string_a
.ASCII the number input is 
.PAD

@string_b
.ASCII and its logarithm is
.PAD

%main
