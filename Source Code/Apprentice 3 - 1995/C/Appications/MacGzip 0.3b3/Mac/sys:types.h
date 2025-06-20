/* sys/types.h - poorman sys/types.h */

/*
modificiation history
---------------------
01b,20may95,ejo  cooparate with P.J Plaugers Standard C headers.
01a,01nov94,ejo  written.
*/

#ifndef INC_sys_types_h
#define INC_sys_types_h

#if defined (__MWERKS__) && !defined (_YVALS)
#include <yvals.h>
#endif

typedef short			dev_t;
typedef long			ino_t;
typedef unsigned long	mode_t;
typedef int 			nlink_t;
typedef long			off_t;

#ifdef __MWERKS__
#ifndef _SIZET
#define _SIZET
typedef _Sizet			size_t;
#endif
#elif defined (THINK_C)
#include <size_t.h>
#else
#ifndef __size_t__
#define __size_t__
typedef unsigned int	size_t;
#endif
#endif


typedef unsigned int	ssize_t;
typedef int				uid_t;
typedef int				gid_t;
typedef int				pid_t;


#endif /* INC_sys_types_h */
