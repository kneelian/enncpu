.ORG 0x0000
.SEC %PGA

	ADRL A, @PROG
	ADRM A, @PROG
	JMR  A

; takes int48 in AB and CD
; (big endian)
; returns int48 in AB
; (big endian)
@ADC_I48
	ADD   A, C
	ADD   B, D
	CGT   D, B
	ADD.P A, #1

	RET

; takes int48 in AB and CD
; (big endian)
; returns int96 in ABCD
; (big endian)
@MUL_I48
	MOV  E, B
	MULA E, D
	PSHS E

	MOV   H, #0
	MOV   E, B
	MULB  E, D
	MOV   F, A
	MULA  F, D
	ADD   E, F
	CLT   E, F
	ADD.P H, #1
	MOV   F, B
	MULA  F, C
	ADD   E, F
	CLT   E, F
	ADD.P H, #1

	MOV   G, #0
	MULB  D, A
	ADD   D, H
	CLT   D, H
	ADD.P G, #1
	MULB  B, C
	ADD   B, D
	CLT   B, D
	ADD.P G, #1
	MOV   H, A
	MULA  H, C
	ADD   B, H
	CLT   B, H
	ADD.P G, #1

	MULB  A, C
	ADD   A, G
	MOV   C, E
	POPS  D

	RET

; AB - CD
; returns AB
@SBC_I48
	SUB   B, D
	CGT   B, D
	SUB   A, C
	SUB.P A, #1
	RET

; AB >= CD
; 	returns 1/0 in E
@CGE_I48
	MOV   E, #0
	CGT   A, C
	ADD.P E, #1
	RET.P
	CLT   A, C
	RET.P ; else equal
	CGT   B, D
OR  CEQ   B, D
	ADD.P E, #1
	RET

@DIV_I48
; AB / CD
; returns AB quo, CD rem
;	while R >= D
;		R = R - D
;		Q = Q + 1
;   AB = Q, CD = R
	CEQ   C, #0
	CEQ.P D, #0
	MOV.P A, #0
	MOV.P B, #0
	RET.P ; escape hatch
FAR JMO @DIV_I48_MAIN
	; too long for PGA
	RET
	
%PGA

.ORG 0x0200
.SEC %PGB

@DIV_I48_MAIN

	RET

%PGB

.ORG 0x1000
.SEC %PGC
	@PROG

	ADRL A, @STK
	ADRM A, @STK
	WSP  A

	MOVL A, #0x2b
	MOVL B, #0xff
	MOVH B, #0xff

	MOVL C, #0x3b
	MOVL D, #0xff
	MOVL D, #0xff
	MOVH D, #0x80

	JLA  @MUL_I48

	ERR

%PGC

.ORG 0x4000
@STK
