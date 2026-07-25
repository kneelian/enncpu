.ORG 0x0000
.SEC %PGA

	MOVL  A, #0x00
	MOVM  A, #0x04
	WSP   A

	MOVL  H, #0x00
	MOVH  H, #0x80	
	
	MOVL  A, #0x8a
	MOVM  A, #0x02
	MOVL  B, #0x90
	MOVM  B, #0x01

	MOV   C, A
	
	MOVL  D, #0x1f
	MOVM  D, #0xf8

	ENDW  D, D

	@LOOP_Y
		
		@LOOP_X
			STRW  D, H+

			SUB   A, #1
			JMNZO A, @LOOP_X

		MOV   A, C
		SUB   B, #1
		
		JMNZO B, @LOOP_Y

	ERR

%PGA
