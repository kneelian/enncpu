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
@DRAW_CHAR
	FAR JMO @DRAW_CHAR_M
@PRINT_STRING
	FAR JMO @PRINT_STRING_M
@PUTCHAR_CURSOR
	FAR JMO @PUTCHAR_CURSOR_M
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
@V3_SUB_V3_REVERSE
	FAR JMO @V3_SUB_V3_REVERSE_M
@V3_NORM
	FAR JMO @V3_NORM_M
@V3_NEAR_ZERO
	FAR JMO @V3_NEAR_ZERO_M
@SETUP_RNG
	FAR JMO @SETUP_RNG_M

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
	@EX_KBD_EVENT
	ADRL  B, @KBD_MAILBOX
	ADRM  B, @KBD_MAILBOX
	ADRH  B, @KBD_MAILBOX

	LDRW  A, B

	MOV   B, KEY_Q

	CEQ   A, B
	JMO.P @KILL

	ADRL  B, @CHAR_MAILBOX
	ADRM  B, @CHAR_MAILBOX
	STRW  A, B

	MOV   A, #0 ; clearing exception state
	WXS   A
	RET

@KILL
	JLA @GET_TIMER
	JMA @DIE

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

	ADRL A, @STRING_1
	ADRM A, @STRING_1
	JLA  @PRINT_STRING_M

	@LOOP
		WFI
		ADRL  A, @CHAR_MAILBOX
		ADRM  A, @CHAR_MAILBOX
		LDRW  B, A
		JMZO  B, @LOOP

	_888_TO_565 A, 0x30, 0x00, 0x20, B
FAR JLO @CLRSCR

	JLA @SETUP_RNG

	ERR

	; we harvest entropy from the time it takes
	; the user to hit the keyboard to continue
	;
	; this lets us get some nondeterministic output
	; for reruns of this thing

	; ----------- raytracer!!

	; loop goes:
	; start from pixel (0, 0) and move right and down
	; subtract from camera centre
	; check hit for all sphere objects in order?

	MOV G, #0 ; x
	MOV H, #0 ; y

	@RAY_LOOP
		PSHW  H
		PSHW  G

		; 


		POPW  G
		ADD   G, #1
		MOVL  H, #650
		MOVM  H, #650
		CEQ   G, H
		MOV.P G, #0
		POPW  H
		ADD.P H, #1
	FAR JMO @RAY_LOOP


FAR JMO @RAY_LOOP

	ERR


%PROG       ; <╟┐
            ;   │ grows down 
.ORG 0x1000 ;   │ from 0x1000
@STK  ; ────────┘      

.ORG 0x2000
.SEC %DATA

@CHAR_MAILBOX
.INT16 0x0000

@STRING_1
.ASCII RAYTRACER EXAMPLE
.INT8  0x0a
.ASCII PRESS ANY KEY TO START,, P
.ASCIZ RESS Q TO EXIT

; eyepoint is at (0, 10, 0)
@CAM_X
.FP24 0.0
@CAM_Y
.FP24 10.0
@CAM_Z
.FP24 0.0
; centre of plane of vision is at (0, 10, 5)
@FOCAL
.FP24 5.0

; these are adjusted from centre of POV
; rather than absolute values
; so top left corner of the pixel
; (0,0) will actually be (-3.25, 12.0, 5.0)
; but the centre of the pixel has to be
; half a delta in from the corner
@PIX_0x0_X
.INT24 0xc04fae ; -3.245
@PIX_0x0_Y
.INT24 0x400052 ; +2.005

; steps for pixels
; picked for convenience
@DELTA_WIDTH
.INT24 0x3c23d7 ; 0.01
@DELTA_HEIGHT
.FP24  0x3c23d7 ; 0.01

; picked to match the aspect ratio
; of the framebuffer (650x400)
@WIDTH_PP
.FP24 6.5
@HEIGHT_PP
.FP24 4.0

@WIDTH_PIX
.INT16 650
@HEIGHT_PIX
.INT16 400

; demo sphere
@SPHERE1_X
.FP24 0.0
@SPHERE1_Y
.FP24 10.0
@SPHERE1_Z
.FP24 8.0  ; POV + 3.0
@SPHERE1_R
.FP24 1.25 ; so it fits on the screen and doesnt clip

%DATA

.ORG 0x4000
.SEC %FONTS
$include font16.enn
%FONTS


.ORG 0x800000
@FRAMEBUFFER
.ORG 0x87f400
@KBD_MAILBOX
