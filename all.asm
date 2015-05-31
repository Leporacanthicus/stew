	;; All assembler instruction variants
	mov	stack,sp
	mov	#42,r0
	mov	r0,r1
	mov	r1,r2
	mov	#21,r3
	mov	r3,r4
	mov	r4,r5
	mov	r5,r6
	mov	r6,r7
	mov	r7,r8
	mov	r8,r9
	mov	r9,r10
	mov	r10,r11
	mov	r11,r12
	mov	r12,r13
	mov	label,r0
	jmp	r0
	jmp	fail
	
label:
	br	lab2
	jmp	fail

lab2:
	mov	#6,r0
	cmp	r3,r2
	bgt	next2
	br	fail
	
next2:
	mov	#0,r0
	cmp	r2,r3
	blt	fail		
	nop
	mov	#1,r0
	cmp	r0,r0
	bne	fail
	mov	#2,r0
	cmp	r1,r1
	bne	fail
	mov	#3,r0
	cmp	r2,r3
	bgt	fail
	mov	#4,r0
	cmp	r3,r4
	bne	fail
	mov	#5,r0
	ble	next
	br	fail
next:
	mov	#7,r0
	cmp	r3,r4
	bge	next3
	br	fail
	
next3:
	mov	#-1,r1
	mov	#-2,r2
	mov	#8,r0
	cmp	r2,r1
	bhi	fail
	blos	next4
	br	fail
	
next4:
	mov	#9,r0
	mov	#-1,r2
	bmi	next5
	br	fail

next5:
	mov	#10,r0
	mov	#1,r2
	bpl	next6
	br	fail

next6:
	mov	#11,r0
	mov	#0x7f000000,r1
	add	#0x7f000000,r1
	bvs	next7
	br	fail

next7:
	mov	#12,r0
	mov	#0x7f000000,r1
	add	#1,r2
	bvc	next8
	br	fail

next8:
;;; flag setting/clearing
	mov	#13,r0
	sec
	bcc	fail
	bcs	next9
	br	fail

next9:
	mov	#14,r0
	clc
	bcs	fail
	bcc	next10
	br	fail


next10:
	mov	#15,r0
	sev
	bvc	fail
	bvs	next11
	br	fail

next11:
	mov	#16,r0
	clv
	bvs	fail
	bvc	next12
	br	fail

next12:
	mov	#17,r0
	clz
	beq	fail
	bne	next13
	br	fail

next13:
	mov	#18,r0
	sez
	bne	fail
	beq	next14
	br	fail

next14:
	mov	#19,r0
	sen
	bpl	fail
	bmi	next15
	br	fail

next15:
	mov	#20,r0
	cln
	bmi	fail
	bpl	next16
	br	fail

next16:
;;; Check automatic sign extension
	mov	#21,r0
	mov.b	#0xff,r2
	cmp	#0xffffffff,r2
	bne	fail

	mov	#22,r0
	mov.b	#0x80,r2
	cmp	#0xffffff80,r2
	bne	fail

finished:
	mov	success,r0
	jsr	print
	hlt


fail:
	mov 	r0,-(sp)
	mov	failmsg,r0
	jsr	print
	mov	(sp)+,r0
	jsr	printnum
	mov	#10,r0
	emt	1
	hlt
	
print:
	mov	r0,r1
loop:
	mov.b	(r1)+,r0
	beq	done
	emt	1
	jmp	loop

done:
	ret

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

failmsg:
	.db	"Failed at testpoint number ",0
success:
	.db	"All tests passed",10,0

	.align	4

	.zero	400
stack:
