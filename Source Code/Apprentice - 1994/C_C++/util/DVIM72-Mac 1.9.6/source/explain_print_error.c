#include <stdio.h>
#include <PrintTraps.h>
#include "mac-specific.h"
#include "TextDisplay.h"

#define		printf	TD_printf

void Explain_print_error( OSErr the_error );

void
Explain_print_error( OSErr the_error )
{
	char message[256];
	char volname[256];
	short		vrefnum;
	ParamBlockRec	pb;
	OSErr		err;
	SysEnvRec	world;
	
	switch (the_error)
	{
		case iPrAbort:
			printf("\nPrinting cancelled by user.\n");
			break;
		case dskFulErr:
			err = SysEnvirons( 1, &world );
			// vrefnum = (**g_print_rec_h).prJob.iFileVol;
			pb.volumeParam.ioCompletion = NIL;
			pb.volumeParam.ioNamePtr = (StringPtr) volname;
			pb.volumeParam.ioVRefNum = world.sysVRefNum;
			pb.volumeParam.ioVolIndex = 0;
			err = PBGetVInfo( &pb, false );
			if (err != noErr)
				BlockMove( "\punknown", volname, 8 );
			// GetVol( (StringPtr)volname, &vrefnum );
			(void) PtoCstr( (StringPtr) volname );
			(void) sprintf( message,
				"There was not enough room on the disk '%s' for"
				" the spool file.  Free some space or print fewer pages.",
				volname );
			Show_error( message );
			break;
		case tmfoErr:
			Show_error( "Too many files open." );
			break;
		case wPrErr:
		case vLckdErr:
			Show_error( "Startup volume is write-protected, can't spool." );
			break;
		case iPrSavPFil:
			Show_error( "Saving spool file." );
			break;
		case controlErr:
			Show_error( "Unimplemented control instruction." );
			break;
		case iIOAbort:
			Show_error( "I/O error while printing." );
			break;
		case iMemFullErr:
			Show_error( "Not enough room in heap zone." );
			break;
		default:
			(void) sprintf( message, "Printing error %d.", the_error );
			Show_error( message );
			break;
	}
	UnloadSeg( sprintf );
}