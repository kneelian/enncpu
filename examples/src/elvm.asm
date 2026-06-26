
.SEC %data
        ADRL  G, @_sec_data
        ADRM  G, @_sec_data
        ADRH  G, @_sec_data
        JMR   G
.INT24 112
.INT24 101
.INT24 101
.INT24 115
.INT24 32
.INT24 99
.INT24 117
.INT24 116
.INT24 101
.INT24 108
.INT24 121
.INT24 0
.INT24 104
.INT24 105
.INT24 0
.INT24 16
.PAD
%data
.SEC %main
        ADRL H, @_sec_main
        ADRM H, @_sec_main
        ADRH H, @_sec_main
        MOVL G, #0x80
        LSHL G, #2
        ADD  H, G
        WSP  H
        SUB  G, G

@L0
        ADRL  G, @L1
        ADRM  G, @L1
        ADRH  G, @L1
        JMR   G
@L1
        MOVL  B, #0x0c
        MOV   G, #3
        MULA  B, G
        ADD   B, #16
        LDRS  A, B
        SUB   B, #16
        DIV   B, G
        DBGC  A
        ADD   B, #1
        MOV   G, #3
        MULA  B, G
        ADD   B, #16
        LDRS  A, B
        SUB   B, #16
        DIV   B, G
        DBGC  A
        ERR ;
@L2
        ERR ;
%main
