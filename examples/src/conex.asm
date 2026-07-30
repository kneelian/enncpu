$macro SAVEALL 0
	PSHS A
	PSHS B
	PSHS C
	PSHS D
	PSHS E
	PSHS F
	PSHS G
	PSHS H
$endm

$macro LOADALL 0
	POPS H
	POPS G
	POPS F
	POPS E
	POPS D
	POPS C
	POPS B
	POPS A
$endm

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

; A has source
; B has target
; C has count
@MEMCPY
	PSHS D
	@MEMCPY_LOOP
		LDRB  D, A+
		STRB  D, B+
		SUB   C, #1
		JMNZO C, @MEMCPY_LOOP
	POPS D
	RET

; A has source
; string must be zero-term
; return in A
@STRLEN
	PSHS B
	MOV  B, A
	MOV  A, #0
	PSHS C
	@STRLEN_LOOP
		LDRB  C, B+ 
		ADD   A, #1
		JMNZO C, @STRLEN_LOOP
	POPS C
	POPS B
	RET

; A has string x
; B has string y
; return in A
; 	0 if different
;   1 if equal
@STREQ
	PSHS C
	PSHS D
	@STREQ_L1
		LDRB  C, A+
		LDRB  D, B+

		; unequals
		CNE   C, D
		MOV.P A, #0
		JMO.P @STREQ_L1_E

		; reached the end?
		CEQ   C, #0
		MOV.P A, #1
		JMO.P @STREQ_L1_E

		JMO   @STREQ_L1
	@STREQ_L1_E
	POPS D
	POPS C
	RET

@KILL
	ERR

@EXVEC
	PSHS A
	PSHS B

	RPS    A
	PSHS   A

	RXS    A
	CNE    A, #4
	POPS.P A
	WPS.P  A
	POPS.P B
	POPS.P A
	JMO.P  @KILL

	ADRL B, #0x00
	ADRM B, #0xf4
	ADRH B, #0x87

	LDRB A, B
	MOVL B, #0x51

	CEQ   A, B
	JMO.P @KILL

FAR JLO   @PUTCHAR_CURSOR

	DBGB A

	MOV  A, #0
	WXS  A

	POPS A
	WPS  A

	POPS B
	POPS A

	ERET

; A has colour 16bpp
; B --> x
; C --> y
@DRAW_PX
	PSHS D
	PSHS E

	ADRL D, #0x00
	ADRH D, #0x80 ; start of framebuffer
	MOVL E, #0x8A
	MOVM E, #0x02
	LSHL E, #1    ; bytes per row @ 16bpp

	LSHL B, #1
	MULA C, E
	ADD  D, C
	ADD  D, B

	STRW A, D

	POPS E
	POPS D
	RET

; A is char
; B --> x
; C --> y
@DRAW_CHAR_KERN
	_SAVEALL

	ADRL D, #0x00
	ADRH D, #0x80 ; start of framebuffer
	MOVL E, #0x8A
	MOVM E, #0x02
	LSHL E, #1    ; bytes per row @ 16bpp

	LSHL B, #1
	MULA C, E
	ADD  D, C
	ADD  D, B

	LSHL A, #4 ; offset in table
	ADRL B, @FONT16x8
	ADRM B, @FONT16x8
	ADD  B, A  ; position of bitmap in font

	MOV  C, D
	MOV  D, E
	MOV  E, #16
	@DRAW_OUTER_KERN
		MOV  F, #8
		LDRB G, B+  ; G now has full byte
		ADD  C, #16
		@DRAW_INNER_KERN
			MOV     H, G
			BAND    H, #1
			CEQ     H, #1
			LSHR    G, #1
			SUB.P   H, #1
			INV.P   H, H
			STRW.P  H, C
			SUB     C, #2
			; just skip bgcol bits
			SUB     F, #1
			JMNZO   F, @DRAW_INNER_KERN

		SUB   E, #1
		ADD   C, D  ; next row
		JMNZO E, @DRAW_OUTER_KERN

	_LOADALL
	RET

; A contains pointer to sprite memory
; B contains X pos
; C contains Y pos
; 	a magenta pixel is transparent
;   i.e. 0xF81F is skipped
@DRAW_SPRITE_16x16
	PSHS D
	PSHS E
	PSHS F
	PSHS G
	PSHS H

	ADRL D, #0x00
	ADRH D, #0x80  ; framebuffer
	MOVL E, #0x8A
	MOVM E, #0x02
	LSHL E, #1    ; bytes per row @ 16bpp
	MULA C, E
	LSHL B, #1
	ADD  C, D
	ADD  C, B     ; top left

	MOVL H, #0x1f
	MOVM H, #0xf8 ; magenta

	MOV  B, #16
	@DRAW_SPRITE_16x16_OUTER
		MOV  D, #16
		@DRAW_SPRITE_16x16_INNER
			LDRW   G, A+  ; has pixel
			ENDW   G, G   ; endian conversion
			CNE    G, H
			STRW.P G, C
			ADD    C, #2
			SUB    D, #1
			JMNZO  D, @DRAW_SPRITE_16x16_INNER
		SUB   C, #32
		ADD   C, E
		SUB   B, #1
		JMNZO B, @DRAW_SPRITE_16x16_OUTER

	POPS H
	POPS G
	POPS F
	POPS E
	POPS D
	RET

; print_next_char takes only a codepoint
; and handles the positioning of the char
; on the framebuffer by manipulating the
; "cursor" variables
@CURSOR_X
.INT8 0
@CURSOR_Y
.INT8 0
; screen is 80 chars wide (with 5 px on either side)
; and 25 tall 
; (640 / 8), (400 / 16)
;
; takes codepoint in A
@PUTCHAR_CURSOR
	PSHS B
	PSHS C
	PSHS D
	PSHS E

	ADRL  D, @CURSOR_X
	ADRM  D, @CURSOR_X
	ADRL  E, @CURSOR_Y
	ADRM  E, @CURSOR_Y

	LDRB  B, D
	LDRB  C, E

	LSHL B, #3  ; *8
	LSHL C, #4  ; *16

	ADD  B, #5  ; padding

	MOV.P D, #1

 FAR JLO @DRAW_CHAR_KERN

 	CEQ   D, #1

	LDRB  B, D
	LDRB  C, E

 	ADD   B, #1
 	MOV   A, #80
 	CEQ   A, B    ; B ?= 80
 	MOV.P B, #0
 	ADD.P C, #1

 	STRB  B, D
	STRB  C, E

	POPS E
	POPS D
	POPS C
	POPS B
	RET

; takes pointer to zero terminated string in A
; prints until empty
@PRINT_STRING
	PSHS B
	MOV  B, A

	@PRINT_STRING_L1
		LDRB  A, B+
		JMZO  A, @PRINT_STRING_L1_E
	FAR JLO   @PUTCHAR_CURSOR
		JMO   @PRINT_STRING_L1
	@PRINT_STRING_L1_E

	POPS B
	ERR
	RET

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
	MOV  C, #0x00
	MOVM C, #0x55

	@PAINT
		STRW  C, B+
		SUB   A, #1
		JMNZO A, @PAINT

	ADRL A, @STRING
	ADRM A, @STRING
FAR JLO  @PRINT_STRING

	ERR

	@LOOP
		JMO @LOOP

	ERR

@STRING
.ASCIZ This is 100% software-rendered from a string 

%PROG

.ORG 0x1000
@STK

.ORG 0x2000
.SEC %FONTS

$include font8.enn

$include font16.enn

%FONTS
