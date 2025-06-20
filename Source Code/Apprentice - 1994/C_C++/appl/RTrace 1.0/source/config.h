/*
 * Copyright (c) 1988, 1992 Antonio Costa, INESC-Norte.
 * All rights reserved.
 *
 * This code received contributions from the following people:
 *
 *  Roman Kuchkuda      - basic ray tracer
 *  Mark VandeWettering - MTV ray tracer
 *  Augusto Sousa       - overall, shading model
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Antonio Costa, at INESC-Norte. The name of the author and
 * INESC-Norte may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/**********************************************************************
 *    RAY TRACING - Configuration - Version 7.3.2                     *
 *                                                                    *
 *    MADE BY    : Antonio Costa, INESC-Norte, October 1988           *
 *    ADAPTED BY : Antonio Costa, INESC-Norte, June 1989              *
 *    MODIFIED BY: Antonio Costa, INESC-Norte, August 1992            *
 **********************************************************************/

#define PROGRAM_VERSION "7.3.2"

#include <stdio.h>
#include <math.h>

/***** Constants *****/
#ifndef NULL
#define NULL (0)
#endif
#ifdef TRUE
#undef TRUE
#endif
#define TRUE (1)
#ifdef FALSE
#undef FALSE
#endif
#define FALSE (0)
#ifndef MAXINT
#define MAXINT (~(1 << (sizeof(int) * 8 - 1)))
#endif

/***** Types *****/
typedef
int             boolean;

typedef
double          real;

typedef
char           *char_ptr;

typedef
FILE           *file_ptr;

/***** Boolean operators *****/
#define NOT !
#define AND &&
#define OR  ||

#define BIT_NOT ~
#define BIT_AND &
#define BIT_OR  |
#define BIT_XOR ^

/***** Integer operators *****/
#define DIV /
#define MOD %

#define SHL <<
#define SHR >>

/***** Others *****/
#define PREINC(x) (++(x))
#define POSINC(x) ((x)++)
#define PREDEC(x) (--(x))
#define POSDEC(x) ((x)--)

#define SUCC(x) ((x) + 1)
#define PRED(x) ((x) - 1)
#define ODD(x)  ((x) & 1)

#define TRUNC(x) ((long int) floor((double) (x)))
#define ROUND(x) ((long int) floor((double) (x) + 0.5))

#define ABS(x)      ((real) fabs((double) (x)))
#define FRAC(x)     ((real) ((double) (x) - floor((double) (x))))
#define SIN(x)      ((real) sin((double) (x)))
#define COS(x)      ((real) cos((double) (x)))
#define SQR(x)      ((real) ((double) (x) * (double) (x)))
#define SQRT(x)     ((real) sqrt((double) (x)))
#define POWER(x, y) ((real) pow((double) (x), (double) (y)))
#define ARCCOS(x)   ((real) acos((double) (x)))

#define EXIT exit(0)

#ifdef THINK_C

#define PROTOTYPES

/* On the mac, we trap HALT to put up an error dialog and continue */

#define HALT mac_halt()

/* On the mac, this macro gives background tasks a little time */

#define PROCESS_MAC_EVENT	if (get_another_event) process_mac_event();

#else

#define HALT exit(1)

#define PROCESS_MAC_EVENT

#endif

/***** Compiler stuffs *****/
#ifdef THINK_C
#include <string.h>
#include <stdlib.h>
#include <console.h>
#ifndef TIME
#define TIME
#endif
#ifdef TIMES
#undef TIMES
#endif
#define RANDOM_START srand(1)
#define RANDOM       ((real) rand() / 32767.0 )
#else

#ifdef vms
#include <string.h>
#include <stdlib.h>
#ifdef TIME
#undef TIME
#endif
#ifndef TIMES
#define TIMES
#endif
#define RANDOM_START srand(1)
#ifdef RAND_MAX
#define RANDOM       ((real) rand() / RAND_MAX)
#else
#define RANDOM       ((real) rand() / (real) MAXINT)
#endif
#else

