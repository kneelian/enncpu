.ORG 0x0000
.SEC %PGA

	MASK
	ADRL A, @PROG
	ADRM A, @PROG
	JMR  A

$macro f48push 0
	PSHS A
	PSHS B
	PSHS C
	PSHS D
$endm

$macro f48pop 0
	POPS D
	POPS C
	POPS B
	POPS A
$endm

$macro f48dup 0
	MOV  E, A
	MOV  F, B
	MOV  G, C
	MOV  H, D
$endm

$macro f48drop 0
	JUNKS
	JUNKS
	JUNKS
	JUNKS
$endm

$macro f48exp 2
	MOVL m0, #m1
	MOVM m0, #m1
	MOVH m0, #0x00
$endm

$macro f48abcd 4
	MOVL A, #m0
	MOVM A, #m0
	MOVH A, #0x00
	MOV  B, #m1
	MOV  C, #m2
	MOVL D, #m3
	MOVM D, #m3
	MOVH D, #m3
$endm

$macro f48efgh 4
	MOVL E, #m0
	MOVM E, #m0
	MOVH E, #0x00
	MOV  F, #m1
	MOV  G, #m2
	MOVL H, #m3
	MOVM H, #m3
	MOVH H, #m3
$endm

; one Horner step: acc = acc*P + c
$macro f48step 4
	POPS H
	POPS G
	POPS F
	POPS E
	PSHS E
	PSHS F
	PSHS G
	PSHS H
	JLA  @MUL_FP48
	MOVL E, #m0
	MOVM E, #m0
	MOVH E, #0x00
	MOV  F, #m1
	MOV  G, #m2
	MOVL H, #m3
	MOVM H, #m3
	MOVH H, #m3
	JLA  @ADD_FP48
$endm

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
@MUL_FP48
	FAR JMO @MUL_FP48_M
@DIV_FP48
	FAR JMO @DIV_FP48_M
@SQRT_FP48
	FAR JMO @SQRT_FP48_M
@FP48_I16
	FAR JMO @FP48_I16_M
@FP48_U16
	FAR JMO @FP48_U16_M
@LN_FP48
	FAR JMO @LN_FP48_M
@SIN_FP48
	FAR JMO @SIN_FP48_M

; takes int48 in AB and CD
; (big endian)
; returns int48 in AB
; (big endian)
@ADC_I48_M
	ADD   A, C
	ADD   B, D
	CGE   D, B
	ADD.P A, #1
	RET

; AB - CD
; returns AB
@SBC_I48_M
	CLT   B, D
	SUB   B, D
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
    JMO.P @DIV_SKIP
    CGT   E, C
    JMO.P @DIV_SUB
    CLT   F, D
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
	CEQ    A, #0
	RET.P  ; early return
	; no normalisation for specials
	CEQ    C, #0
	CEQ.P  D, #0
	MOV.P  A, #0
	RET.P ; early return
	; normalises to exp = 0x0000

	PSHS   E

	LEAD   E, C
	CEQ    E, #16
	POPS.P E
	RET.P  ; early return
	; float is already ok (clz = 16)

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

	PSHB H
	CEQ H, #1
	JLA.P @ADC_I48
	POPB H
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

@MUL_FP48_M
	BXOR  B, F ; we just need the sign xor
	; this gives us a temp reg in F
	MOV   F, E
	CEQ   F, #0
 OR CEQ   A, #0
	MOV.P A, #0
	MOV.P C, #0
	MOV.P D, #0
	RET.P ; early escape
	; mul with zero or special

	ADD   A, E
	MOVL  F, #0x7fff
	MOVM  F, #0x7fff

	SUB   A, F

	; exponents summed

	SWAP
	LITE  C
	LITE  D
	LITE  G
	MOV   A, G
	LITE  H
	MOV   B, H
	JLA   @MUL_I48

	MOV   A, B
	MOV   B, C
	MOV   C, #7
	JLA   @LSHR_I48
	CBIT  A, #8
	MOV.P C, #1
	MOV   H, #0
	MOV.P H, #1
	JLA.P @LSHR_I48

	MOV   C, A
	MOV   D, B
	SWAP
	LITE  C
	LITE  D
	LITE  H
	ADD   A, H ; if we had to readjust exp

	JLA   @NORMALISE_FP48
	RET

