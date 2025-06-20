/*	NAME:
		NotifyMsg.c

	WRITTEN BY:
		Joe Holt
				
	MODIFIED BY:
		Dair Grant

	DESCRIPTION:
		When your INIT/cdev runs into a problem, clean things up, show the X'ed
		version of your icon and call NotificationMessage() with the ID of
		the string you want displayed.

	NOTES:
		�	Depends on a separate code resource to remove the actual
			notification message from the System Heap.

	___________________________________________________________________________

	VERSION HISTORY:
		(June 1990, jhh)
			�	Response to Usenet news posting. Archived in Usenet Macintosh
				Programmer's Guide.

		(Oct 1993, dg)
			�	Altered the params to NotificationMessage to allow specifying of
				the 'STR#' resource that the error string is pulled from. If no
				string can be found, no note is posted.
			�	Altered the logic to do with 'RESP' code. If we can't get a 'RESP' code
				we still post the note, but allow it to stay stuck in the System Heap.
				The memory hit is negligible compared to the advantage of the user
				seeing the note.

		(Jan 1994, dg)
			�	Changed the type of the 'RESP' code to 'CODE'. Specified in
				ESConstants.h


	___________________________________________________________________________
*/
//=============================================================================
//		Include files																	 
//-----------------------------------------------------------------------------
#include <Memory.h>
#include <Notification.h>
#include "ESConstants.h"
#include "NotifyMsg.h"





//=============================================================================
//		Private defines															 
//-----------------------------------------------------------------------------
//	T_NMInstall and T_Unimplemented are Mac toolbox trap numbers used to
//	test for the existence of the Notification Manager.
#define T_NMInstall         (0xA05E)
#define T_Unimplemented     (0xA89F)










//=============================================================================
//		NotificationMessage : Post a Notification Manager request.																 
//-----------------------------------------------------------------------------
//		Note :	The 'RESP' code is needed to dispose of the NM record we leave
//				in the System Heap. If the 'RESP' code cannot be loaded, the
//				record is stuck until the next reboot. As these records are pretty
//				small - this isn't that important.
//-----------------------------------------------------------------------------
void NotificationMessage(int theStrings, int theStr)
{	register NMRec  *note;
    register Handle responseCode;
    register long   size;
    register THz    svZone;
    Str255          errorText;




	// Make sure we've got a Notification Manager.
	if (NGetTrapAddress(T_NMInstall, OSTrap) != NGetTrapAddress(T_Unimplemented, ToolTrap))
		{
		// All of the memory we allocate from here on out is in the System
		// Heap.  First create a Notification Manager record and fill it in.
		svZone	= TheZone;
		TheZone	= SysZone;
		note	= (NMRec *) NewPtr(sizeof(NMRec));
		if (!note)
			goto exit;
		note->qType		= nmType;
		note->nmMark	= 0;
		note->nmIcon	= 0L;
		note->nmSound	= (Handle) -1;



		// Get the error message corresponding to the error number given.
		// For maximum performance, the STR# resource should be tagged
		// "Preload" and not "System Heap".  This way, you can be sure
		// the messages will be there even if memory space is the cause of
		// the error.
		// 
		// We create a pointer in the System Heap just big enough for the
		// string and copy the string into it.  Point the NM record at this
		// string. If the string's length is zero we abort.
		GetIndString(errorText, theStrings, theStr);
		size		= (*(unsigned char *) errorText) + 1;
		note->nmStr	= (StringPtr) NewPtr(size);
		if (!note->nmStr || errorText[0] == 0)
			{
			DisposPtr(note);
			goto exit;
			}
		BlockMove(errorText, note->nmStr, size);



		// The response procedure also must be in a pointer in the System
		// Heap.  You need to include the compiled code resource in your
		// INIT's resources.
		// 
		// Create a pointer just big enough for it and point the NM record
		// at it, also.
		//
		//    dg -	Changed logic so that note still gets posted even if we
		//			couldn't get a 'RESP' procedure to unload it.
		responseCode = GetResource(kRespCodeType, kRespCodeID);
		if (!responseCode)
			note->nmResp = (Ptr) -1;
		else
			{
			size = GetHandleSize(responseCode);
			note->nmResp = (ProcPtr) NewPtrSys(size);
			if (!note->nmResp)
				{
				DisposPtr(note->nmStr);
				DisposPtr(note);
				goto exit;
				}
			HLockHi(responseCode);
			BlockMove(*responseCode, note->nmResp, size);
			}



		// Now post the note.  As soon as startup is complete, the NM
		// will display the note for the user's edification.  Hurrah.
		NMInstall(note);
exit:
		TheZone = svZone;
		}
}
