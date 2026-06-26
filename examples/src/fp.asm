@START
MOVM 	A, #0xf
WSP  	A

FCPI  A
FCPHI B
FMUL  A, B
FADD  A, B
FCE   B
FMUL  A, B
FSIN  B, B
FDIV  A, B
FCPI  B
FMUL  A, B

DBGF  A

CINF   A
JMO.P  @INF
CNAN   A
JMO.P  @NAN

FC0    	E
FCLT   	A, E
MOV.P  	F, #45
CINV	
MOV.P  	F, #0
DBGC   	F
FABS	A, A

CNEV

MOV 	H, A    ; backup

MOV 	D, #10
FCNV	D, D

MOV     G, #1

@M_LOOPA
	MOV  	B, A
	FDIV.P  B, D
	MOV.P   A, B
	FMOD 	B, D
	FCST 	B, B
	FCST    C, A
	CNE  	C, #0
	PSHB.P 	B
	ADD.P   G, #1
	JMO.P   @M_LOOPA

@M_LOOPB
	SUB     G, #1
	CEQ     G, #0
	JMO.P   @M_LOOPB_E
	POPB	A

	ADD     A, #48
	DBGC    A

	JMO   @M_LOOPB
@M_LOOPB_E

; -------------------
; so far this handles
; the integer part of
; the fp24 number
; -------------------

MOV  A, #46
DBGC A

MOV  A, H
FCST B, H
FCNV B, B
FSUB A, B ; trim integer

FC0  C

MOV  G, #6

@M_LOOPC
	SUB   G, #1
	FMUL  A, D
	FCST  B, A
	MOV   E, B
	ADD   E, #48
	DBGC  E
	FCNV  B, B
	FSUB  A, B
	JMNZO G, @M_LOOPC

MOV  A, #10
DBGC A

ERR

@INF
ADRL A, @INF_STR
ADRM A, @INF_STR
@INFLOOP
	LDRB  B, A+
	DBGC  B
	JMNZO B, @INFLOOP

MOV  B, #10
DBGC B
ERR
@INF_STR
.ASCII infty.
.PAD

@NAN
ADRL A, @NAN_STR
ADRM A, @NAN_STR
@NANLOOP
	LDRB  B, A+
	DBGC  B
	JMNZO B, @NANLOOP

MOV  B, #10
DBGC B
ERR
@NAN_STR
.ASCII NaN
.PAD


; --------------
; takes one arg
; in gpr A
; trashes shadow
; --------------
@PRINTDEC
SWAP
LITE A

SUB  E, E

ADRL B, @CHARS
ADRM B, @CHARS

MOV  C, #10

@LOOPA
	MOV   D, A
	MOD   D, C
	PSHB  D
	ADD   E, #1
	DIV   A, C
	JMNZO A, @LOOPA

@LOOPB
	POPB  A
	MOV   C, B
	ADD   C, A
	LDRB  C, C
	DBGC  C
	SUB   E, #1
	JMNZO E, @LOOPB

SWAP
RET

@CHARS
.ASCII 0123456789x