; abandon all hope
; ye who enter here
;
; since we have int48 division
; that gives us the upper 16 bits
; and we can extract the lower 16
; by dividing the remainder with
; the same divisor and readjusting
@DIV_FP48_M
	BXOR B, F
	PSHB B

	; sign
	MOVL F, #0x7fff
	MOVM F, #0x7fff

	ADD A, F
	SUB A, E
	PSHW A
	; exponent

	MOV A, C
	MOV B, D
	PSHS G
	PSHS H
	MOV C, #15
	JLA @LSHL_I48

	POPS D
	POPS C
	JLA @DIV_I48

	PSHS C
	PSHS D
	PSHS E
	PSHS F
	MOV C, #16
	JLA @LSHL_I48

	POPS H
	POPS G ; remainder
	POPS D
	POPS C ; divisor

	PSHS A
	PSHS B ; upper 16 bits

	MOV A, G
	MOV B, H

	PSHS C
	PSHS D
	MOV C, #15
	JLA @LSHL_I48
	POPS D
	POPS C
	JLA @DIV_I48

	POPS E
	LSHL B, #1
	BOR B, E
	POPS A

	MOV C, A
	MOV D, B
	POPW A
	POPB B

	JLA @NORMALISE_FP48
	RET

; helper functions for logarithm and sine
@FP48_U16_M
	MOV  C, E
	LSHR C, #8 
	MOV  D, E
	LSHL D, #16
	_f48exp A 0x800e
	JLA  @NORMALISE_FP48
	RET
@FP48_I16_M
	MOV  B, #0
	CBIT E, #15
	JMO.P @FP48_I16_NEG 
	JLO @FP48_U16_M
	RET
@FP48_I16_NEG
	_f48exp H 0xffff
	SUB  H, E           
	ADD  H, #1
	MOV  E, H
	JLO @FP48_U16_M
	MOV  B, #1
	RET

@LN_FP48_M
		CNE  B, #0
		FAR JMO.P @LN_NAN  
		CEQ  A, #0
		FAR JMO.P @LN_ZERO 
		_f48exp F 0x7fff
		SUB  A, F           
		PSHW A              
		CGE  C, #0xb5
		FAR JMO.P @LN_HALVE 
		MOV  E, #0
		FAR JMO  @LN_QRDY
	@LN_HALVE
		MOV  E, #1
	@LN_QRDY
		POPW H
		ADD  H, E
		PSHW H
		_f48exp F 0x7fff
		SUB  F, E
		MOV  A, F
		MOV  B, #0

		_f48efgh 0x7fff 0 0x80 0x000000  
		JLA  @SUB_FP48
		_f48push
		_f48efgh 0x8000 0 0x80 0x000000 
		JLA  @ADD_FP48 
		_f48dup         
		_f48pop         
		JLA  @DIV_FP48 

		_f48push
		_f48dup
		JLA  @MUL_FP48 
		_f48push        

		_f48abcd 0x7ffb 0 0x9d 0x89d89e     ; 1/13
		_f48step 0x7ffb 0 0xba 0x2e8ba3     ; 1/11
		_f48step 0x7ffb 0 0xe3 0x8e38e4     ; 1/9
		_f48step 0x7ffc 0 0x92 0x492492     ; 1/7
		_f48step 0x7ffc 0 0xcc 0xcccccd     ; 1/5
		_f48step 0x7ffd 0 0xaa 0xaaaaab     ; 1/3
		_f48step 0x7fff 0 0x80 0x000000     ; 1

		_f48dup            
		_f48drop           
		_f48pop            
		JLA  @MUL_FP48    
		CNE  A, #0
		ADD.P A, #1
		POPW H            
		_f48push           
		CEQ  H, #0
		FAR JMO.P @LN_TAIL

		MOV  E, H
		JLA @FP48_I16    ; fp48(q)
		_f48dup           ; EFGH = q
		_f48abcd 0x7ffe 0 0xb1 0x7217f8     
						 ; ln2   ^
		JLA  @MUL_FP48   ; q*ln2
		_f48dup           ; EFGH = q*ln2
		_f48pop           ; ABCD = ln m
		JLA  @ADD_FP48   ; ln x
		RET

	@LN_TAIL
		_f48pop
		RET
	@LN_NAN
		_f48abcd 0x0000 0 0xff 0xffffff
		RET
	@LN_ZERO
		_f48abcd 0x0000 1 0x00 0x000000
		RET

