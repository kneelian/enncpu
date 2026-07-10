.ORG 0x0000
.SEC %PGA

	ADRL A, @STK
	ADRM A, @STK
	WSP  A

	ADRL A, @PROG
	ADRM A, @PROG
	JMR  A

	ERR

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

%PGA

.ORG 0x0400
.SEC %PGB
	@PROG

	MOVL B, #0xfe
	MOVM B, #0xff
	MOVH B, #0xff

	MOV  A, #3

	MOV  D, #16
	MOV  C, #3

	JLA  @ADC_I48

	ERR

%PGB

.ORG 0x4000
@STK
