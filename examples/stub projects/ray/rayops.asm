; -----------------------
; all routines here take
; a v3 input in ABC / xyz
; and return either a v3
; in ABC / xyz order, or
; a scalar in A
;
; those that take a scalar
; take it in D
; those that take another v3
; take it in DEF / xyz order
;
; those that take both v3 and
; a scalar take them in
; DEF(v3) G(f)
;
; routines that need more than
; two v3s will
;	A   - address of v3
;	B   - address of v3
;	c   - address of v3
;   D   - address of v3
;   E   - address of v3
;   F   - address of v3
;	G   - scalar
;
; routine list:
;	- ray_at
;
; routines do not aim to
; preserve any of the regs
; so back them up if needed
; -------------------------

.ORG 0x1d00
.SEC %rayops

;----------------
;    takes
; ABC - origin
; DEF - direction
; G   - distance
;
;    returns
; ABC - point
;----------------

@ray_at
	FMUL D, G
	FMUL E, G
	FMUL F, G
	FADD A, D
	FADD B, E
	FADD C, F
	RET

; A   - sphere centre address
; D   - ray origin address
; E   - ray direction address
; G   - sphere radius
@hit_sphere
	ADRL H, @hit_sphere_tempa
	ADRM H, @hit_sphere_tempa
	LD3S ABC, A
	ST3S ABC, H

	ADRL H, @hit_sphere_tempb
	ADRM H, @hit_sphere_tempb
	LD3S ABC, D
	ST3S ABC, H

	ADRL H, @hit_sphere_tempc
	ADRM H, @hit_sphere_tempc
	LD3S ABC, E
	ST3S ABC, H

	ADRL H, @hit_sphere_tempr
	ADRM H, @hit_sphere_tempr
	FMUL G, G
	STRS G, H

	; compute oc = ray.origin() - center 
	ADRL H, @hit_sphere_tempb
	ADRM H, @hit_sphere_tempb
	LD3S ABC, H-
	LD3S DEF, H
	FSUB A, D
	FSUB B, E
	FSUB C, F
	ADRL H, @hit_sphere_tempd
	ADRM H, @hit_sphere_tempd
	ST3S ABC, H

	; compute a = rdir . rdir
	ADRL H, @hit_sphere_tempc
	ADRM H, @hit_sphere_tempc
	LD3S ABC, H
	FMUL A, A
	FMUL B, B
	FMUL C, C
	FADD A, B
	FADD A, C
	ADRL H, @hit_sphere_tempe
	ADRM H, @hit_sphere_tempe
	STRS A, H

	; compute b = -2 * rdir . oc
	ADRL H, @hit_sphere_tempc ; rdir
	ADRM H, @hit_sphere_tempc
	LD3S ABC, H
	ADRL H, @hit_sphere_tempd ; oc
	ADRM H, @hit_sphere_tempd
	LD3S DEF, H
	FMUL A, D
	FMUL B, E
	FMUL C, F
	FADD A, B
	FADD A, C
	FC2  C
	FNEG C, C
	FMUL A, C
	ADRL H, @hit_sphere_tempf
	ADRM H, @hit_sphere_tempf
	STRS A, H
	
	; compute c = oc . oc - r^2
	ADRL H, @hit_sphere_tempd
	ADRM H, @hit_sphere_tempd
	LD3S ABC, H
	FMUL A, A
	FMUL B, B
	FMUL C, C
	FADD A, B
	FADD A, C
	ADRL H, @hit_sphere_tempr
	ADRM H, @hit_sphere_tempr
	LDRS G, H
	FSUB A, G
	ADRL H, @hit_sphere_tempg
	ADRM H, @hit_sphere_tempg
	STRS A, H
	MOV  C, A

	; compute 4ac
	FC2  B
	FMUL A, B
	FMUL A, B ; 4c
	ADRL H, @hit_sphere_tempe
	ADRM H, @hit_sphere_tempe
	LDRS B, H
	FMUL A, B

	; compute b^2
	ADRL H, @hit_sphere_tempf
	ADRM H, @hit_sphere_tempf
	LDRS D, H
	FMUL D, D

	; compute D = b^2 - 4ac
	FSUB D, A

	; if D < 0 return 0
	; FC1.P  A
	; FNEG.P A, A
	FSUB E, E
	FCLT D, E	
	MOV.P A, #0
	RET.P

	; else return (-b - sqrt(D)) / (2a)
	ADRL H, @hit_sphere_tempe
	ADRM H, @hit_sphere_tempe

	MOV  A, #1

	LDRS A, H+
	LDRS B, H
	FSQT D, D
	FNEG B, B
	FSUB B, D
	FC2  H
	FMUL A, H
	FDIV B, A
	MOV  A, B

;	BOR  A, A
	
	RET

@hit_sphere_tempa   ; sphere_centre
	.FP24 0.0
	.FP24 0.0
	.FP24 0.0
@hit_sphere_tempb   ; ray_origin
	.FP24 0.0
	.FP24 0.0
	.FP24 0.0
@hit_sphere_tempc   ; ray_direction
	.FP24 0.0
	.FP24 0.0
	.FP24 0.0
@hit_sphere_tempd   ; oc : centre - rayorigin
	.FP24 0.0
	.FP24 0.0
	.FP24 0.0
@hit_sphere_tempr ; sphere r^2
	.FP24 0.0
@hit_sphere_tempe ; a
	.FP24 0.0
@hit_sphere_tempf ; b
	.FP24 0.0
@hit_sphere_tempg ; c
	.FP24 0.0

%rayops
