/* utime.h - function to set file access times */

/*
modification history
--------------------
01d,06jun95,ejo  setting creation date = mod date = struct utimbuf.modtime.
01c,22may95,ejo  added uglyness for CW.
01b,22may95,ejo  fixed bug when times arg to utime was NULL. 
01a,31oct94,ejo  written.
*/

#include <errno.h>
#include "catinfo.h"
#include "utime.h"

#ifndef EACCES
#define EACCES		13 /* Permission denied */
#endif
#ifndef EFAULT
#define EFAULT		14 /* Bad address */
#endif

/*******************************************************************************
*
* utime - set file (or directory) access and modification times.
*
* The access time is ignored. (No such thing on the Mac).
*
* !!! NOTE: 06jun95,ejo: we now set creation date equal to mod date because	!!!
* !!! some programs have problems with things that was changed before birth	!!!										!!!
*
* RETURNS: zero on succes, otherwise -1.
*/
int utime (const char *filename, const struct utimbuf *times)
	{
	CInfoPBRec info;
	long dirID;
	int err;
	
	if ((err = GetCatInfo (filename, &info, &dirID)) != 0)
		errno = err;
	else
		{
		info.hFileInfo.ioFlMdDat = times != NULL ? times->modtime : time (NULL);
		info.hFileInfo.ioFlCrDat = info.hFileInfo.ioFlMdDat; /* 06jun95,ejo */
		info.hFileInfo.ioDirID = dirID;
			
		if ((err = PBSetCatInfoSync (&info)) == noErr)
			return 0;
				
		errno = EACCES;
		}

	return -1;
	}
