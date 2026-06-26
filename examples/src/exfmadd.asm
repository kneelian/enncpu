.SEC %main

@start
	ADRL H, @consts

	PRE	LD2S AB, H
		LD2S CD, +H
		LD2S EF, +H
		LD2S GH, +H

	    FMADD_48 AB, CD, EF

;	ADD E, #14

     MOV   A, #0
;    MOV.P A, #1

    ERR

@consts
.FP48  3.141499999
.FP48  1.4142
.FP48  5.18

.INT24 0x3e0000
.INT24 0x000000 ; epsilon for fsub

.INT24 0x4024ef
.INT24 0x21f6c9

%main