#ifdef _transputer
#include <stdlib.h>
#include <string.h>
#ifdef TIMES
#undef TIMES
#endif
#ifndef TIME
#define TIME
#endif
#define RANDOM_START srand(1)
#ifdef RAND_MAX
#define RANDOM       ((real) rand() / RAND_MAX)
#else
#define RANDOM       ((real) rand() / 32767.0)
#endif
#undef __DATE__
#undef __TIME__
#define ECHO
#else

#ifdef dos
#include <stdlib.h>
#include <string.h>
#ifdef __TURBOC__
#include <alloc.h>
#include <dos.h>
#endif
#ifdef TIMES
#undef TIMES
#endif
#ifndef TIME
#define TIME
#endif
#ifdef __TURBOC__
#ifdef MAIN_MODULE
extern unsigned int _stklen = 65000U;
#endif
#endif
#define RANDOM_START srand(1)
#ifdef RAND_MAX
#define RANDOM       ((real) rand() / RAND_MAX)
#else
#define RANDOM       ((real) rand() / (real) MAXINT)
#endif
#define ECHO
#else

#ifdef hpux
#include <memory.h>
#ifdef TIME
#undef TIME
#endif
#ifndef TIMES
#define TIMES
#endif
#define RANDOM_START srand(1)
#ifdef RAND_MAX
#define RANDOM       ((real) rand() / RAND_MAX)
#else
#define RANDOM       ((real) rand() / 32767.0)
#endif
#else

#include <memory.h>
#include <stdlib.h>
#ifdef TIME
#undef TIME
#endif
#ifndef TIMES
#define TIMES
#endif
#define RANDOM_START srand48(1)
#define RANDOM       ((real) drand48() )
#endif
#endif
#endif
#endif
#endif		/* ...for THINK_C */

#ifndef __DATE__
#define __DATE__ "Aug 11 1992"
#endif
#ifndef __TIME__
#define __TIME__ "00:30:00"
#endif

/***** If there is no void type, define NOVOID *****/
#ifdef NOVOID
#define void char
#define void_ptr char_ptr
#else
typedef
void           *void_ptr;
#endif

/***** If there are no register variables, define NOREGISTERS *****/
#ifdef NOREGISTERS
#define REG
#undef NOREGISTERS
#else
#define REG register
#endif

/***** If there are function prototypes, define PROTOTYPES *****/
#ifndef PROTOTYPES
#if __STDC__
#ifndef NOVOID
#define PROTOTYPES
#endif
#endif
#endif

#ifdef NOVOID
#undef NOVOID
#endif

/***** If there is a function that gives time, define TIME or TIMES *****/
#ifndef TIME
#ifndef TIMES
#define CPU_CLOCK ((real) 0)
#endif
#endif
/* Using time() function */
#ifdef TIME
#include <time.h>
/* CPU_CLOCK must return time in milliseconds */
#define CPU_CLOCK ((real) ((double) time(0) * 1000.0))
#undef TIME
#endif
/* Using times() function */
#ifdef TIMES
#ifdef vms                      /* VAX-VMS */
#include <time.h>
#define tms_utime proc_user_time
#define tms_stime proc_system_time
#define tms tbuffer_t
#else
#include <sys/types.h>
#include <sys/times.h>
#ifndef CLK_TCK
#ifdef ultrix                   /* ULTRIX */
#define CLK_TCK (60)
#else                           /* Others */
#define CLK_TCK (60)
#endif
#endif
#endif
/* CPU_CLOCK must return time in milliseconds */
#define CPU_CLOCK cpu_clock()
static real
cpu_clock()
{
#ifdef vms
  tms             cpu_time;
#else
  struct tms      cpu_time;
#endif
  (void) times(&cpu_time);
  return (real) ((double) cpu_time.tms_utime * 1000.0 / (double) CLK_TCK);
}
#undef TIMES
#endif

/***** If cannot assign structs to structs, define NOSTRUCTASSIGN *****/
#ifdef NOSTRUCTASSIGN
#define STRUCT_ASSIGN(d, s)\
(void) memcpy((char_ptr) &(d), (char_ptr) &(s), sizeof(d))
#undef NOSTRUCTASSIGN
#else
#define STRUCT_ASSIGN(d, s) (d) = (s)
#endif

