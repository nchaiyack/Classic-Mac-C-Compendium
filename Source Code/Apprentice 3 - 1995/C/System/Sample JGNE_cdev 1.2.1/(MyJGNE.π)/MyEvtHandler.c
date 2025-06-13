/*
	MyEvtHandler.c

	Written by Ken Worley, 06/03/94, using Symantec Think C 7.0.
	Copyright 1994.
	AOL KNEworley
	internet KNEworley@aol.com
	
	Feel free to use this code in a project of your own, but give me proper
	credit in your documentation or about box.  Feel free to distribute this
	code in its entirety to anyone, but never do so without the copyright
	notice above nor without its accompanying files.  This code is NOT in
	the public domain.  Use of this code in a commercial product requires my
	permission.
	
	This code is called by the JGNE filter code to examine events and to
	take action on intercepted events.  MyEvtHandler returns nonzero if the event
	should be intercepted (returned to the calling application as a NULL event),
	or false if the event should be returned to the app as usual.  MyEvtHandler
	gets called for EVERY event sent to any application.
*/


#include <Processes.h>

/*************************** FUNCTIONS *****************************/

// The MyEvtHandler routine gets called to examine every event passed to
// any application.  Currently, it only checks to see if the event is
// a mouseDown event and if the same modifier keys are down that are set
// in the control panel.  If so, it plays a system beep and indicates
// that the event should be intercepted (by returning nonzero).  Otherwise,
// the routine simply returns zero to indicate the event should be handled
// by the system as usual (pass it to the application).
//
Boolean	MyEvtHandler( EventRecord *event )
{
	Boolean	retValue;
	
	if ( ( event->what == mouseDown ) &&
		( myData.CPmodifiers == event->modifiers ) )
	{
		SysBeep( 5 );
		retValue = true;
	}
	else
		retValue = false;
	
	return retValue;
}
