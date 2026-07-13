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

; AB - CD
; returns AB
@SBC_I48
	SUB   B, D
	CGT   B, D
	SUB   A, C
	SUB.P A, #1
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

; takes int48 dividend in AB (big endian)
; takes int48 divisor in CD (big endian)
; returns int48 quotient in AB (big endian)
; returns int48 remainder in EF (big endian)
@DIV_I48
	CEQ   C, #0
	CEQ.P D, #0
	RET.P

    MOV   E, #0
    MOV   F, #0

    MOV   G, #48

@DIV_LOOP
    MOV   H, F
    LSHR  H, #23
    LSHL  E, #1
    BOR   E, H

    MOV   H, A
    LSHR  H, #23
    LSHL  F, #1
    BOR   F, H

    MOV   H, B
    LSHR  H, #23
    LSHL  A, #1
    BOR   A, H

    LSHL  B, #1

    CLT   E, C
 OR CLT   F, D
    JMO.P @DIV_SKIP

@DIV_SUB
    CLT   F, D 
    SUB   F, D
    SUB   E, C 
    SUB.P E, #1 
    ADD   B, #1

@DIV_SKIP
    SUB   G, #1
    CGT   G, #0
    JMO.P @DIV_LOOP

    RET

; bitwise
@BOR_I48
	BOR A, C
	BOR B, D
	RET
@BAND_I48
	BAND A, C
	BAND B, D
	RET
@BXOR_I48
	BXOR A, C
	BXOR B, D
	RET

; shifts
; amount in C
; trashes D, E
@LSHL_I48
	LSHL A, C
	MOV  D, B
	LSHL B, C
	MOV  E, #24
	SUB  E, C
	LSHR D, E
	BOR  A, D
	RET
@LSHR_I48
    LSHR B, C
    MOV  D, A
    LSHR A, C
    MOV  E, #24
    SUB  E, C
    LSHL D, E
    BOR  B, D
    RET
; AB = ~AB + 1
@NEG_I48
    INV   A, A
    INV   B, B
    ADD   B, #1
    CEQ   B, #0
    ADD.P A, #1
    RET

; tighter inc/dec
; to avoid setting up AB +/- CD
@INC_I48
    ADD   B, #1
    CLT   B, #1
    ADD.P A, #1
    RET
@DEC_I48
    SUB   B, #1
    CLT   B, #1
    SUB.P A, #1
    RET

; ditto for mul 2, div 2
@MUL2_I48
    MOV   H, B
    LSHR  H, #23
    LSHL  A, #1
    BOR   A, H
    LSHL  B, #1
    RET
@DIV2_I48
    MOV   H, A
    LSHL  H, #23
    LSHR  B, #1
    BOR   B, H
    LSHR  A, #1
    RET

; AB == CD
@CEQ_I48
	CEQ   A, C
	CEQ.P B, D
	MOV.P E, #1
	RET.P
	MOV   E, #0
	RET
; AB > CD
@CGT_I48
	MOV   E, #0
	CLT   A, C
	RET.P
	CGT   B, D
	ADD.P E, #1
	RET

; takes int48 dividend in AB (big endian)
; returns int48 quotient in AB (big endian)
; returns int48 remainder in EF (big endian)
@DIV10_I48
    MOV   C, #10

    MOV   E, A
    DIV   A, C

    MOV   F, A
    MULA  F, C
    SUB   E, F

    MOV   F, B
    LSHR  F, #12
    LSHL  E, #12
    BOR   E, F

    MOV   F, E
    DIV   E, C

    MOV   G, E
    MULA  G, C
    SUB   F, G

    MOV   G, B
    LSHL  G, #12
    LSHR  G, #12

    LSHL  F, #12
    BOR   F, G

    MOV   G, F
    DIV   F, C

    MOV   H, F
    MULA  H, C
    SUB   G, H

    LSHL  E, #12
    BOR   E, F
    MOV   B, E

    MOV   F, G
    MOV   E, #0
    RET

%PGA

.ORG 0x0200
.SEC %PGB

%PGB

.ORG 0x1000
.SEC %PGC
	@PROG

	ADRL A, @STK
	ADRM A, @STK
	WSP  A

	MOVL A, #0x4b
	MOVL B, #0xff
	MOVH B, #0xff

	JLA @DIV10_I48

	ERR

	MOVL C, #0x00
	MOVL D, #0xff
	MOVM D, #0xff
	MOVH D, #0x80

	JLA  @DIV_I48

	ERR

%PGC

.ORG 0x4000
@STK
