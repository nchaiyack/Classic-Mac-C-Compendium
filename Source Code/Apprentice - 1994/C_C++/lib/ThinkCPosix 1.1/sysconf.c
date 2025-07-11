/* sysconf.c						POSIX 4.8.1
 *	long int sysconf(int name);
 *
 *	POSIX allows some of the values in <limits.h>
 *  to be increased at run time.
 *  The sysconf() function allows such values
 *  to be checked at run time.
 *  We do not use this facility.
 */

#include "ThinkCPosix.h"

long int sysconf(name)
int name;			/* property being inspected */
{
  switch(name) {
	case _SC_CLOCKS_PER_SEC:
		return (long) CLOCKS_PER_SEC;

	case _SC_OPEN_MAX:
		return (long) OPEN_MAX;

	case _SC_JOB_CONTROL:
		return -1L;			/* no job control */

	case _SC_SAVED_IDS:
		return -1L;			/* no saved uid/gid */

	case _SC_VERSION:
		return (long) _POSIX_VERSION;

	default:
		errno = EINVAL;
		return -1L;
  }
}
