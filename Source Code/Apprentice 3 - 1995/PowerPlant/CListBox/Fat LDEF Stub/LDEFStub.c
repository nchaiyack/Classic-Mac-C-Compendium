// =================================================================================
//	LDEFStub.c			©1994 Harold Ekstrom, AG Group, Inc. All rights reserved.
// =================================================================================
//	This ldef stub calls the ldef proc stored in the list handle's refCon field.
//	It does the right thing for PowerPC code and can be compiled into a "safe"
//	fat code resource.

#ifndef __LISTS__
#include <Lists.h>
#endif

#ifndef __MIXEDMODE__
#include <MixedMode.h>
#endif

pascal void main( short lMessage, Boolean lSelect, Rect *lRect, Cell lCell,
	short lDataOffset, short lDataLen, ListHandle lHandle );

#ifdef	__powerc
ProcInfoType __procinfo = uppListDefProcInfo;
#endif

pascal void main( short lMessage, Boolean lSelect, Rect *lRect, Cell lCell,
	short lDataOffset, short lDataLen, ListHandle lHandle )
{
	ListDefProcPtr	ldefProc = (ListDefProcPtr) (**lHandle).refCon;
	
	if ( ldefProc ) {
		CallListDefProc( ldefProc, lMessage, lSelect, lRect, lCell, lDataOffset,
			lDataLen, lHandle );
	}
}
