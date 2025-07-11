#include "bmminclude.h"
#if MEM_DEBUG>0

/*
 * fixes to CObject to record the birth of objects
 * by Aaron Wohl (n3liw+@cmu.edu)
 * public domain no rights reserved
 */

#include "CObject.h"
#include "MemWatchInternal.h"
#include "TCLUtilities.h"

CObject*	CObject::Copy()
{
	Handle		copyOfObject;
	OSErr		err;
	Boolean		savedAlloc;
	
	savedAlloc = SetAllocation( kAllocCanFail);
	
	copyOfObject = (Handle) this;		/* Objects are just handles			*/
	err = HandToHand(&copyOfObject);
	
	SetAllocation( savedAlloc);
	FailOSErr( err);
	
	mem_record_birth_c((CObject*)copyOfObject);

	return((CObject*) copyOfObject);
}
#endif
