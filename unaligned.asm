	;; Use unaligned access...
	mov	#42,r0
	mov	label,r1
	add	#1, r1
	mov	r0,(r1)
	hlt
	
label:
	.long	1
	
