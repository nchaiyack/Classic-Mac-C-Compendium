#NO_APP
gcc2_compiled.:
___gnu_compiled_c:
.text
	.even
_isalnum_:
	link a6,#0
	moveq #0,d0
	moveb a6@(11),d0
	movel __ctype,a0
	moveb a0@(d0:l),d0
	andb #14,d0
	moveq #0,d1
	moveb d0,d1
	movel d1,d0
	jra L1
L1:
	unlk a6
	rts
	.even
_isalpha_:
	link a6,#0
	moveq #0,d0
	moveb a6@(11),d0
	movel __ctype,a0
	moveb a0@(d0:l),d0
	andb #12,d0
	moveq #0,d1
	moveb d0,d1
	movel d1,d0
	jra L2
L2:
	unlk a6
	rts
	.even
_iscntrl_:
	link a6,#0
	moveq #0,d0
	moveb a6@(11),d0
	movel __ctype,a0
	moveb a0@(d0:l),d0
	andb #1,d0
	moveq #0,d1
	moveb d0,d1
	movel d1,d0
	jra L3
L3:
	unlk a6
	rts
	.even
_isdigit_:
	link a6,#0
	moveq #0,d0
	moveb a6@(11),d0
	movel __ctype,a0
	moveb a0@(d0:l),d0
	andb #2,d0
	moveq #0,d1
	moveb d0,d1
	movel d1,d0
	jra L4
L4:
	unlk a6
	rts
	.even
_isgraph_:
	link a6,#0
	moveq #0,d0
	moveq #0,d1
	moveb a6@(11),d1
	movel __ctype,a0
	moveb a0@(d1:l),d1
	andb #17,d1
	tstb d1
	jne L6
	moveq #0,d1
	moveb a6@(11),d1
	movel __ctype,a0
	tstb a0@(d1:l)
	jeq L6
	moveq #1,d0
L6:
	jra L5
L5:
	unlk a6
	rts
	.even
_islower_:
	link a6,#0
	moveq #0,d0
	moveb a6@(11),d0
	movel __ctype,a0
	moveb a0@(d0:l),d0
	andb #8,d0
	moveq #0,d1
	moveb d0,d1
	movel d1,d0
	jra L7
L7:
	unlk a6
	rts
	.even
_isprint_:
	link a6,#0
	moveq #0,d0
	moveq #0,d1
	moveb a6@(11),d1
	movel __ctype,a0
	moveb a0@(d1:l),d1
	andb #1,d1
	tstb d1
	jne L9
	moveq #0,d1
	moveb a6@(11),d1
	movel __ctype,a0
	tstb a0@(d1:l)
	jeq L9
	moveq #1,d0
L9:
	jra L8
L8:
	unlk a6
	rts
	.even
_ispunct_:
	link a6,#0
	moveq #0,d0
	moveb a6@(11),d0
	movel __ctype,a0
	moveb a0@(d0:l),d0
	andb #32,d0
	moveq #0,d1
	moveb d0,d1
	movel d1,d0
	jra L10
L10:
	unlk a6
	rts
	.even
_isspace_:
	link a6,#0
	moveq #0,d0
	moveb a6@(11),d0
	movel __ctype,a0
	moveb a0@(d0:l),d0
	andb #16,d0
	moveq #0,d1
	moveb d0,d1
	movel d1,d0
	jra L11
L11:
	unlk a6
	rts
	.even
_isupper_:
	link a6,#0
	moveq #0,d0
	moveb a6@(11),d0
	movel __ctype,a0
	moveb a0@(d0:l),d0
	andb #4,d0
	moveq #0,d1
	moveb d0,d1
	movel d1,d0
	jra L12
L12:
	unlk a6
	rts
	.even
_isxdigit_:
	link a6,#0
	moveq #0,d0
	moveb a6@(11),d0
	movel __ctype,a0
	moveb a0@(d0:l),d0
	andb #64,d0
	moveq #0,d1
	moveb d0,d1
	movel d1,d0
	jra L13
