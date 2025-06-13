/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°

	CThread.h
	
	Thread Manager class.
	
	SUPERCLASS = CObject.
	
	Copyright © 1994-95 Ithran Einhorn. All rights reserved.
	
°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */

#ifndef _H_CThread
#define _H_CThread

#include "Threads.h"				/* Apple Includes */
#include "CPtrArray.h"

/* Class definition */

#define		kPoolSegment	10		// number of threads to allocate at a time

class CThread {

protected:

	/* Class Variables */
	
	static	CPtrArray<CThread>	*cThreadList;
	static	long				threadCount;

	/* Instance Variables */
	Str32		threadName;
	ThreadID	itsThreadID;
	ThreadStyle	itsThreadStyle;
	void		*itsArgument;
	OSErr		lastThreadErr;
	Boolean		isActive;
	
public:

	/* Instance Variables */

	Boolean			active;
	
	/* Class Methods */
	
	static Boolean	cIsPresent(void);
	static void		cInitManager(void);
	static short	cGetTMVersion(void);
	static CThread	*ThreadObjectFromID(ThreadID threadID);
	static OSErr	YieldToThread (CThread *suggestedThread);
	static CThread	*GetCurrentThread (void);

	/* Instance Methods */
	
	CThread(ThreadStyle	threadStyle, ThreadEntryProcPtr callBackFunction, void *objectArgument, Size stackSize = 0);
	virtual ~CThread();
	
	ThreadID		GetID(void);
	void			SetName(Str32 name);
	OSErr			LastError(void);
	Boolean			IsActive(void);
	OSErr			CurrentStackSpace(unsigned long *freeStack);
	OSErr			GetThreadState(ThreadState *threadState);
	OSErr			SetThreadState(ThreadState newState,CThread *suggestedThread);
};

#endif

/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */
