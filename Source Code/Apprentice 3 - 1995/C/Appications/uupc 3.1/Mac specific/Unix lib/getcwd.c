#ifdef THINK_C
# include "unixlibproto.h"
#endif THINK_C

#ifndef	THINK_C
#include <memory.h>
#include <aztec/shell.h>

#ifdef TEST
#include <stdio.h>
#define _DEBUG
#include <max/debug.h>
#endif
#endif	THINK_C
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "host.h"

#ifndef NULL
#define NULL 0L
#endif

#define MACGETCWDFORMAT


#include "getcwd.proto.h"

char * getcwd(char *path, int size)
{

	static		char	name[255];
	char		*p, *dir;
	OSErr		rc;
	WDPBRec		wd;

#ifdef THINK_C
	memset((char *)(&wd), (int)NULL, (size_t)sizeof(wd));
#else THINK_C
	repmem((char *)&wd, "", 1, sizeof(wd));
#endif THINK_C
	
	if (PBHGetVol(&wd, false) != noErr) {
		return NULL;
	}

	if (av_getname(name, wd.ioWDDirID)) {
		return NULL;
	}
	if (*(name+strlen(name)-1) == DIRCHAR) *(name+strlen(name)-1) = '\0';
	dir = name;
#ifndef MACGETCWDFORMAT
	/* convert to Unix style name */
	if ((p = index(name, DIRCHAR)) == NULL) p = name;
	else {
		dir = p;
		*p++ = SEPCHAR;		/* remove vol name and inser leading / */
	}
	/* replace all : with / */
	while (*p) {
		if (*p == DIRCHAR) *p = SEPCHAR;
		p++;
	}
#endif
	if (path != NULL) {
		if (strlen(dir) > size) return(NULL);
		strcpy(path, dir);
		return path;
	}
	else {
		return(dir);
	}
}

int av_getname(char *name, long n)
{
	CInfoPBRec	pb;
	OSErr		rc;
	char		space[32];

#ifdef THINK_C
		memset((char *)(&pb), (int)NULL, (size_t)sizeof(pb));
#else THINK_C
		repmem((char *)&pb, "", 1, sizeof(pb));
#endif THINK_C
	
	pb.dirInfo.ioNamePtr    = (StringPtr)space;
	pb.dirInfo.ioFDirIndex  = -1;
	pb.dirInfo.ioDrDirID    = n;
		
	if (PBGetCatInfo(&pb, FALSE) != noErr) {
		return -1;
	}
	PtoCstr(space);
	if (n <= 2) {
		strcpy(name, space);
	} else {
		av_getname(name, pb.dirInfo.ioDrParID);
		strcat(name, ":");
		strcat(name, space);
	}
	return 0;
}

#ifdef TEST
main(void)
{

	fprintf( stderr, "%s\n", getcwd( NULL, 0 ));
}

#endif

