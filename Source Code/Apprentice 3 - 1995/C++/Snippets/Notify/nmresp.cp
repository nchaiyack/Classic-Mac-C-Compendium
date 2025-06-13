#include <stddef.h>

#include <Memory.h>
#include <Notification.h>

#include "notification_request.h"
#include "nmresp.h"
//
// From the TPM manual, page 282:
//
// =====================================================================
//
// Warning
//			To use objects in a code resource, you must turn on
//			the Multi-Segment option, even if your code resource
//			has only one segment.
//
// =====================================================================
//
// This is the NMResponse routine which must be present in 'NTFY' resource #63
// for class 'notify_user' to work properly
//
pascal void main( struct NMRec *theNMRec)
{
	#ifdef DEBUG
		DebugStr( "\pNMResponse proc");
	#endif
	notification_request *theRequest = (notification_request *)theNMRec->nmRefCon;
	notify_info *theInfo = theRequest->theInfo;

	(void)NMRemove( theNMRec);
	DisposePtr( (char *)theNMRec->nmStr);
	delete theRequest;
	
	theInfo->num_answered += 1;
	if( theInfo->num_answered == theInfo->num_constructed)
	{
		//
		// remove all evidence. Normally the application would do this when
		// the last instance was destructed, but at that time there apparently
		// was still at least one notification unanswered.
		//
		// Note that this code may be buggy since the last 'DisposPtr'
		// frees the block this code is in.
		//
		delete theInfo;
	}
}
