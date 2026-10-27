.ORG 0x0000
.SEC %PGA

	MASK
	ADRL A, @PROG
	ADRM A, @PROG
	JMR  A

@ADC_I48
	FAR JMO @ADC_I48_M
@SBC_I48
	FAR JMO @SBC_I48_M
@MUL_I48
	FAR JMO @MUL_I48_M
@DIV_I48
	FAR JMO @DIV_I48_M
@BOR_I48
	FAR JMO @BOR_I48_M
@BAND_I48
	FAR JMO @BAND_I48_M
@BXOR_I48
	FAR JMO @BXOR_I48_M
@LSHL_I48
	FAR JMO @LSHL_I48_M
@LSHR_I48
	FAR JMO @LSHR_I48_M
@NEG_I48
	FAR JMO @NEG_I48_M
@INC_I48
	FAR JMO @INC_I48_M
@DEC_I48
	FAR JMO @DEC_I48_M
@MUL2_I48
	FAR JMO @MUL2_I48_M
@DIV2_I48
	FAR JMO @DIV2_I48_M
@CEQ_I48
	FAR JMO @CEQ_I48_M
@CGT_I48
	FAR JMO @CGT_I48_M
@DIV10_I48
	FAR JMO @DIV10_I48_M
@ISQRT_I48
	FAR JMO @ISQRT_I48_M
@UNPACK_FP48
	FAR JMO @UNPACK_FP48_M
@PACK_FP48
	FAR JMO @PACK_FP48_M
@NORMALISE_FP48
	FAR JMO @NORMALISE_FP48_M
@ADD_FP48
	FAR JMO @ADD_FP48_M
@SUB_FP48
	FAR JMO @SUB_FP48_M

; takes int48 in AB and CD
; (big endian)
; returns int48 in AB
; (big endian)
@ADC_I48_M
	ADD   A, C
	ADD   B, D
	CGT   D, B
	ADD.P A, #1

	RET

; AB - CD
; returns AB
@SBC_I48_M
	SUB   B, D
	CGT   B, D
	SUB   A, C
	SUB.P A, #1
	RET

; takes int48 in AB and CD
; (big endian)
; returns int96 in ABCD
; (big endian)
@MUL_I48_M
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
@DIV_I48_M
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
@BOR_I48_M
	BOR A, C
	BOR B, D
	RET
@BAND_I48_M
	BAND A, C
	BAND B, D
	RET
@BXOR_I48_M
	BXOR A, C
	BXOR B, D
	RET

; shifts
; amount in C
; trashes D, E
@LSHL_I48_M
	LSHL A, C
	MOV  D, B
	LSHL B, C
	MOV  E, #24
	SUB  E, C
	LSHR D, E
	BOR  A, D
	RET
@LSHR_I48_M
    LSHR B, C
    MOV  D, A
    LSHR A, C
    MOV  E, #24
    SUB  E, C
    LSHL D, E
    BOR  B, D
    RET
; AB = ~AB + 1
@NEG_I48_M
    INV   A, A
    INV   B, B
    ADD   B, #1
    CEQ   B, #0
    ADD.P A, #1
    RET

; tighter inc/dec
; to avoid setting up AB +/- CD
@INC_I48_M
    ADD   B, #1
    CEQ   B, #0
    ADD.P A, #1
    RET
@DEC_I48_M
	CEQ   B, #0
    SUB   B, #1
    SUB.P A, #1
    RET

; ditto for mul 2, div 2
@MUL2_I48_M
    MOV   H, B
    LSHR  H, #23
    LSHL  A, #1
    BOR   A, H
    LSHL  B, #1
    RET
@DIV2_I48_M
    MOV   H, A
    LSHL  H, #23
    LSHR  B, #1
    BOR   B, H
    LSHR  A, #1
    RET

; AB == CD
@CEQ_I48_M
	CEQ   A, C
	CEQ.P B, D
	MOV.P E, #1
	RET.P
	MOV   E, #0
	RET
; AB > CD
@CGT_I48_M
	MOV   E, #0
	CLT   A, C
	RET.P
	CGT   B, D
	ADD.P E, #1
	RET

; takes int48 dividend in AB (big endian)
; returns int48 quotient in AB (big endian)
; returns int48 remainder in EF (big endian)
@DIV10_I48_M
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

; takes int48 in AB (big endian)
; returns int48 root in B
@ISQRT_I48_M
    MOV   C, #0 
    MOV   D, #1
    LSHL  D, #23

	@ISQRT_LOOP
	    MOV   E, C
	    BOR   E, D
	    
	    MOV   F, E
	    MULA  F, E
	    MOV   H, E
	    MULB  H, E

		CEQ   H, A
	    CLE.P F, B
	 OR CLT   H, A
	    MOV.P C, E

	    LSHR  D, #1
	    JMNZO D, @ISQRT_LOOP

    MOV   B, C
    MOV   A, #0
    RET

; ------
; this is where the fp48 block starts
; 
; the fp48 format is
;
; 	eeeeeeee eeeeeeee
;	smmmmmmm mmmmmmmm
;	mmmmmmmm mmmmmmmm
; 16 bits of exponent
; 1 bit of sign
; 31 + 1 bit of mantissa
; 	with an implicit bit

; all operations on fp48
; have to break them up first
; so all op routines only
; take pre-broken floats
; using the below utility routines

; this function takes
; fp48 in [AB] big endian
; and breaks it up as
;   A - exponent
;   B - sign
;   C - mantissa high byte
;   D - mantissa low sesqui
@UNPACK_FP48_M
	MOV  C, A
	LSHR A, #8 ; now holds raw exponent
	MOV  D, B  ; no adjustment needed

	GETL  C, C
	CBIT  C, #7
	MOV   B, #0
	MOV.P B, #1 ; sign set
	SET   C, #7 ; always 1: implicit bit
	RET
; reverses above
@PACK_FP48_M
	LSHL  A, #8
	CLR   C, #7 ; implicit
	CNE   B, #0
	SET.P C, #7 ; encode sign
	BOR   A, C
	MOV   B, D
	RET

@NORMALISE_FP48_M
	CEQ    C, #0
	CEQ.P  D, #0
	MOV.P  A, #0 ; zero normalises
	; to exp = 0x0000
	RET.P ; early return

	PSHS   E

	LEAD   E, C
	CEQ    E, #16
	POPS.P E
	RET.P  ; early return
	; the float is already
	; normalised (clz = 16)

	PSHS F

	CGT    E, #16
	JMO.P  @NFP48_MANTISSA_RIGHT

	; first if the mantissa is too far to the left
	MOV    F, #16
	SUB    F, E   ; how far left?
	SWAP

	LITE   C
	LITE   D
	LITE   F

	MOV    A, C
	MOV    B, D
	MOV    C, F

	JLA    @LSHR_I48
	MOV    C, A
	MOV    D, B
	SWAP
	ADD    A, F   ; adjust exponent
	LITE   C      ; restore mantissa
	LITE   D

	POPS   F
	POPS   E
	RET

	@NFP48_MANTISSA_RIGHT
	; and now if it's to the right

	CEQ    E, #24 ; if top byte is fully empty
	MOV    F, #0
	LEAD.P F, D
	ADD    E, F
	SUB    E, #16 ; how far right?
	SWAP

	LITE   C
	LITE   D
	LITE   E

	MOV    A, C
	MOV    B, D
	MOV    C, E

	JLA @LSHL_I48
	MOV    C, A
	MOV    D, B
	SWAP
	SUB    A, E
	LITE   C
	LITE   D

	POPS F
	POPS E
	RET

; [A.B.CD] + [E.F.GH]
; running a tight ship
; with no registers to spare
@ADD_FP48_M
	CGT   E, A ; canonically order them
	XCG.P A, E
	XCG.P B, F
	XCG.P C, G
	XCG.P D, H ; now the first is bigger

	SWAP
	; shadow registry is full volatile

	LITE  A
	LITE  E
	SUB   A, E
	CGT   A, #32
	RET.P ; second too small to change first

	MOV   C, A
	LITE  G
	MOV   A, G
	LITE  H
	MOV   B, H

	JLA   @LSHR_I48 ; adjust mantissa

	SWAP
	CEQ   B, F

	SWAP  ; we need to know if we're adding
	MOV   H, #0
	MOV.P H, #1 ; store flag because 
	; @ADC_I48 overwrites conditionals

	LITE  C
	LITE  D

	XCG   A, C
	XCG   B, D

	CEQ H, #1
	JLA.P @ADC_I48
	CNE H, #1
	JLA.P @SBC_I48

	CBIT  A, #23
	INV.P A, A
	INV.P B, B
	MOV.P H, #2
	JLA.P @INC_I48

	MOV   C, A
	MOV   D, B
	SWAP
	LITE  C
	LITE  D

	LITE  H
	CEQ   H, #2
	TGL.P B, #0

	JLA   @NORMALISE_FP48
	RET

@SUB_FP48_M
	TGL F, #0
	JMA  @ADD_FP48

; abandon all hope
; ye who enter here
@DIV_FP48_M
	BXOR B, F ; sign
	PSHB B

	SWAP

	LITE E
	MOVL A, #0xfffe
	MOVM A, #0xfffe
	SUB  A, E ; new exponent

	PSHS A
	MOVL A, #0x800000
	MOVH A, #0x800000
	MOV  B, #0

	LITE G
	MOV C, G
	LITE H
	MOV D, H

	JLA @DIV_I48
	MOV C, #16
	JLA @LSHL_I48

	PSHS A
	PSHS B

	SWAP

	POPS H
	POPS G
	POPS E
	MOV B, #0
	MOV F, #0 ; pretending it's pos/pos

	POPB B
	RET

%PGA

.ORG 0x0600
.SEC %PGC
	@PROG

	ADRL A, @STK
	ADRM A, @STK
	WKS  A
	
	MOVL A, #0x800140 ; 6.0
	MOVM A, #0x800140
	MOVH A, #0x800140
	MOVL B, #0x000000

	JLA @UNPACK_FP48

	MOV  E, A
	MOV  F, B
	MOV  G, C
	MOV  H, D

	MOVL A, #0x800160 ; 7.0
	MOVM A, #0x800160
	MOVH A, #0x800160 
	MOVL B, #0x000000
	
	JLA @UNPACK_FP48

	DBGH

	JLA @SUB_FP48
	
	DBGH

	JLA @PACK_FP48

	ERR

%PGC

.ORG 0x4000
@STK
