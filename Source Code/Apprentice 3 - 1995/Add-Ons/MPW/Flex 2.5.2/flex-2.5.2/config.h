/***
 *
 * config.h - For Macintosh System 7.5, MPW 3.4
 *			  by Christopher E. Hyde, 95-06-29
 *
 ***/

#include <Types.h>
extern int	unlink	(char*);

// Define to empty if the keyword does not work.
//#undef const

// Define to `unsigned' if <sys/types.h> doesn't define.
//#undef size_t

// Define if you have the ANSI C header files.
#define STDC_HEADERS

// Define if you have the <malloc.h> header file.
//#undef HAVE_MALLOC_H

// Define if you have the <string.h> header file.
#define HAVE_STRING_H

// Define if you have the <sys/types.h> header file.
#define HAVE_SYS_TYPES_H

// Define if you have <alloca.h> and it should be used (not on Ultrix).
//#define HAVE_ALLOCA_H

// Define if platform-specific command line handling is necessary.
#undef NEED_ARGV_FIXUP

#include <CursorCtl.h>
