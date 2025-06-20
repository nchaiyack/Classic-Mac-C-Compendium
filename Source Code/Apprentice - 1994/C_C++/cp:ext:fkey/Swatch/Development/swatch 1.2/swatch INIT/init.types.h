
#ifndef _H_TYPES
#define _H_TYPES

#include <BBtypes.h>


#define VERSION				(100)
#define FILE_NAME_LENGTH	(31)
#define elementsof(array)	(sizeof(array) / sizeof(array[0]))

#define FSetUpA4()\
	asm {\
			subq.l	#8, A7\
			move.l	A0, (A7)\
			lea		@__globals, A0\
			move.l	(A0), 4(A7)\
			bne.s	@__a\
\
			move.l	A4, (A0)\
			move.l	(A7), A0\
			addq.l	#8, A7\
			return\
\
@__globals:	dc.l	0\
\
@__a:		move.l	(A7), A0\
			move.l	A4, (A7)\
			move.l	4(A7), A4\
	}

#define FRestoreA4()\
	asm {\
			move.l	(A7), A4\
			addq.l	#8, A7\
	}


#endif	/* _H_TYPES */
