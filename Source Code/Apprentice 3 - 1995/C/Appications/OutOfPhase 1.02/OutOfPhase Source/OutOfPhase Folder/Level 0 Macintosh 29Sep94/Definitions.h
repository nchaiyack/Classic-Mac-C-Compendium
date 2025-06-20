/* Definitions.h */

#ifndef Included_Definitions_h
#define Included_Definitions_h

/* Definitions module depends on: */
/* MiscInfo.h */
/* Audit */
/* Debug */

/* generally useful values and types */
#ifdef __cplusplus
	#define NIL (0)
#else
	#define NIL ((void*)0L)
#endif
#define True (0 == 0)
#define False (0 != 0)
typedef int MyBoolean;
#define CODE4BYTES(a,b,c,d) ((a)*(1L << 24) + (b)*(1L << 16) + (c)*(1L << 8) + (d))

/* available operating systems */
#define OSTYPEMACINTOSH (CODE4BYTES('M','A','C','S'))
#define OSTYPEWIN16 (CODE4BYTES('W','n','3','1'))
#define OSTYPEUNKNOWN (CODE4BYTES('W','a','t','\?'))

/* available processor types */
#define PROC68000 (CODE4BYTES('M','6','8','0'))
#define PROC68020 (CODE4BYTES('M','6','8','2'))
#define PROC80386 (CODE4BYTES('i','3','8','6'))
#define PROCPOWERPC (CODE4BYTES('M','P','P','C'))
#define PROCUNKNOWN (CODE4BYTES('D','u','m','b'))

/* what operating system are we running on. */
#define CURRENTOSTYPE (OSTYPEMACINTOSH)

/* what processor are we running on */
#ifdef THINK_C
	#if __option(mc68020)
		#define CURRENTPROCTYPE (PROC68020)
	#else
		#define CURRENTPROCTYPE (PROC68000)
	#endif
#else
	#define CURRENTPROCTYPE (PROC68000)
#endif

/* what line feed string is used on this system? */
#define SYSTEMLINEFEED "\x0d"

/* generalized optimized block move operation.  Blocks will be handled */
/* correctly even if the overlap */
void				MoveData(char* Source, char* Destination, long NumBytes);

/* generalized block move operation.  Blocks can't overlap */
void				CopyData(char* Source, char* Destination, long NumBytes);

#endif
