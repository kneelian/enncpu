$include ray-macros.enn
$include raylib-constants.enn

.ORG 0x0000
.SEC %PGA
	MASK
	ADRL A, @STK
	ADRM A, @STK
	WSP  A

	ADRL A, @EXVEC
	ADRM A, @EXVEC
	WXV  A

	ADRL A, @MAIN
	ADRM A, @MAIN

	JMR  A

; PGA-reachable routines
; suffixing all of these with _M
; to denote "main part" of their code
; most of these are just stubs
@DIE
	ERR
@MEMCPY
	FAR JMO @MEMCPY_M
@INT_TO_DEC
	FAR JMO @INT_TO_DEC_M
@DEC_TO_INT
	FAR JMO @DEC_TO_INT_M
@INT_TO_HEX
	FAR JMO @INT_TO_HEX_M
@HEX_TO_INT
	FAR JMO @HEX_TO_INT_M
@DRAW_PX
	FAR JMO @DRAW_PX_M
@CLRSCR
	FAR JMO @CLRSCR_M
@GET_TIMER
	FAR JMO @GET_TIMER_M
@DRAW_HLINE
	FAR JMO @DRAW_HLINE_M
@DRAW_VLINE
	FAR JMO @DRAW_VLINE_M
@DRAW_RECT
	FAR JMO @DRAW_RECT_M
@DRAW_RECT_FILL
	FAR JMO @DRAW_RECT_FILL_M

@V3_DOT_V3
	FAR JMO @V3_DOT_V3_M
@V3_CRX_V3
	FAR JMO @V3_CRX_V3_M
@V3_LEN
	FAR JMO @V3_LEN_M
@V3_ADD_V3
	FAR JMO @V3_ADD_V3_M
@V3_SUB_V3
	FAR JMO @V3_SUB_V3_M
@V3_NORM
	FAR JMO @V3_NORM_M
@V3_NEAR_ZERO
	FAR JMO @V3_NEAR_ZERO_M

$include pga.enn

; exceptions load an addr from
; this table by offset; XS contains
; the offset to the correct handler
; and the interrupt routine uses
; this offset to find where to jump
@EX_TABLE
.INT16 @KILL            ; 0x00
.INT16 @EX_TIMER_EVENT  ; 0x02
.INT16 @EX_KBD_EVENT    ; 0x04
.INT16 @KILL            ; 0x06

@EX_TIMER
.INT24 0x00
.INT24 0x00
@EX_TIMER_EVENT
	ADRL  A, @EX_TIMER
	ADRM  A, @EX_TIMER
	LDRS  B, A
	ADD   B, #1
	STRS  B, A+
	CEQ   B, #0
	LDRS  B, A
	ADD.P B, #1
	STRS  B, A
	RET

@EX_KBD_EVENT
	RET

@KILL
	JLA @GET_TIMER
	ERR

@EXVEC
	MASK
	PSHS A
	PSHS B
	RPS  A
	PSHS A

	RXS  A
	ADRL B, @EX_TABLE
	ADRM B, @EX_TABLE
	ADD  B, A
	LDRW A, B
	JLR  A

	POPS A
	WPS  A
	POPS B
	POPS A
	UNMASK
	ERET

%PGA

.ORG 0x0800
.SEC %PROG

@MAIN
	UNMASK
	_888_TO_565 A, 0x30, 0x00, 0x20, B
FAR JLO @CLRSCR

	MOV  A, #0
	INV  A, A
	MOV  B, #250
	MOV  C, #125
	MOV  D, #250
	ADD  D, B
	MOV  E, #150
	JLA  @DRAW_RECT

	MOV  B, #250
	MOV  C, #170
	MOV  D, #250
	ADD  D, B
	MOV  E, #195
	JLA  @DRAW_RECT_FILL

	MOVL B, #0xB00BA5
	MOVM B, #0xB00BA5
	MOVH B, #0xB00BA5
	SEED B

	MOV  D, #0x04
	FCNV A, D
	FCNV B, D
	FCNV C, D
	FCNV D, D

	_V3_OP_FP24 A, B, C, FDIV, D

	FCST E, A

	ERR

	@LOOP
		WFI
		JLA  @GET_TIMER
		JMO  @LOOP
	ERR

%PROG
            ; <╟┐
            ;   │ grows down 
.ORG 0x1000 ;   │ from 0x1000
@STK  ; ────────┘      

.ORG 0x800000
@FRAMEBUFFER
.ORG 0x87f400
@KBD_MAILBOX
