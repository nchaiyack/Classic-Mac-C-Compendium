/***
 *
 *	Updated: by Christopher E. Hyde, 95-06-30
 *
 ***/

#ifndef _TYPES_H_
#define	_TYPES_H_

typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;
typedef	unsigned short	ushort;		// Sys V compatibility

typedef	char*	caddr_t;		// core address
typedef	long	daddr_t;		// disk address
typedef	short	dev_t;			// device number
typedef	u_long	ino_t;			// inode number
typedef	long	off_t;			// file offset (should be a quad)
typedef	u_short	nlink_t;		// link count
typedef	long	swblk_t;		// swap offset
typedef	long	segsz_t;		// segment size
typedef	u_short	uid_t;			// user id
typedef	u_short	gid_t;			// group id
typedef	short	pid_t;			// process id
typedef	u_short	mode_t;			// permissions
typedef	u_long	fixpt_t;		// fixed point number

#include <stdlib.h>
#include <time.h>

#endif // !_TYPES_H_
