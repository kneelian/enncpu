.ORG 0x0000
.SECTION %pga

	MOVL A, @start
	MOVM A, @start
	JMR  A

; address in A
; temp in B
@printstr
	LDRB  B, A+
	DBGC  B
	JMNZO B, @printstr
	RET

; char in A
; return in B
@is_alpha
	GETL  A, A    ; save only low byte
	CGT   A, #47  ; >= '0'
	MOV   B, #0
	MOV.P B, #1
	RET

; D holds expr string
; E holds operand stack
; F holds target string
; G holds opstack head
@shunt
	LDRB  A, D+
	CEQ   A, #0
	JMA.P A, @quit
	JLA   @is_alpha
	; 1 if alpha
	JMZO  B, @operand

	STRB  A, F+
	JMO   @shunt

@operand
	CEQ    A, #40  ; '('
 OR CEQ    A, #42  ; '*'
 OR CEQ    A, #47  ; '/'
	JMA.P  @pushout

	; else closing bracket

	CEQ    A, #41  ; ')'
	JMO.P  @closing

	; else + and -

@lower_ops
	CEQ    E, G
	JMA.P  @shunt  ; stack cleared

	LDRB   B, -G
	CEQ    B, #42  ; '*'
 OR CEQ    B, #43  ; '+'
 OR	CEQ    B, #45  ; '-'
 OR	CEQ    B, #47  ; '/'
	STRB.P B, F+
	JMO.P  @lower_ops

	; else we found a bracket

	JMA    @pushout

@pushout
	STRB   A, G+
	JMA    @shunt

@closing
	CEQ    E, G      ;
	JMA.P  @shunt    ; stack cleared
	LDRB   B, -G     ;
	CEQ    B, #40    ;
	JMA.P  @shunt    ; found opening bracket
	STRB   B, F+     ; else pop to output
	JMO    @closing  ;

@quit
	CEQ    E, G
	RET.P
	LDRB   B, -G
	STRB   B, F+
	MOV    B, #0
	STRB   B, F+
	JMO    @quit

%pga

.ORG 0x800
.SECTION %main
@start
	ADRL A, @_sec_data
	LSHL A, #1
	WSP  A

	ADRL A, @expr_string
	ADRM A, @expr_string
	JLA  @printstr
	MOV  B, #0x0a
	DBGC B
	DBGC B

	ADRL F, @tgt_string
	ADRM F, @tgt_string

	ADRL E, @op_stack
	ADRM E, @op_stack

	MOV  G, E

	ADRL D, @expr_string
	ADRM D, @expr_string

	MOV  H, F

	JLA  @shunt

	MOV  A, H

	JLA  @printstr

	MOV  B, #0x0a
	DBGC B
	DBGC B

	ERR

%main

.ORG 0x1000
.SECTION %data

@expr_string
	.INT8  0x0a
	.ASCIZ (4*(2+3))*(4+5)/6
	.INT8  0
	.PAD

@tgt_string
	.REP  32 0xff
	.INT8 0x00

@op_stack
	.REP  32 0xff
	.INT8 0x00

%data