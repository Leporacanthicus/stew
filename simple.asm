	;;  Simple example
	mov	#10, r0
loop:
	sub	#1, r0
	bne	loop
	hlt