#define ARRAY_ASSIGN(d, s, c)\
(void) memcpy((char_ptr) (d), (char_ptr) (s), (c) * sizeof(d))

/***** Input & Output *****/
#define IO_EOF   (-1)
#define IO_OK    (0)
#define IO_OPEN  (1)
#define IO_READ  (2)
#define IO_WRITE (3)
#define IO_SEEK  (4)

#define EOT     '\0'
#define NEWLINE '\n'
#define SPACE   ' '
#define TAB     '\t'

#define INPUT  stdin
#define OUTPUT stdout
#define ERROR  stderr

#define READ_BINARY  "r+b"
#define READ_TEXT    "r+t"
#define WRITE_BINARY "w+b"
#define WRITE_TEXT   "w+t"

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

static int      IO_status;

#define STATUS(file, code)\
IO_status = feof(file) ? IO_EOF : (ferror(file) ? code : IO_OK)


#ifdef THINK_C

	/* on the mac, we need to check when we're opening a temp file,
		and make sure we create it in the RTrace directory */

#define OPEN(file, name, mode)\
do {\
	if ((((char *) name)[0] == '~') && (((char *) name)[1] == '~'))\
		SetVol( (StringPtr) NULL, temp_folder_wd_id);\
	else\
		SetVol( (StringPtr) NULL, sff_file_wd_id);\
	(file) = fopen(name, mode);\
	IO_status = (file) ? IO_OK : IO_OPEN;\
} while (0)

#else

#define OPEN(file, name, mode)\
do {
  (file) = fopen(name, mode);\
  IO_status = (file) ? IO_OK : IO_OPEN;\
} while (0)

#endif


#define CLOSE(file)\
IO_status = fclose(file) ? IO_EOF : IO_OK

#define FLUSH(file)\
IO_status = fflush(file) ? IO_EOF : IO_OK

#ifdef THINK_C

/* When running on the mac, we need to send the output lines to the log.  */
extern void	mac_write(FILE *file, char *format, ...);

#define WRITE	(void) mac_write

#else

#define WRITE (void) fprintf

#endif

#define WRITE_CHAR(file, c)\
do {\
  PROCESS_MAC_EVENT	\
  (void) putc(c, file);\
  STATUS(file, IO_WRITE);\
} while (0)

static int
PEEK_CHAR(file)
  file_ptr        file;
{
  int             character;

  character = getc(file);
  STATUS(file, IO_READ);
  if (IO_status == IO_OK)
    return ungetc(character, file);
  return IO_EOF;
}

#define END_OF_LINE(file)\
(PEEK_CHAR(file) == NEWLINE)

#define READ_LINE(file)\
{\
  PROCESS_MAC_EVENT \
  while ((getc(file) != NEWLINE) AND((STATUS(file, IO_READ)) == IO_OK));\
  }\

#define READ_CHAR(file, character)\
do {\
  PROCESS_MAC_EVENT \
  (character) = (unsigned char) getc(file);\
  STATUS(file, IO_READ);\
} while (0)

#define READ_REAL(file, real)\
do {\
	PROCESS_MAC_EVENT \
	IO_status = fscanf(file, "%lf", real) ? IO_OK : IO_READ; \
	}\
while (0)

#define READ_STRING(file, string)\
{ PROCESS_MAC_EVENT	IO_status = fscanf(file, "%s", string) ? IO_OK : IO_READ; }

#define SEEK(file, offset)\
do { PROCESS_MAC_EVENT	IO_status = fseek(file, offset, SEEK_SET) ? IO_SEEK : IO_OK; }\
while (0)

/***** Memory allocation *****/

#ifdef THINK_C

/* on the mac, we make a list of all allocations so we can easily free
	them later */

#define ALLOC(p, s, c)\
(p) = (s *) mac_alloc((unsigned int) ((c) * sizeof(s)))

#define FREE(p) mac_free((char_ptr) (p))


#else

#define ALLOC(p, s, c)\
(p) = (s *) malloc((unsigned int) ((c) * sizeof(s)))

#define FREE(p) free((char_ptr) (p))

#endif
