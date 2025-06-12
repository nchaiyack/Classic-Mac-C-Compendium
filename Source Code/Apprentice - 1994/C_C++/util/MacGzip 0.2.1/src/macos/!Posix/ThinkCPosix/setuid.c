/* $Id: $ */

#include "ThinkCPosix.h"

int
setuid(uid_t uid)
{
	__uid = uid;
	return 0;
}

int
setgid(gid_t gid)
{
	__gid = gid;
	return 0;
}

