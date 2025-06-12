/***
 * CThread.h
 *
 *  Abstract class to implement a thread.
 *		Copyright © Gordon Watts 1994 (gwatts@fnal.fnal.gov)
 *
 ***/
#pragma once

#include "CMainThread.h"
#include <Threads.h>

/** Errors **/
#define errCouldNotMakeThread 500

class CThread : public CMainThread {
protected:
	ThreadOptions	theOptions;
	Size			theStackSize;
	Boolean			recycleThread;
	Boolean			deleteWhenDone;

	void			ThreadRoutine (void);			/* Method to be called in the thread */
	static pascal void
					*ThreadExtProc (void *param);	/* External thread procedure called */

public:
					CThread (void);
					~CThread (void);				/* Delete, kill off this thread too if running */

	void			Start (void);					/* Start up the thread */
	void			Kill (void);					/* Kill off a thread */

	void			DeleteOnFinish (Boolean doIt);	/* Release obj when done? */

	void			Sleep (void);					/* Put ourselves to sleep */
};