/*
	AppleEventQueue.c
	
	A simple queue used to keep track of suspended Apple events
	
	Created:	02 Sep 1992	Constructor, Put, Get � add a destructor later? (it should call an error-returning handler)
	Modified:	
	
	Copyright � 1992 by Paul M. Hoffman
	Send comments or suggestions to paul.hoffman@umich.edu -or- dragonsmith@umich.edu
	
	This source code may be freely used, altered, and distributed in any way as long as:
		1.	It is GIVEN away rather than sold (except as expressly permitted by the author)
		2.	This statement and the above copyright notice are left intact.

*/

#include	"AppleEventQueue.h"

AppleEventQueue::AppleEventQueue (void)
{
	head = tail = NULL;
	numAEvents = 0;
}

OSErr AppleEventQueue::Put (AppleEvent *event, AppleEvent *reply, AEHandlerFunc handler, long refcon)
{
	// Add an Apple event and its associated data to end the queue
	
	PendingAERec	**h;
	
	h = (PendingAERec **) NewHandle (sizeof (PendingAERec));
	if (h == NULL)
		return memFullErr;
		
	numAEvents++;
	
	(*h)->event = *event;
	(*h)->reply = *reply;
	(*h)->handler = handler;
	(*h)->refcon = refcon;
	(*h)->next = NULL;
	
	if (head == NULL)			// If the queue is empty, set head = the new entry
		head = h;
	else						// Otherwise, set 
		(*tail)->next = h;
	tail = h;					// Insertion is always done at the tail
	
	return noErr;
}

OSErr AppleEventQueue::Get (AppleEvent *event, AppleEvent *reply, AEHandlerFunc *handler, long *refcon)
{
	// Get an apple event and its associated data from the front of the queue

	PendingAERec	**h;
	
	if (numAEvents <= 0)				// Make a sanity check � we don't want to return a bogus function address!
		return noOutstandingHLE;
	
	h = head;
	if (h == NULL) {						// One more sanity check
		numAEvents = 0;
		return noOutstandingHLE;
	}
	
	head = (*h)->next;
	if (--numAEvents == 0)
		tail = NULL;
		
	// Return values
	*event = (*h)->event;
	*reply = (*h)->reply;
	*handler = (*h)->handler;
	*refcon = (*h)->refcon;
	
	DisposHandle ((Handle) h);
}