; sqrt(2^e * m) = 
;	= 2^(e/2) * sqrt(m)
;		--> if e is even
;   = sqrt(2) * 2^((e-1)/2) * sqrt(m)
;		--> if e is odd
@SQRT_FP48_M
	CNE    B, #0
	MOV.P  A, #0
	MOVL.P C, #0xff
	MOVL.P D, #0xff
	RET.P
	; no root for negative number
	; early return
	CEQ    A, #0
	RET.P
	; return for specials and zero

	PSHS A
	PSHS C
	PSHS D ; save arg

	MOVL F, #0x7fff
	MOVM F, #0x7fff
	SUB  A, F
	MOV  E, #1
	BAND E, A
	LSHR A, #1
	ADD  A, F
	PSHS A
	; exponent

	MOV A, C
	MOV B, D
	MOV C, #15
	ADD C, E
	JLA @LSHL_I48
	JLA @ISQRT_I48
	MOV C, #8
	JLA @LSHL_I48

	MOV C, A
	MOV D, B
	POPS A
	MOV B, #0

	; first approximation of √n

	POPS H
	POPS G
	MOV F, #0
	POPS E ; arg

	XCG A, E
	XCG B, F
	XCG C, G
	XCG D, H

	PSHS E
	PSHS G
	PSHS H

	JLA @DIV_FP48

	POPS H
	POPS G
	MOV F, #0
	POPS E

	JLA @ADD_FP48

	SUB A, #1

	RET

@SIN_FP48_M
	CEQ  A, #0
	FAR JMO.P @SIN_ZERO

	PSHB B
	MOV  B, #0 
	_f48push    
	_f48efgh 0x7ffd 0 0xa2 0xf9836e
	JLA  @MUL_FP48

	_f48exp F 0x7ffe
	SUB  A, F               
	CBIT A, #15
	FAR JMO.P @SIN_SMALL    
	CGT  A, #15
	FAR JMO.P @SIN_RANGE    

	MOV  H, C
	LSHL H, #8
	MOV  C, D
	LSHR C, #16
	BOR  H, C              
	MOV  G, #16
	SUB  G, A              
	MOV  C, G
	SUB  C, #1
	MOV  E, #1
	LSHL E, C              
	ADD  E, H
	MOV  H, E
	LSHR H, G              

	_f48pop                 
	POPB G                 
	CBIT H, #0
	TGL.P G, #0            
	PSHB G                 
	MOV  B, #0             
	_f48push                
	PSHW H                 

	POPW E                 
	PSHW E
	JLA  @FP48_U16           
	_f48efgh 0x8000 0 0xc9 0x0f0000
	JLA  @MUL_FP48
	_f48dup          
	POPW G
	_f48pop          
	PSHW G          
	JLA  @SUB_FP48  

	POPW E          
	_f48push            
	JLA  @FP48_U16
	_f48efgh 0x7ff0 0 0xda 0xa22169
	JLA  @MUL_FP48
	_f48dup  
	_f48pop
	JLA  @SUB_FP48      
	FAR JMO  @SIN_SERIES

@SIN_SMALL
	_f48pop        

@SIN_SERIES
	_f48push       
	_f48dup
	JLA  @MUL_FP48
	_f48push       
	_f48abcd 0x7fd6 1 0xd7 0x3f9f3a ; seed
	_f48step 0x7fde 0 0xb0 0x92309d ; 1/13!
	_f48step 0x7fe5 1 0xd7 0x322b40 ; 1/11!
	_f48step 0x7fec 0 0xb8 0xef1d2b ; 1/9!
	_f48step 0x7ff2 1 0xd0 0x0d00d0 ; 1/7!
	_f48step 0x7ff8 0 0x88 0x888889 ; 1/5!
	_f48step 0x7ffc 1 0xaa 0xaaaaab ; 1/3!
	_f48step 0x7fff 0 0x80 0x000000 ; 1

	_f48dup        
	_f48drop       
	_f48pop        
	JLA  @MUL_FP48
	POPB G        
	CNE  G, #0
	TGL.P B, #0   
	RET

@SIN_ZERO
	_f48abcd 0x0000 0 0x00 0x000000
	RET
@SIN_RANGE
	_f48drop
	POPB A
	_f48abcd 0x0000 0 0xff 0xffffff
	RET

%PGA

.ORG 0x0a00
.SEC %PGC
	@PROG

	ADRL A, @STK
	ADRM A, @STK
	WKS  A

;	MOVL A, #0x800029  ; √7.0
;	MOVM A, #0x800029
;	MOVH A, #0x800029 
;	MOVL B, #0x53fd00
;	MOVM B, #0x53fd00
;	MOVH B, #0x53fd00
;
;	DBGFP48 A
;	
;	JLA @UNPACK_FP48
;
;	MOV E, A
;	MOV F, B
;	MOV G, C
;	MOV H, D

	MOVL A, #0x7ffe00  ; 0.5
	MOVM A, #0x7ffe00
	MOVH A, #0x7ffe00 
	MOVL B, #0x000000

	DBGFP48 A

	JLA @UNPACK_FP48
	JLA @SIN_FP48
	JLA @NORMALISE_FP48
	JLA @PACK_FP48

	DBGFP48 A

	ERR

%PGC

.ORG 0x4000
@STK
