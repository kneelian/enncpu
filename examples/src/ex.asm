
MOV A, #32
MOV B, #24
MOV H, #6
@LOOP
	CLT A, B
	ADD.P C, A
	CGT A, B
	ADD.P D, A
	SUB H, #1
	CNE H, #0 ;poopee
JMA.P @LOOP
ERR
MVH A, #37
MVL H, #2
;peepee poopoo
.INT16 0x55
JMO @LOOP
JMNZO A, #6
JMZO A, @LOOP
.INT8 0xff
.ASCII no need to enclose in quotes
.INT8 0xff
.PAD
.INT16 0xdead
JMO @LOOP
