/*	NAME:
		sample.c

	WRITTEN BY:
		Ammon Skidmore <ammon@cs.byu.edu>
				
	DESCRIPTION:
		This source allows you to test StandardGetAnything.c.

	___________________________________________________________________________
*/

#include "StandardGetAnything.h"

void main()
{	StandardFileReply	reply;

	InitGraf(&qd.thePort);
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