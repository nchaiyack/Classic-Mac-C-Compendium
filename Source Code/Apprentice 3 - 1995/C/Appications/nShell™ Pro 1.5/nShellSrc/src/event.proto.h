/* ==========================================

	event.proto.h
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

#ifndef __APPLEEVENTS__
#include <AppleEvents.h>
#endif

void event_do( EventRecord *AERecord );
int	 event_got_parms (AppleEvent *theAppleEvent);
void event_init( void );
int  event_install_core( AEEventID event, EventHandlerProcPtr handler);
int  event_vRefNum( FSSpec *specPtr );

pascal OSErr event_open_app(AppleEvent *messagein, AppleEvent *reply, long refIn);
pascal OSErr event_quit_app(AppleEvent *messagein, AppleEvent *reply, long refIn);
pascal OSErr event_open_docs(AppleEvent *messagein, AppleEvent *reply, long refIn);
pascal OSErr event_print_docs(AppleEvent *messagein, AppleEvent *reply, long refIn);
