/****************************************************************
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1986-1992,									*
*	Board of Trustees of the University of Illinois				*
****************************************************************/

#ifdef MPW
#pragma segment main
#endif

#define GLOBALMASTER			/* Global structures are charged to us... see below */
#include "TelnetHeader.h"
#include "event.proto.h"
#include "netevent.proto.h"
#include "init.proto.h"
#include "network.proto.h"		// For netshut proto
#include "dnr.proto.h"			// For CloseResolver proto
#include "wind.h"
#include "MacBinary.h"
#include "binsubs.proto.h"			// For close_mb_files proto

#include "vgtek.proto.h"		// For TEKMAINunload proto
#include "vr.h"					// So vr.proto.h works (sigh)
#include "vr.proto.h"			// For ICRunload proto
#include "rsmac.proto.h"		// For RSunload proto
#include "vsem.proto.h"			// For VSunload proto
#include "configure.proto.h"	// For CONFIGUREunload proto
#include "macros.proto.h"		// For MACROSunload proto
#include "bkgr.proto.h"			// For FTPServerUnload proto
#include <Printing.h>			// So printing.proto.h doesn't die
#include "printing.proto.h"		// For PrintingUnload proto
#include "sets.proto.h"			// For SETSunload proto
#include "ftpbin.proto.h"		// For FTPClientunload proto
#include "rg0.proto.h"			// For TEKNULLunload proto
#include "rgmp.proto.h"			// For TEKMacPicunload proto
#include "maclook.proto.h"		// For MiscUnload proto

#include "mainseg.proto.h"

//#define	PERFORMANCE					//	Define to use MPW performance tools
#ifdef	PERFORMANCE
#include	<Perf.h>
TP2PerfGlobals	ThePGlobals;
#endif

Cursor *theCursors[NUMCURS];		/* all the cursors in a nice bundle */

WindRec
	*screens,		/* Window Records (VS) for :	Current Sessions */
	*ftplog;

short scrn=0;

TelInfoRec	*TelInfo;			/* This is defined here and externed everywhere else. */

main(void)
{
	short	i;
	long	lastCount = 0;
	MaxApplZone();
	
	for (i=0; i<9; i++)
		MoreMasters();

#ifdef	PERFORMANCE
	ThePGlobals = nil;
	if (!InitPerf(&ThePGlobals, 10, 8, TRUE, TRUE, "\pCODE", 0, "\p", FALSE, 0, 0, 0))
		Debugger();
	PerfControl(ThePGlobals, TRUE);
#endif

	init();					/* JMB 2.6 - Call all init routines */
	UnloadSeg(&init);		/* Bye, bye to init routines... 	*/

	do {						/* BYU - Do this forever */
		UnloadSegments();
		DoEvents();
		if (!TelInfo->done) DoNetEvents();
		if (TickCount() - lastCount > 1800) {			// Once every 30 seconds
			CheckFreeMemory();
			lastCount = TickCount();
			}
		} while (!TelInfo->done);						/* BYU mod */
		
#ifdef	PERFORMANCE
	if (!PerfDump(ThePGlobals, "\pPerform.out", TRUE, 80))
		DebugStr("\pDump Failed");
	Debugger(); 
	TermPerf(ThePGlobals);
#endif

}

void	quit( void)
{
	if (TelInfo->numwindows>0)
		return;

	netshut();
	CloseResolver();			/* BYU 2.4.16 */

	close_mb_files();			/* BYU - Don't leave any files open! */
	TelInfo->done = 1;			/* BYU */
}

void	forcequit(void)
{
	quit();
	ExitToShell();
}

//	This is rather simple right now, but I'll make it smarter when I have the time.
void	CheckFreeMemory(void)
{
	long	space, block;
	
	PurgeSpace(&space, &block);
	
	if (space < (30 * 1024))
		Alert(MemoryLowAlert, NULL);
}

//	This unloads the segments that are safe to unload.  Right now, the DNR and network
//	segments ARE NOT safe to unload.  Do _not_ add anything to this unless you really
//	know what you are doing!  (I MEAN IT!)
void	UnloadSegments(void)
{
	UnloadSeg(&TEKMAINunload);
	UnloadSeg(&ICRunload);
	UnloadSeg(&RSunload);
	UnloadSeg(&VSunload);
	UnloadSeg(&CONFIGUREunload);
	UnloadSeg(&MACROSunload);
	UnloadSeg(&FTPServerUnload);
	UnloadSeg(&PrintingUnload);
	UnloadSeg(&SETSunload);
	UnloadSeg(&FTPClientunload);
	UnloadSeg(&TEKNULLunload);
	UnloadSeg(&TEKMacPicunload);
	UnloadSeg(&MiscUnload);
}