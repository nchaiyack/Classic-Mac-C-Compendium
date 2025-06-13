/*
*	iNetwork.c
*	General network initialization routines
*	Called only by init.c
*
*****************************************************************
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1986-1993,									*
*	Board of Trustees of the University of Illinois				*
*****************************************************************
*  Revisions:
*  7/92		Telnet 2.6:	Initial version.  Jim Browne
*/

#ifdef MPW
#pragma segment INIT
#endif

#include "TelnetHeader.h"
#include "iNetwork.proto.h"
#include "debug.h"
#include "InternalEvents.h"
#include "telneterrors.h"			/* For printing errors */
#include "network.proto.h"	// For Mnetinit proto
#include "netevent.proto.h"	// For Stask & setblocksize proto
#include "bkgr.proto.h"			// For StartUpFTP proto

extern QHdr		gEventsQueue, gEventsFreeQueue;
extern short	gQueueLength;

/*************************************************************************/
/* neteventinit
*  load up the pointers for the event queue
*  makes a circular list to follow, required for error messages
*/
void neteventinit( void)
{
	gEventsFreeQueue.qHead = 0;
	gEventsFreeQueue.qTail = 0;
	gEventsFreeQueue.qFlags = 0;
	gEventsQueue.qHead = 0;
	gEventsQueue.qTail = 0;
	gEventsQueue.qFlags = 0;
	
	ChangeQueueLength(NEVENTS);
	gQueueLength = NEVENTS;
}

/* initnet - init all of the network stuff... */
void initnet( void)
{
	neteventinit();							/* initializes for error messages to count */

	Mnetinit();
	setblocksize(512);						// BUGG
	StartUpFTP();

#ifdef PC
	Srcpmode(Scon.rcp);
#endif
	Stask();
}