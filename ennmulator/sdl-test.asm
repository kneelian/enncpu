.SEC %main
@start
	CXS
;	ERR
FAR JMO.P @handler
FAR JMO   @coldboot

@handler
	RXS   H
	GETL  H, H
	
	CEQ   H, #37  ; refresh interrupt
FAR JMO.P @screen_refresh
; we don't handle
; other interrupts
	MOV   F, #0
	WXS   F
	RET

@coldboot

MOVL A, #0x00
MOVM A, #0x40
WSP  A

UNMASK

SUB  A, #37
SEED A

MOVL C, #0x08
MOVM C, #0x01
MOVL D, #0x74
MOVM D, #0x01
MULA C, D
SHDW C

MOVL B, #0x40
MOVH B, #0x01

SUB  A, A
STRW A, B

MOV  D, #16
LSHL D, #3

@repeat
	MASK
	MOVL B, #0x80
	MOVH B, #0x01

	LITE C
	@loop
		
		RND  A
		STRW A, B+

		SUB   C, #1
		JMNZO C, @loop

	UNMASK

	MOVL B, #0x40
	MOVH B, #0x01

	@stall
	LDRW  E, B
	CEQ   E, #0
	JMO.P @stall
	; DBGS  E

	MOV   E, #0
	STRW  E, B
	SEED  A
	SUB   D, #1
	JMNZO D, @repeat

ERR

; address in F
; trashes G
@printstr
	LDRB  G, F+
	CEQ   G, #0
	MOV.P G, #10
	DBGC  G
	RET.P
	JMO   @printstr

@screen_refresh
	ADRL  F, @string
	ADRM  F, @string
FAR	JLO   @printstr	
	UNMASK
	MOV   F, #0
	WXS   F
	RET

@string
.ASCIZ Screen refreshed!

%main
