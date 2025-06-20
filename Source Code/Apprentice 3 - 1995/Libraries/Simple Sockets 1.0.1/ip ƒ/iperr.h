/*
 * iperr.h
 *
 * Constants that define the UNIX errors for the socket-like interface
 *
 * Mike Trent 8/94
 *
 */

#ifndef _IPERR_
#define _IPERR_

#include <Types.h>

#define	EINTR			4		/* interrupted system call		*/
#define	EIO				5		/* I/O error		*/
#define	EBADF			9		/* Bad file number */
#define	EINVAL			22		/* Invalid argument */
#define	EMFILE			24		/* Too many open files */
#define	EFBIG			27		/* File too large			*/
#define	EPIPE			32		/* Broken pipe				*/
#define	ENOTSOCK		38		/* Socket operation on non-socket */
#define	ESOCKTNOSUPPORT	44		/* Socket type not supported */
#define	EOPNOTSUPP		45		/* Operation not supported on socket */
#define EPFNOSUPPORT	46		/* if family != AF_INET */
#define	EADDRINUSE		48		/* Address already in use */
#define	EADDRNOTAVAIL	49		/* Can't assign requested address */
#define EWOULDBLOCK     54
#define	EISCONN			56		/* Socket is already connected */
#define	EISNCONN		72		/* Socket is not connected */
#define ECONNRESET      73      /* Connection reset by peer */

#define errno 	gErrno
extern int		gErrno;		//defined in ip.c
extern OSErr	gMacErrno;	//defined in ip.c

#endif