L13:
	unlk a6
	rts
	.even
_tolower_:
	link a6,#0
	movel a6@(8),sp@-
	jbsr _tolower
	addqw #4,sp
	movel d0,d1
	movel d1,d0
	jra L14
L14:
	unlk a6
	rts
	.even
_toupper_:
	link a6,#0
	movel a6@(8),sp@-
	jbsr _toupper
	addqw #4,sp
	movel d0,d1
	movel d1,d0
	jra L15
L15:
	unlk a6
	rts
	.even
.globl _isalnum
_isalnum:
	link a6,#0
	movel a6@(8),sp@-
	jbsr _isalnum_
	addqw #4,sp
	movel d0,d1
	movel d1,d0
	jra L16
L16:
	unlk a6
	rts
	.even
.globl _isalpha
_isalpha:
	link a6,#0
	movel a6@(8),sp@-
	jbsr _isalpha_
	addqw #4,sp
	movel d0,d1
	movel d1,d0
	jra L17
L17:
	unlk a6
	rts
	.even
.globl _iscntrl
_iscntrl:
	link a6,#0
	movel a6@(8),sp@-
	jbsr _iscntrl_
	addqw #4,sp
	movel d0,d1
	movel d1,d0
	jra L18
L18:
	unlk a6
	rts
	.even
.globl _isdigit
_isdigit:
	link a6,#0
	movel a6@(8),sp@-
	jbsr _isdigit_
	addqw #4,sp
	movel d0,d1
	movel d1,d0
	jra L19
L19:
	unlk a6
	rts
	.even
.globl _isgraph
_isgraph:
	link a6,#0
	movel a6@(8),sp@-
	jbsr _isgraph_
	addqw #4,sp
	movel d0,d1
	movel d1,d0
	jra L20
L20:
	unlk a6
	rts
	.even
.globl _islower
_islower:
	link a6,#0
	movel a6@(8),sp@-
	jbsr _islower_
	addqw #4,sp
	movel d0,d1
	movel d1,d0
	jra L21
L21:
	unlk a6
	rts
	.even
.globl _isprint
_isprint:
	link a6,#0
	movel a6@(8),sp@-
	jbsr _isprint_
	addqw #4,sp
	movel d0,d1
	movel d1,d0
	jra L22
L22:
	unlk a6
	rts
	.even
.globl _ispunct
_ispunct:
	link a6,#0
	movel a6@(8),sp@-
	jbsr _ispunct_
	addqw #4,sp
	movel d0,d1
	movel d1,d0
	jra L23
L23:
	unlk a6
	rts
	.even
.globl _isspace
_isspace:
	link a6,#0
	movel a6@(8),sp@-
	jbsr _isspace_
	addqw #4,sp
	movel d0,d1
	movel d1,d0
	jra L24
L24:
	unlk a6
	rts
	.even
.globl _isupper
_isupper:
	link a6,#0
	movel a6@(8),sp@-
	jbsr _isupper_
	addqw #4,sp
	movel d0,d1
	movel d1,d0
	jra L25
L25:
	unlk a6
	rts
	.even
.globl _isxdigit
_isxdigit:
	link a6,#0
	movel a6@(8),sp@-
	jbsr _isxdigit_
	addqw #4,sp
	movel d0,d1
	movel d1,d0
	jra L26
L26:
	unlk a6
	rts
	.even
.globl _tolower
_tolower:
	link a6,#0
	movel a6@(8),sp@-
	jbsr _tolower_
	addqw #4,sp
	movel d0,d1
	movel d1,d0
	jra L27
L27:
	unlk a6
	rts
	.even
.globl _toupper
_toupper:
	link a6,#0
	movel a6@(8),sp@-
	jbsr _toupper_
	addqw #4,sp
	movel d0,d1
	movel d1,d0
	jra L28
L28:
	unlk a6
	rts
