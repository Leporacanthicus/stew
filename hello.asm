	;; Simple "Hello, World!" program in stew assembly.

	mov	hello, r1
loop:
	mov.b	(r1)+,r0
	beq	done
	emt	1
	jmp	loop

done:
	hlt

hello:
	.db	"Hello, World!",10,0
