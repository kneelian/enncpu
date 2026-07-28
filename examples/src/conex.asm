.ORG 0x0000
.SEC %PGA
	UNMASK

	ADRL A, @STK
	ADRM A, @STK
	WSP  A

	ADRL A, @EXVEC
	ADRM A, @EXVEC
	WXV  A

	ADRL A, @MAIN
	ADRM A, @MAIN

	JMR  A

@DRAW_CHAR
	SWAP

	ADRL A, @OFFSET
	ADRM A, @OFFSET
	LDRW B, A
	ADD  B, #8
	STRW B, A

	LITE A     ; which char?
	; in the future
	;	B - x position of char
	;   C - y position of char

	LSHL A, #3 ; offset in table

	ADRL B, @FONT8x8
	ADRM B, @FONT8x8

	ADD  B, A     ; position of bitmap in font

	ADRL C, #0x00
	ADRH C, #0x80 ; start of framebuffer

	MOVL D, #0x8A
	MOVM D, #0x02
	LSHL D, #1    ; bytes per row @ 16bpp

	MOVL E, #200
	MULA E, D     ; random position top left
	ADD  C, E     ; where in framebuffer
	MOVL E, #200
	ADD  C, E     ; and x offset

	ADRL A, @OFFSET
	ADRM A, @OFFSET
	LDRW E, A
	ADD  C, E
	ADD  C, E ; 16bpp

	MOV  E, #8
	@DRAW_OUTER
		MOV  F, #8
		LDRB G, B+  ; G now has full byte
		@DRAW_INNER
			MOV   H, G
			BAND  H, #1
			SUB   H, #1
			LSHR  G, #1
			STRW  H, C+
			SUB   F, #1
			JMNZO F, @DRAW_INNER 

		SUB   E, #1
		ADD   C, D  ; next row
		SUB   C, #16
		JMNZO E, @DRAW_OUTER

	SWAP
	RET

@KILL
	ERR

@EXVEC
	PSHS A
	PSHS B

	RXS    A
	CNE    A, #4
	POPS.P B
	POPS.P A
	JMO.P  @KILL

	; for some reason
	; currently enters int
	; draws character fine
	; but overwrites it
	; immediately after exit?

	ADRL B, #0x00
	ADRM B, #0xf4
	ADRH B, #0x87

	LDRB A, B
	MOVL B, #0x51

	CEQ   A, B
	JMO.P @KILL

	JLA   @DRAW_CHAR

	DBGB A

	MOV  A, #0
	WXS  A

	POPS B
	POPS A

	ERET

@OFFSET
.INT16 0x00

%PGA

.ORG 0x0400
.SEC %PROG

@MAIN

	ADRL B, #0x00
	ADRM B, #0x00
	ADRH B, #0x80

	MOVL A, #0x8a
	MOVM A, #0x02

	MOVL C, #0x90
	MOVM C, #0x01
	MULA A, C
	MOV  C, #0xff
	MOVM C, #0xff

	@PAINT
		STRW  C, B+
		SUB   A, #1
		JMNZO A, @PAINT

	@LOOP
		JMO @LOOP

	ERR

%PROG

.ORG 0x1000
@STK

.ORG 0x2000
.SEC %FONTS

$include font8.enn

%FONTS