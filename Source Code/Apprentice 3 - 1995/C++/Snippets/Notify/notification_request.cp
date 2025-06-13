#include <SegLoad.h>
#include <Memory.h>
#include <Resources.h>
#include <Notification.h>
#include <Processes.h>

#include <string.h>
#include <stdlib.h>

#include "notification_request.h"

notify_info::notify_info()
{
	Handle theResource = Get1Resource( 'NTFY', 63);
	
	if( theResource == 0)
	{
		DebugStr( "\pCould not read the NTFY resource. Will exit immediately");
		ExitToShell();
	}
	DetachResource( theResource);

	num_constructed = 0;
	num_answered    = 0;
	num_destructed  = 0;
	response_handle = (Handle) theResource;
	response_proc   = (NMProcPtr) *theResource;
}

notification_request::notification_request( const char * const message, int audible)
{
	#ifdef DEBUG
		DebugStr( "\pnotification_request::notification_request( message, flag)");
	#endif
	if( staticInfo == 0)
	{
		staticInfo = new notify_info;
	}
	theInfo = staticInfo;

	const int message_length = strlen_255( message);

	theNMRec.qType    = nmType;
	theNMRec.nmMark   = 0;
	theNMRec.nmIcon   = 0L;

	if( audible)
	{
		theNMRec.nmSound  = (Handle) -1L;
	} else {
		theNMRec.nmSound  = 0;
	}
	theNMRec.nmStr    = (unsigned char *)NewPtrSys( message_length + 1);		
	theNMRec.nmResp   = (UniversalProcPtr)theInfo->response_proc;
	theNMRec.nmRefCon = (long)this;

	if( theNMRec.nmStr != 0)
	{
		theNMRec.nmStr[ 0] = message_length;
		BlockMove( message, &theNMRec.nmStr[ 1], message_length);

		if( NMInstall( &theNMRec) == noErr)
		{
			staticInfo->num_constructed += 1;
		} else {
			if( (num_instances() == 0) && (num_to_be_answered() == 0))
			{
				delete staticInfo;
				staticInfo = 0L;
				theInfo    = 0L;
			}
			DisposePtr( (char *)theNMRec.nmStr);
		}
	}
}

notification_request::~notification_request()
{
	#ifdef DEBUG
		DebugStr( "\pnotification_request::~notification_request()");
	#endif
	//
	// we can be sure that theInfo != 0
	//
	theInfo->num_destructed += 1;

	if( num_instances() == 0)
	{
		if( num_to_be_answered() == 0)
		{
			delete staticInfo;
		}
		staticInfo = 0L; // force reAlloc; there _will_ be a delete when the last response_proc runs
	}
}

notify_info *notification_request::staticInfo = 0L;

const int notification_request::an_error_occurred = -1;

int notification_request::strlen_255( const char * const string)
{
	int result = 0;
	
	const char * pointer = string;
	
	while( (*pointer != 0) && (result < 255))
	{
		result  += 1;
		pointer += 1;
	}
	return result;
}

void notify_user( const char * const message, int audible)
{
	(void) new notification_request( message, audible);
}
