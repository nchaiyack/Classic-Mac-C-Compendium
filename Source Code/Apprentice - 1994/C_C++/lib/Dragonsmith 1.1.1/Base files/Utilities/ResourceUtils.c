/*
	ResourceUtils.c
	
	Created	21 Aug 1992	CopyResource
	Modified	22 Aug 1992	Revised CopyResource and renamed it CopyResHandle
						Added SaveRes and PreserveRes
			29 Aug 1992	Added overwrite parameter to CopyResHandle
			10 Oct 1992	Made CopyResHandle smarter when replacing resources
			
	Copyright � 1992 by Paul M. Hoffman
	Send comments or suggestions to paul.hoffman@umich.edu -or- dragonsmith@umich.edu
	
	This source code may be freely used, altered, and distributed in any way as long as:
		1.	It is GIVEN away rather than sold (except as expressly permitted by the author)
		2.	This statement and the above copyright notice are left intact.

*/

#include	"ResourceUtils.h"
#include	"HandleUtils.h"

OSErr SaveRes (Handle h)
{
	OSErr	err;
	
	if (h != NULL && *h != NULL) {
		ChangedResource (h);
		if ((err = ResError ()) == noErr) {
			WriteResource (h);
			err = ResError ();
		}
		return err;
	} else
		return nilHandleErr;
}

OSErr CopyResHandle (Handle h, short destRF, Boolean overwrite)
{
	// Copy a resource to the open resource file specified by the destRF parameter
	
	// The handle specified by the parameter h must be a resource handle � it must have been obtained with a call to
	//	GetResource or any other resource-loading call.  It may be purged (i.e., an empty handle) � if so, the resource
	//	will be read into memory before any copying takes place.  An error will be returned if the resource couldn't be
	//	read into memory.  Regardless of what happens, the state of h will not be affected in any way
	
	// The resource is copied using the same type, ID, and name.  All resource attributes (Purgeable, Locked, Protected,
	//	etc.) will be preserved in the destination file's copy
	
	// If a resource of the same type and ID is already present in the destination file, then we'll either overwrite it with the block
	//	referred to by h (if overwrite == TRUE), or return with no error
	
	// If the resource has been changed, the changes will be preserved whether ChangedResource has been called or not
	
	short	ID, attrs, sourceRF, saveRF;
	ResType	type;
	Str255	name;
	OSErr	err;
	Handle	newHndl = NULL, tempHndl = NULL;
	char		hState;
	
	// Avoid thrashing memory with references to an invalid handle
	if (h == NULL)
		return resNotFound;
	
	// Make sure that we have a valid refNum
	if (destRF == kInvalidRefNum)
		return rfNumErr;
	
	// Make sure the resource does NOT belong to the destination file � if it does, then we're done
	if (HomeResFile (h) == destRF)
		return noErr;
		
	// Make sure the resource is in memory
	if (*h == NULL) {
		LoadResource (h);
		if (err = ResError ()) return err;
	}
		
	// Remember which resource file was the current one
	saveRF = CurResFile ();
	if (saveRF != destRF)
		UseResFile (destRF);

	// Find out what type, ID, name, and attributes the resource we're copying has
	GetResInfo (h, &ID, &type, name);
	if (err = ResError ()) goto END;
	attrs = GetResAttrs (h);
	if (err = ResError ()) goto END;
	
	// Don't let the resource be purged before we can copy it
	hState = HGetState (h);
	HNoPurge (h);
		
	// Check to see if there's already a resource of the same type and ID in the destination file
	SetResLoad (FALSE);					// Avoid actually reading the resource into memory
	tempHndl = Get1Resource (type, ID);
	SetResLoad (TRUE);					// Without this, the system would crash and burn very soon!
	if (tempHndl != NULL) {
		// If the resource already exists then check to see if we should overwrite it
		if (overwrite) {
			LoadResource (tempHndl);				// Read the resource into memory
			err = ResError ();
			if (err == noErr) {
				err = CopyHandle (h, tempHndl);		// Overwrite it with the data in the parameter handle
				if (err == noErr) {
					SetResInfo (tempHndl, ID, name);
					err = ResError ();
				}
				if (err == noErr) {
					SetResAttrs (tempHndl, attrs & ~resChanged);
					err = SaveRes (tempHndl);		// Save the modified resource
				}
			}
			ReleaseResource (tempHndl);			// Free up the memory it was using
			if (err == noErr)
				err = ResError ();
		} else
			err = addResFailed;
		goto END;
	}

	// Now make a copy of the resource and add it to the destination file with the correct attributes
	newHndl = h;
	if (err = HandToHand (&newHndl)) goto END;
	HNoPurge (newHndl);
	AddResource (newHndl, type, ID, name);
	if (err = ResError ()) goto END;
	SetResAttrs (newHndl, attrs);
	if (err = ResError ()) {
		RmveResource (newHndl);
		goto END;
	}
	
	// Save the resource to disk
	ChangedResource (newHndl);
	WriteResource (newHndl);

	// Make it a non-resource handle (another copy is safe and sound in the destination file, unless there was an error)
	if ((err = ResError ()) == noErr)
		DetachResource (newHndl);
	else
		RmveResource (newHndl);

END:

	if (newHndl != NULL)
		DisposHandle (newHndl);

	// Preserve saved states
	HSetState (h, hState);
	if (saveRF != destRF)
		UseResFile (saveRF);

	return err;
	
}

