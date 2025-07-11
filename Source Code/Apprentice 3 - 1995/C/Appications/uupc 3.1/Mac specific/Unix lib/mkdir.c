#ifdef THINK_C
# include "unixlibproto.h"
#endif THINK_C

#ifndef  THINK_C
#include <errno.h>
#include <memory.h>
#include <pb.h>
#include <aztec/shell.h>

#define _DEBUG
#include <max/debug.h>
#ifdef TEST
#include <stdio.h>
#endif

#ifndef NULL
#define NULL 0L
#endif

#include "mkdir.proto.h"

mkdir(char *path)
{

	char npath[255];
	CInfoPBRec cpb;
	int err;
	register char * cp;


	/* fix name, and get volume reference number */


	cpb.ioVRefNum =  hfixnam( path, npath );

#ifdef TEST
	fprintf( stderr, "opendir: %s\n", npath );
#endif

	cpb.ioNamePtr = ctop( npath );
	cpb.ioFDirIndex = 0;
	cpb.u.di.ioDrDirID = 0L;
	if ((err = PBDirCreate( &cpb, 0 )) != 0 ) {
		printmsg( 0, "setdir: PBDirCreate error %d on %s\n", err, path );
		return ENOENT;
	}



	return 0;

}

#ifdef TEST
main(void)
{
	char command[100];

	gets( command );
	fprintf( stderr, "%d\n", mkdir( command ));
}

#endif
#else	THINK_C
# include	<string.h>
#ifndef NULL
#define NULL 0L
#endif

OSErr mkdir(char *path)
{
	HParamBlockRec	pb;
	WDPBRec			wd;
	char			npath[255];
	int				idx, len;
	
	cnvMac(path, npath);		/* convert to Mac-style pathname */
	if (strchr(npath, ':') == NULL || *npath == ':') {
		/* relative to the current working directory */
#ifdef THINK_C
		memset((char *)(&wd), (int)NULL, (size_t)sizeof(wd));
#else THINK_C
		repmem((char *)&wd, "", 1, sizeof(wd));
#endif THINK_C
		/* get parent dir id */
		if (PBHGetVol(&wd, false) != noErr) {
			return -1;
		}
		pb.fileParam.ioDirID = wd.ioWDDirID;
	}
	else {
		/* absolute pathname */
		pb.fileParam.ioDirID = 0;
	}
	CtoPstr(npath);
	pb.fileParam.ioNamePtr = (StringPtr)npath;
	pb.fileParam.ioVRefNum = 0;
	pb.fileParam.ioCompletion = NULL;
	return(PBDirCreate(&pb, FALSE));
}
#endif	THINK_C
