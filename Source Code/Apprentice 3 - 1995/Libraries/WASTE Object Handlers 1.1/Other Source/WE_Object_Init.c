// WASTE Object Archive intialization code
// by Michael F. Kamprath, kamprath@earthlink.net
//
// v1.0.1, 6 April 1995
//
// History
//
//		v1.0.1: Updated the handler installer calls to be compatible with v1.1a6
//				of WASTE (CWATE v1.1r10 or later).
//		v1.0.2: Updated the handler installer calls to be compatible with v1.1a8
//				of WASTE (CWASTE v1.1r12 or later).  Basically, they now use UPPs.
//				Code update actually done by Dan Crevier.
//		v1.1:	Changed code to use individual object installers.
//

#include <Drag.h>
#include "WASTE.h"
#include "WE_hfs_Handler.h"
#include "WE_PICT_Handler.h"
#include "WE_snd_Handler.h"

#include "WASTE_Objects.h"

OSErr   InstallAllWASTEObjHandlers( void )
{
OSErr   iErr;

	iErr = InstallPICTObject( nil );
	if (iErr) return(iErr);

	iErr = InstallSoundObject( nil );
	if (iErr) return(iErr);
	
	iErr = InstallHFSObject( nil );
	if (iErr) return(iErr);
        
	return(noErr);
}
