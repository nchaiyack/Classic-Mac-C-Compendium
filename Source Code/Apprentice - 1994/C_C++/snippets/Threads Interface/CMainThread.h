/***
 * CMainThread.h
 *
 *  Implement a dummy thread that is really the main thread.  Mostly, we disable
 *  most thread calls.  This object *MUST* be created by the ThreadManager only.
 *  It relies on being created in the main thread!!!!!
 *		Copyright � Gordon Watts 1994 (gwatts@fnal.fnal.gov)
 *
 ***/
#pragma once

#include "CGWObject.h"
#include <Threads.h>

class CMainThread;

typedef struct {
	long			theAppA5;
	CMainThread		*threadObj;
} switcherInfo;

class CMainThread : public CGWObject {
private:
	short			savedLastError;			// Stuff saved by thread switcher routines
	long			savedLastMessage;
	char			savedDefaultPropagation;
	void			*savedTopHandler;

protected:
	ThreadID		theThread;
	switcherInfo	*theSInfo;

	void			SetupThreadSwitcher (Boolean useCurrentValues);
												// Setup thread switch handler.
	void			KillThreadSwitcher (void);	// Turn it off.

	static pascal void		switcherCallBackIn (ThreadID thread, void *ptr);
	void			DoSwitchIn (void);			// Restore globals...
	static pascal void		switcherCallBackOut (ThreadID thread, void *ptr);
	void			DoSwitchOut (void);			// Kill off globals

public:
					CMainThread (void);			// Init the stuff in this thread
					~CMainThread (void);		// Kill off everything in this thread

	Boolean			isRunning (void);			// Return true if we are up and running

	void			Start (void);				// Do the setup we need
	void			Kill(void);				// Kill the thread.

};