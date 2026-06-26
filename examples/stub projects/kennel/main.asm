
; --------------------------
; 
;	the kennel (enncpu kernel)
;
;   the ennmulator starts execution
;   from 0x0000, which is technically
;   by default covered with the section
;   directive which expands to JMO 4
;   and two sesquis, start and length
;   of a section when loading into
;   memory.
;
;	since the enncpu has special addressing
;   that jumps to 0x000--0x200 in two bytes,
;   as opposed to loading the address
;   and jumping to it in 2-4 instructions,
;   some of the most frequently used routines
;   go in this space to be easily called.
;
; -------------------------------------

.ORG 0x0000
.SEC %jmapage
@entry
	ADRL	A, @setup	; 0x08
	ADRM	A, @setup	; 0x0a
	JMR		A        	; 0x0c

; 	returns hash of A in A
;	trashes B
;
@hash_a
	; 0xcb3957
	MOVL	B, #0x57   	; 0x0e
	MOVM	B, #0x39	; 0x10
	MOVH	B, #0xcb	; 0x12
	MULA	A, B		; 0x14
	MOV 	B, A		; 0x16
	LSHR	B, #17		; 0x18
	BXOR	A, B		; 0x0a
	; 0x62ba47
	MOVL	B, #0x47	; 0x0c
	MOVM	B, #0xba 	; 0x0e
	MOVH	B, #0x62 	; 0x20
	MULA	A, B 		; 0x22
	MOV 	B, A 		; 0x24
	LSHR	B, #15 		; 0x26
	ADD 	A, B 		; 0x28
RET						; 0x2a

; 	prints to debug 
;	string pointed to in A
;   trashes B
;
@cstr
	LDRB  B, A+			; 0x2c
	DBGC  B 			; 0x2e
	JMNZO B, @cstr		; 0x30
RET 					; 0x32

; 	fetches from debug input
;   output location pointed to in A
;   buffer length in B
;   trashes C
;
@indbg
	@indbg_l
	DBGINC C 			; 0x34

	; buffer empty?
	CEQ    B, #1 		; 0x36
	RET.P 				; 0x38

	; newline?
	CEQ    C, #0x0a 	; 0x3a
	MOV.P  C, #0 		; 0x3c
	STRB   C, A+ 		; 0x3e
	RET.P 				; 0x40

	SUB    B, #1 		; 0x42
	JMO @indbg_l 		; 0x44


; 	puts pixel (A,B) col (C)
;	trashes D, E
;
@putpx
	; root of framebuffer
	MOVL  D, #0x80 		; 0x46
	MOVH  D, #0x01 		; 0x48

	; width in px
	MOVL  E, #0x74 		; 0x4a
	MOVM  E, #0x01 		; 0x4c

	; 2 bytes per pixel
	LSHL  E, #1 		; 0x4e

	; calculate pos in fb
	MULA  B, E 			; 0x50
	ADD   B, A 			; 0x52
	ADD   D, B 			; 0x54

	; write
	STRW  C, D 			; 0x56
RET 					; 0x58

;	copies bytes from A to B
;	C number of times
;	trashes D
;
@memcpy
	LDRB  D, A+ 		; 0x5a
	STRB  D, B+ 		; 0x5c
	SUB   C, #1 		; 0x5e
	JMNZO C, @memcpy 	; 0x60
RET 					; 0x62

;	converts num in reg A
;	to string in buffer pointed in B
;	of length C
;	trashes D
;
@reg2str
	; get end of buffer
	ADD   C, B 			; 0x64

	@reg2str_l
	; fetch last nibble
	MOV   D, A 			; 0x66
	BAND  D, #0x0f 		; 0x68

	; convert to char
	CGT   D, #9 		; 0x6a
	ADD.P D, #7 		; 0x6c
	ADD   D, #48 		; 0x6e

	; store
	STRB  D, C- 		; 0x70

	; are we at start of buffer?
	CEQ   C, B 			; 0x72
	RET.P 				; 0x74
	JMNZO A, @reg2str_l ; 0x76
RET 					; 0x78

%jmapage

; 	kennel entry point
;
.ORG 0x1000
.SEC %start
@setup
	; in case we're entering from
	; an elevation from userland
	SWAP

	RSP A
	RPS B
	RXS C

	CNE    C, #0
	ADRL.P D, @took_exception
	ADRM.P D, @took_exception
	JMR.P  D

	; otherwise
	; this is a cold boot
	; we can trash ABC

	; kennel stack starts at 128 B
	; below the kennel code
	MOVL   A, #0x80
	MOVM   A, #0x0f
	WSP    A

	KERNI #3

	ERR

@took_exception
	ADRL A, @test
	ADRM A, @test
	PSHS A
	CNEV
	SUB  A, A
	WXS  A
	SWAP
	ERET

	ERR

%start

.ORG 0x2000
.SEC %test
@test
	JUNKS
	ERR

%test