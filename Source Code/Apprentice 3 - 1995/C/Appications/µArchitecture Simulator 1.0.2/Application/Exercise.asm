;test program manually compiled from C

;i	EQU	-2
;k	EQU	-1

;main()
;{
;int i,k;
;int v[16];
main	LINK	#-19
;for (i=0; i<16; i++) v[i]=i;
	MOVQ	#16
	INSP	#16	;SP after array end
ciclo1	DEAC	#1
	PUSH
	JNZE	ciclo1
;for (i=0; i<16; i++) v[i]=i+v[i]*v[i];
;	MOVQ	#0	AC is already zeroed
	STOL	#-2	;i
	SWBP
	PUSH		;save locals BP
	DEAC	#19	;AC points at the array beginning
	SWBP		;temporarily put it in BP
	POP		;the locals BP
	SWBP		;stuff back into BP
	STOL	#-3	;stored
ciclo2	LODL	#-3	;take v
	ADDL	#-2	;find v[i]
;	DESP	#1	;reserve space for result
	PSHI
	PSHI
	CALL	mult
;	POP
	ADDL	#-2	;add i to the square
	PUSH		;save it
	LODL	#-3	;take v
	ADDL	#-2	;find v[i]
	POPI		;put into v[i]
	MOVQ	#1
	ADDL	#-2	;i
	STOL	#-2	;increment i
	DEAC	#16
	JNZE	ciclo2
;k = sum(v,16);
;	MOVQ	#0
	STOL	#-2	;i
	LODL	#-3	;v
;	DESP	#1	;reserve space for result
	PUSH		;v
	MOVQ	#16
	PUSH		;size
	CALL	sum
;	POP
	STOL	#-1	;k
	MOVQ	#69
	CALL	output
	MOVQ	#110
	CALL	output
	MOVQ	#100
	CALL	output
	MOVQ	#13
	CALL	output
;example input: waiting for a carriage return
;waitcr	CALL	input
;	DEAC	#13
;	JNZE	waitcr
	MOVQ	#79
	CALL	output
	MOVQ	#75
	CALL	output
	MOVQ	#13
	CALL	output
	RETN	#0
;}

;int sum(int *vect, int size)
;{
;int i,k=0;
sum	LINK	#-2
;for(i=0; i<size; i++) k=k+vect[i];
	MOVQ	#0
	STOL	#-2	;i
	STOL	#-1	;k, forgotten?
loops	LODL	#3	;&v
	ADDL	#-2	;address of v[i]
	PSHI
	POP		;value of v[i]
	ADDL	#-1	;k
	STOL	#-1	;k
	MOVQ	#1
	ADDL	#-2	;i
	STOL	#-2	;i
	SUBL	#2	;size (ac-=size)
	JNZE	loops
	LODL	#-1	;k
;	STOL	#4	;return k
	RETN	#2
;}

;int mult(int a, int b)
;{
mult	LINK	#-1
	MOVQ	#0
	STOL	#-1	;zero out result
	LODL	#2
	JZER	zero	;if one is zero, exit immediately
retry	ANDI	#1	;check if we can shift
	JNZE	loopm	;jump if we cannot
	LODL	#3
	LSL1
	STOL	#3
	LODL	#2
	ASRA
	STOL	#2
	JUMP	retry
loopm	LODL	#3
	ADDL	#-1
	STOL	#-1
	LODL	#2
	DEAC	#1
	STOL	#2
	JNZE	loopm
	LODL	#-1
zero	RETN	#2
;}

input	LODD	#-2
	JZER	input	;wait for input
	PUSH
	MOVQ	#0
	STOD	#-2	;input acknowledged, zero loc
	POP
	RTS	#0

output	PUSH		;save character to output
loop	LODD	#-1
	JNZE	loop	;wait for acknowledge of previous char
	POP		;get back the char
	STOD	#-1	;put it onto memory mapped output loc
	RTS	#0
