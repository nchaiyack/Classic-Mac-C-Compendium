/*
	AppleEventQueue.h
*/

#pragma once

// Format for an Apple event handler function
typedef pascal OSErr (*AEHandlerFunc) (AppleEvent*, AppleEvent*, long);

typedef struct PendingAERec {
	AppleEvent			event;
	AppleEvent			reply;
	long					refcon;
	AEHandlerFunc		handler;
	struct PendingAERec	**next;
};

typedef struct PendingAERec PendingAERec;

class AppleEventQueue : indirect {

	protected:
		PendingAERec	**head;			// Handle to a record for the first pending Apple event
		PendingAERec	**tail;			// Handle to a record for the last pending Apple event
		short			numAEvents;		// Number of pending Apple events
		
	public:
						AppleEventQueue (void);		// Constructor
		virtual OSErr		Put (AppleEvent *event, AppleEvent *reply, AEHandlerFunc handler, long refcon);
		virtual OSErr		Get (AppleEvent *event, AppleEvent *reply, AEHandlerFunc *handler, long *refcon);
		
};

