#include "host.h"

#ifdef THINK_C
# include "unixlibproto.h"
#endif THINK_C

#include <string.h>
#ifdef	TEST
#	include	<stdio.h>
#endif	TEST
#ifndef NULL
#define NULL 0L
#endif

#include "chdir.proto.h"
#include "pcmail.h"
#include "uupcproto.h"

#define USEWD

chdir(char *path)
{

	char 		npath[255];
	CInfoPBRec	cpb;
	HParamBlockRec hpbr;
	WDPBRec		wdpb;
	int			vRef;
	OSErr		err;

	printmsg(1, "chdir(%s)", path );
	/* get default WD id */
	(void)GetVol((StringPtr)npath, &vRef);
	
	/* convert to Mac-style pathname */
	cnvMac( path, npath );

	cpb.dirInfo.ioVRefNum = 0;
	cpb.dirInfo.ioNamePtr = (StringPtr)CtoPstr(npath);
	cpb.dirInfo.ioFDirIndex = 0;
	cpb.dirInfo.ioDrDirID = 0L;

	if ((err = PBGetCatInfo( &cpb, 0 )) != noErr ) {
		printmsg(0, "chdir: PBGetCatInfo error %d on %s", err, path );
		return((int)NULL);
	}

#ifdef THINK_C
	memset((char *)(&wdpb), (int)NULL, (size_t)sizeof(wdpb));
	memset((char *)(&hpbr), (int)NULL, (size_t)sizeof(hpbr));
#else THINK_C
	repmem((char *)&wdpb, "", 1, sizeof(wdpb));
#endif THINK_C
	hpbr.volumeParam.ioVolIndex = -1;
	hpbr.volumeParam.ioNamePtr = (StringPtr) npath;
	if ((err = PBHGetVInfo(&hpbr, FALSE)) != noErr) {
		printmsg(0, "chdir: PBHGetVInfo error %d on %s", err, path );
		return((int)NULL);
	}	
	wdpb.ioVRefNum = hpbr.volumeParam.ioVRefNum;
	wdpb.ioWDProcID = APPSIGNATURE;
	wdpb.ioWDDirID = cpb.dirInfo.ioDrDirID;
	if ((err = PBOpenWD(&wdpb, FALSE)) != noErr) {
		printmsg(0, "chdir: PBOpenWD error %d on %s", err, path );
		return((int)NULL);
	}
	if ((err = SetVol((int)NULL, wdpb.ioVRefNum)) != noErr) {
		printmsg(0, "chdir: SetVol error %d on %s", err, path );
		return((int)NULL);
	}

#ifdef TEST
	fprintf( stderr, "chdir: %s\n", npath );
#endif
#if DEBUG
	printf("wd.ioVRefNum = %x, wd.ioWDDirID = %lx\n", wd.ioVRefNum, wd.ioWDDirID);
#endif
	return(noErr);	
}

#ifdef TEST
main(void)
{
	char command[100];

	gets( command );
	fprintf( stderr, "%d\n", chdir( command ));
}

#endif

