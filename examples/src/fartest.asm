.ORG 0x0000
.SEC %main
@start
	MOVM A, #0x01
	WSP  A

	JMO  @death

FAR JMO  @veryfar
	JMO  @death
	ERR

@death

@backj
	ADRL A, @str2
	ADRM A, @str2
	@loop2
		LDRB  B, A+
		DBGC  B
		JMNZO B, @loop2
	MOV  B, '\n'
	DBGC B

	RET
	
%main

.ORG 0x1234
.SEC %far

@veryfar
	ADRL A, @str
	ADRM A, @str

	@loop
		LDRB  B, A+
		DBGC  B
		JMNZO B, @loop
	MOV  B, '\n'
	DBGC B

FAR JLO @backj
	ERR

@str
.ASCIZ did it work?
@str2
.ASCIZ what abt this one?
%far
