/*
 * MacAE.c, (C) SPDsoft
 * 1994, MacGzip 0.3
 */
 
#include <AppleEvents.h>
#include "MacAE.h"
#include "ThePrefs.h"
#include "MacErrors.h"

extern	KeyMap	theKeys;
Boolean StartupFiles = true;


/* in MacGzip.c */
extern OSErr	DoOpen( FSSpec *myFSSpec );
extern void		MyBeep(void);



OSErr MyGotRequiredParams (AppleEvent *theAppleEvent)
{
	DescType	returnedType;
	Size	actualSize;
	OSErr	err;

	err = AEGetAttributePtr (theAppleEvent, keyMissedKeywordAttr,
									typeWildCard, &returnedType, nil, 0,
									&actualSize);
	if (err == errAEDescNotFound)	// you got all the required parameters
			return noErr;
	else if (!err)				// you missed a required parameter
			return errAEEventNotHandled;
	else						// the call to AEGetAttributePtr failed
			return err;
}


/******************************************************************************/
/* Standard AE */

pascal OSErr  MyHandleODoc (AppleEvent *theAppleEvent, AppleEvent* reply, long
														handlerRefCon)
{
	FSSpec		myFSS;
	AEDescList	docList;
	OSErr		err, res;
	long		index,
				itemsInList;
	Size		actualSize;
	AEKeyword	keywd;
	DescType	returnedType;

	err = AEGetParamDesc (theAppleEvent, keyDirectObject, typeAEList,
			&docList);
	if (err)
			return err;

	err = MyGotRequiredParams (theAppleEvent);
	if (err)
			return err;

	err = AECountItems (&docList, &itemsInList);

	GetKeys(theKeys);
	
	for (index = 1, res=0; index <= itemsInList; index++) {

			err = AEGetNthPtr (&docList, index, typeFSS, &keywd,
					&returnedType, (Ptr) &myFSS, sizeof(myFSS), &actualSize);
			if (err)
					return err;

			res += (GzipOK!=DoOpen(&myFSS));
	}

	err = AEDisposeDesc (&docList);
	
	if ((0==res) && (currPrefs.BeepWhenDone)) MyBeep();
	
	return res;
}

pascal OSErr  MyHandlePDoc (AppleEvent *theAppleEvent, AppleEvent *reply, long
														handlerRefCon)
{
	/* can't print by now */
	return ( errAEEventNotHandled );
}

pascal OSErr  MyHandleOApp (AppleEvent *theAppleEvent, AppleEvent *reply, long
														handlerRefCon)
{
	OSErr	err;

	StartupFiles = false;

#if 1
	/* 22may85,ejo: this is to avoid a warning about possible unintended assignment */
	if ((err = MyGotRequiredParams (theAppleEvent)) != noErr)
#else	
	if ( err = MyGotRequiredParams (theAppleEvent))
#endif
			return err;
	return ( noErr );
}


pascal	OSErr	MyHandleQuit (AppleEvent *theAppleEvent, AppleEvent *reply,
					long handlerRefcon)
{
	OSErr	err;
	extern Boolean quitting;
	
#if 1
	/* 22may85,ejo: this is to avoid a warning about possible unintended assignment */
	if ((err = MyGotRequiredParams(theAppleEvent)) != noErr) {
#else
	if (err = MyGotRequiredParams(theAppleEvent)) {
#endif
		// an error occurred:  do the necessary error handling
		return	err;
	}
	
	/* we should cleanup here */
	
	quitting = true;
	
	return ( noErr );
}

