/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�

	CThread.cp
	
	Thread Manager class.
		
	Copyright � 1994-95 Ithran Einhorn. All rights reserved.
	
같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같� */

#include <CError.h>
#include <Constants.h>
#include <TBUtilities.h>
#include <TCLUtilities.h>

#include "CThread.h"					/* Other includes */

#include "string.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "ThreadUtil.h"

#ifdef __cplusplus
}
#endif

/* Constants & Macros */

#define RESET_ALRT_ID		2100	/* Reset alert resource ID */

/* Application globals */

/* Class variables initialization */

CPtrArray<CThread>	*CThread::cThreadList = NULL;
long				CThread::threadCount;

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cIsPresent
 *
 * Return TRUE if the Thread Manager is present.
 *
 */
 
Boolean CThread::cIsPresent(void)
{
	Boolean	isHere = false;
	long	res;
	short	comparebit;
	OSErr	threadErr;
	
	threadErr = Gestalt (gestaltQuickdrawVersion, &res);
	if (threadErr == noErr)
	{	
		threadErr = Gestalt (gestaltThreadMgrAttr, &res);
		if (threadErr == noErr)
		{
			comparebit = gestaltThreadMgrPresent;
	
			if (BitTst(&res, 31 - comparebit) == TRUE)
				isHere = true;
		}
	}

	return isHere;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cInitManager
 *
 * Thread Manager Initialization
 *
 */
 
void CThread::cInitManager(void)
{
	InitializeThreadUtilities();
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * cGetTMVersion
 *
 * return the version of the Thread Manager
 *
 */
 
short CThread::cGetTMVersion(void)
{
	return 0;
}	

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * ThreadObjectFromID
 *
 * find CThread object that has given ID
 *
 */

 		/* Test routine */

		static Boolean FindThread(CThread *theThread,long threadID)
		{
			return theThread->GetID() == (ThreadID)threadID;
		}


CThread *CThread::ThreadObjectFromID(ThreadID threadID)
{
	if (cThreadList == NULL)
		return nil;
	else
	{
		return (CThread *)cThreadList->FindItem1(FindThread, (long)threadID);
	}
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * YieldToThread
 *
 * Try to yield to specified Thread object
 *
 *
 */
 
OSErr CThread::YieldToThread (CThread *suggestedThread)
{
 return ::YieldToThread(suggestedThread->GetID());
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetCurrentThread
 *
 * Try to get the current Thread object
 *
 *
 */
 
CThread *CThread::GetCurrentThread (void)
{
 CThread	*currThread = nil;
 ThreadID	currentThreadID;
 
 if (::GetCurrentThread (&currentThreadID) == noErr)
 {
	currThread = ThreadObjectFromID(currentThreadID);
 }
 
 return currThread;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * IThread
 *
 * Initialization of the thread object
 *
 *
 */
 
CThread::CThread(ThreadStyle threadStyle, ThreadEntryProcPtr callBackFunction, void *objectArgument, Size stackSize)
{
	short	currentCount = 0;

	isActive = false;
	itsThreadStyle = threadStyle;
	itsArgument = objectArgument;

	lastThreadErr = noErr;
	
	if (cThreadList == NULL)	// make sure we're all initialized
		cInitManager();

	lastThreadErr = GetFreeThreadCount (itsThreadStyle, &currentCount);

	if (lastThreadErr == noErr && ! currentCount)
	{
		lastThreadErr = CreateThreadPool (itsThreadStyle, kPoolSegment, (Size)0);
	}

	if (lastThreadErr == noErr)
	{
		lastThreadErr = NewThread (itsThreadStyle, 
									callBackFunction, 	// callback function
									objectArgument,		// parameter to callback function
									stackSize,
							 		kUsePremadeThread,
							 		nil,
							 		&itsThreadID);

		if (lastThreadErr == noErr)
		{
			if (cThreadList == NULL)			/* first thread object ? */
			{			
				cThreadList = new CPtrArray<CThread>;
				threadCount = 0L;
			}
		
			cThreadList->Add(this);				/* add thread to list */
			threadCount++;
			isActive = true;
		}
	}
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * Destructor
 *
 * Dispose of a thread object
 *
 */
 
CThread::~CThread()
{
	ASSERT(cThreadList != NULL);
	
	threadCount--;
	cThreadList->Remove(this);			/* Dispose of the thread */
	
	if (cThreadList->IsEmpty())
		ForgetObject(cThreadList);		/* Dispose of the cluster */

	DisposeThread (itsThreadID, 0L, TRUE);
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetID
 *
 * get thread object's ID
 *
 */
 
ThreadID CThread::GetID(void)
{
	return itsThreadID;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * SetName
 *
 * Set the name for this thread object
 *
 */
 
void CThread::SetName(Str32 name)
{
	name[sizeof(threadName) - 1] = '\0';
	
	strcpy((char *)threadName, (char *)name);
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * LastError
 *
 * get last error
 *
 */
 
OSErr CThread::LastError(void)
{
	return lastThreadErr;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * IsActive
 *
 * return thread state
 *
 */
 
Boolean CThread::IsActive(void)
{
	return isActive;
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * CurrentStackSpace
 *
 * 
 *
 */
 
OSErr CThread::CurrentStackSpace(unsigned long *freeStack)
{
 return ThreadCurrentStackSpace (itsThreadID,freeStack);
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * GetThreadState
 *
 * 
 *
 */

OSErr CThread::GetThreadState(ThreadState *threadState)
{
 return ::GetThreadState (itsThreadID, threadState);
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

/*
 * SetThreadState
 *
 * 
 *
 */

OSErr CThread::SetThreadState(ThreadState newState,CThread *suggestedThread)
{
 return ::SetThreadState (itsThreadID, newState, suggestedThread->GetID());
}

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */

