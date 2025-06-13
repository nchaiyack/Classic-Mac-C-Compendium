/*	NAME:
		THINKsample.c

	WRITTEN BY:
		Ammon Skidmore <ammon@cs.byu.edu>
				
	DESCRIPTION:
		This source allows you to test StandardGetAnything.c under THINK C.  It
		also provides an interesting compiled code comparison between Metrowerks
		and Symantec.

	___________________________________________________________________________
*/

#include "StandardGetAnything.h"

void main()
{	StandardFileReply	reply;

	InitGraf(&thePort);
	InitFonts();
	InitWindows();
	FlushEvents(everyEvent, 0);
	InitCursor();
	InitMenus();
	TEInit();
	InitDialogs(0L);

	reply = StandardGetAnything(3000);	// My get file DLOG id
	
	if (reply.sfGood)
		DebugStr(reply.sfFile.name);
}
