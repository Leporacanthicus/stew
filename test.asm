label:
	mov	r0,r1
	mov	r2,r3
	mov	(r1),r4
	mov	(r2)+,r5
	mov	r4,-(sp)
	mov	(pc)+,r8
	mov	r10,r11
	mov	r12,r13
	mov	r14,r15

	add	r0,r1
	sub	r1,r0
	mul	r1,r0
	div	r1,r0

	xor	r0,r0
	or	r1,r2
	and	r3,r4

	lsr	r0,r1
	lsl	r1,r0
	asr	r2,r3
	asl	r4,r5

	ror	r1,r2
	rol	r2,r3
	adc	r9,r10
	sbc	r11,r12
	
