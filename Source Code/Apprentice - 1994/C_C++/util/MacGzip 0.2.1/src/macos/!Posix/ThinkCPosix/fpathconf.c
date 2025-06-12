/* $ld: $ */

#include "ThinkCPosix.h"

long fpathconf(fd, name)
int fd;				/* file descriptor being interrogated */
int name;			/* property being inspected */
{
/* POSIX allows some of the values in <limits.h> to be increased at
 * run time.  The pathconf and fpathconf functions allow these values
 * to be checked at run time.  We do not use this facility.
 */

  struct stat stbuf;

  switch(name) {

	case _PC_NAME_MAX:
		return( (long) NAME_MAX);

	case _PC_PATH_MAX:
		return( (long) PATH_MAX);

	case _PC_CHOWN_RESTRICTED:
		return( (long) 0);	/* We do not define CHOWN_RESTRICTED */

	case _PC_NO_TRUNC:		/* We do not define NO_TRUNC */
		return( (long) 0);

	default:
		errno = EINVAL;
		return(-1L);
  }
}
