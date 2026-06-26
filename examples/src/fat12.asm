.ORG 0x0000
.SEC %main
JMA @start

@i2s
	PSHS A
	PSHS B
	PSHS C

	MOV  B, #10
	PSHB B

	@i2s_La
		MOV   C, A
		MOD   C, B
		PSHB  C
		DIV   A, B
		JMNZO A, @i2s_La

	MOV B, '0'

	@i2s_Lb
		POPB  A
		CEQ   A, #10
		JMO.P @i2s_quit
		ADD   A, B
		DBGC  A
		JMO   @i2s_Lb
	@i2s_quit

	POPS C
	POPS B
	POPS A

	RET

@start
	MOVM A, #0x10
	WSP  A

	MOVL A, #0xff
	MOVM A, #0xff

	PSHW A

	MOV  F, #2

FAR JMO @trawl_chain

@trawled
	ADRL  D, @fatchain
	ADRM  D, @fatchain

	MOVL  F, #0xff
	MOVM  F, #0xff

	@tr_loop
	POPW   B
	CNE    B, F
	STRW.P B, D+
	JMO.P  @tr_loop

	ADRL  E, @fatchain
	ADRM  E, @fatchain

	@tr_loop2
		LDRW  A, -D
		JLA   @i2s
		MOV   A, '\n'
		DBGC  A
		CNE   D, E
		JMO.P @tr_loop2	
	@trawl_end
	ERR

; F takes which sector to start
; with and the function returns
; reverse-order sectors on the stack
@trawl_chain
	ADRL H, @fat_data
	ADRM H, @fat_data
	
	MOV  E, F
	BAND E, #1
	BXOR F, E

	PSHB E
	
	MOV  G, F
	LSHR G, #1
	ADD  F, G
	
	ADD  H, F

FAR JLO   @read_sectors

	POPB  E
	CEQ   E, #1
	MOV.P A, C

	PSHW  A
	MOVL  B, #0xff
	MOVM  B, #0x0f

	CEQ   A, B
FAR JMO.P @trawled

	MOV   F, A
	
	JMO   @trawl_chain

	ERR

@read_sectors

	LDRS A, H+

	GETL C, A
	GETM B, A
	GETH A, A

	MOV  D, B
	MOV  E, B
	BAND D, #0x0f   ; get low 4 bits
	LSHR E, #4      ; get high 4 bits

	LSHL D, #8
	LSHL C, #4

	BOR  A, D
	BOR  C, E

	; A holds low  sector
	; C holds high sector

	RET

@fat_data

ERR

%main

.ORG 0x800
.SEC %scratch
@fatchain

.REP 0x40 0xff
%scratch
