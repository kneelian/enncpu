.ORG 0x0000
.SEC %pga
@ENTRY
	ADRL B, @START
	ADRM B, @START
	JMR  B

@NEWROWB
	SWPR B
	SWPR D
	ADD  B, D
	SWPR B
	SWPR D
	RET

@PUTPX
	SWPR B
	PSHS C
	MOV  C, A
	LSHL A, #11
	BOR  A, C
	STRW A, B+
	POPS C
	SWPR B
	RET
%pga

.ORG 0x0100
.SEC %main
@START

MOVL D, #2  ; per row pixel offset
MOVL B, #0
MOVH B, #0x01
ADD  B, #0x20
ADD  B, #0x20
ADD  B, #0x20
ADD  B, #0x20

SHDW B
SHDW D
SHDW H

MOVM A, #4
WSP  A

ADRL A, @VARS
ADRM A, @VARS

; stepx = (maxre - minre) / xsize (370)
; stepy = (maxim - minim) / ysize (260)

LDRS D, A+
LDRS H, A+
FSUB D, H
MOVL H, #0x72
MOVM H, #0x01 ; 370
FCNV H, H
FDIV D, H

LDRS E, A+
LDRS H, A
FSUB E, H
MOVL H, #0x04
MOVM H, #0x01
FCNV H, H
FDIV E, H

; D holds stepx
; E holds stepy

; for(y = 0; y < YSize; y++)
; stored in B
SUB  B, B
@LoopX
    FCNV F, B ; fp(y)
    FMUL F, E ; *stepy
    LDRS H, A 
    FADD F, H ; +MinIM = im

    ; for(x = 0; x < XSize; x++)
    ; stored in C
    SUB  C, C
    @LoopY
        FCNV G, C ; fp(x)
        FMUL G, D ; *stepx
        SUB  A, #6
        LDRS H, A
        ADD  A, #6
        FADD G, H ; +MinRE = re

        PSHS D
        PSHS E

        MOV  D, F ; Zi
        MOV  E, G ; Zr

        PSHW A
        PSHW B
        PSHW C

        SUB  A, A
        @LoopN

            MOV  B, D
            FMUL B, B ; Zi^2

            MOV  C, E
            FMUL C, C ; Zr^2

            MOV  H, B
            FADD H, C ; Zi^2 + Zr^2

            PSHB  A
            FC2   A
            FADD  A, A
            FCGT  H, A ; A+B > 4.0?
            POPB  A
            JMO.P @LoopNEnd ; escape condition

            FMUL D, E ; zi := zi*zr
            FADD D, D ; *2
            FADD D, F

            MOV  E, C
            FSUB E, B
            FADD E, G

        SUB   A, #56
        SUB   A, #56
        CNE   A, #0
        ADD.P A, #56
        ADD.P A, #57
        JMO.P @LoopN
        @LoopNEnd

        MOVL B, #96
        LSHL B, #1
        SUB  B, A
        JLA  @PUTPX
        
        POPW C
        POPW B
        POPW A
        POPS E
        POPS D

    MOVL  G, #0x72
    MOVM  G, #0x01
    CLT   C, G
    ADD.P C, #1
    JMO.P @LoopY
    JLA   @NEWROWB
    ; MOV   C, #10
    ; DBGC  C

MOVL  H, #248
ADD   H, #12
CLT   B, H
ADD.P B, #1
JMO.P @LoopX

ERR

@VARS
.FP24 1.0
.FP24 -2.0
.FP24 1.0
.FP24 -1.0

%main
