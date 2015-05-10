	;; Factorial function

	mov	#0, r7
	mov	stack, sp
	add	#4096, sp
loop:	
	mov	r7, r0
	jsr	fact
	jsr	printnum
	;; Newline...
	mov	#10,r0
	emt	1
	add	#1, r7
	cmp	#14, r7
	bne	loop
	hlt

printnum:
	mov	#0,-(sp)
ploop:
	;; R0 = number divided, R1 = modulo.
	div	#10,r0
	add	#48,r1
	mov	r1,-(sp)
	cmp	#0,r0
	bne	ploop
ploop2:
	mov	(sp)+,r0
	beq	pdone
	emt	1
	jmp	ploop2
pdone:
	ret

fact:
	cmp	#0,r0
	beq	fact_end
	mov	r0,-(sp)
	sub	#1,r0
	jsr	fact
	mov	(sp)+,r1
	mul	r1,r0
	ret
	
fact_end:
	mov	#1,r0
	ret


stack:
