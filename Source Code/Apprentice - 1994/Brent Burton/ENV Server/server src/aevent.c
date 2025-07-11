/*
** aevent.c
**
** This module contains all the code for the program to handle the
** required Apple Events.  Of the four types, only two really
** pertain to the program:  open and quit.
*/

#include "es_headers"

#if LOGGING
#	include <stdio.h>
#	include "logging.h"
#endif /* LOGGING */

#include <Errors.h>
#include "envserver.h"
#include "aevent.h"
#include "hash.h"
#include "utils.h"
#include "LoadEnv.h"


static pascal OSErr  QuitHandler    (AppleEvent*, AppleEvent*, long);
static pascal OSErr  OAppHandler    (AppleEvent*, AppleEvent*, long);
static pascal OSErr  ODocHandler    (AppleEvent*, AppleEvent*, long);
static pascal OSErr  GetEnvHandler  (AppleEvent*, AppleEvent*, long);
static pascal OSErr  PutEnvHandler  (AppleEvent*, AppleEvent*, long);

static void PostEnvReply( AppleEvent *, TargetID *, Ptr, long, OSType);
static OSErr ESGetRequestParams(AppleEvent *, Ptr *, short *);


void InitAEs(void)
{
	OSErr err;
	
	err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, QuitHandler, 0, FALSE);
	err = AEInstallEventHandler( kCoreEventClass, kAEOpenApplication, OAppHandler, 0, FALSE);
	err = AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments, ODocHandler, 0, FALSE);
    err = AEInstallEventHandler( kESEventClass,   kESGetEnvMsg, GetEnvHandler, 0, FALSE);
    err = AEInstallEventHandler( kESEventClass,   kESPutEnvMsg, PutEnvHandler, 0, FALSE);

} /* InitAEs */



void DoAppleEvent(EventRecord *theEvent)
{
	OSErr err;

	err = AEProcessAppleEvent(theEvent);  /* dispatch the handler for event. */
} /* DoAppleEvent */


/*===================================  HANDLERS FOLLOW... */

static pascal OSErr QuitHandler(AppleEvent *theAppleEvent,
				AppleEvent *reply, long handlerRefcon)
{
	extern Boolean gDone;
	
	gDone = TRUE;	/* don't call ExitToShell() here! or so says Apple... */
	return noErr;
} /* QuitHandler */


static pascal OSErr OAppHandler(AppleEvent *theAppleEvent,
				AppleEvent *reply, long handlerRefcon)
{
	LoadEnvFile("Environment");
	return noErr;
} /* OAppHandler */


static pascal OSErr ODocHandler(AppleEvent *theAppleEvent,
				AppleEvent *reply, long handlerRefcon)
{
	/* for each filename in the AEDescList, do: */
	/*    get filename                          */
	/*    LoadEnvFile( filename);               */
	return noErr;
} /* ODocHandler */


/******************************************************/
/*
** GetEnvHandler()
*/
static pascal OSErr GetEnvHandler(AppleEvent *theAE,
				AppleEvent *reply, long handlerRefcon)
{
	HashNodePtr hnp;
	OSErr err;
	short msgLen;
	Ptr msgBuf;
	AEDescList paramList;
	Boolean succOrFail = FALSE;

	/*
	** Get the parameters out:  the string length (typeShortInteger)
	** and the string's bytes.  (typeChar).
	**
	** Put the string (converted to a C string) into msgBuf.
	** Put the length of the string into msgLen.
	*/

	err = ESGetRequestParams( theAE, &msgBuf, &msgLen);

	if (err != noErr)
	{
		if (msgBuf != NULL)
			DisposPtr(msgBuf);
		return err;
	}
	
	/*
	** At this point, we've received the request; msgBuf contains the C string 
	** which is the name of the environment variable.  Now look up that var
	** name, and if it exists, return the var's string value.
	*/
	
	hnp = hash_get_entry( msgBuf);

	if (hnp == NULL)
		return errESNotFound;
	
	/*
	** OK, now add params to the reply event and return error code.
	*/
	
	DisposPtr( msgBuf);
	msgLen = mystrlen((char*)hnp->value);
	msgBuf = NewPtr( msgLen+1 );
	if (msgBuf == NULL)
		return memFullErr;
	succOrFail = TRUE;
	mystrcpy( msgBuf, hnp->value);
	
	
	err = AEPutKeyPtr( reply, keyESFlag, typeBoolean, (Ptr)&succOrFail,
				sizeof(succOrFail));
	if (err != noErr)
		return err;

	err = AEPutKeyPtr( reply, keyESLen, typeShortInteger, (Ptr)&msgLen,
				sizeof(msgLen));
	if (err != noErr)
		return err;

	/*
	** Note that I just return the string's raw bytes, no C trailing nul
	** nor Pascal's preceding length byte.
	*/
	err = AEPutKeyPtr( reply, keyESString, typeChar, msgBuf, msgLen);
	
	return err;
	
} /* GetEnvHandler */


/******************************************************/
/*
** PutEnvHandler()
*/
static pascal OSErr PutEnvHandler(AppleEvent *theAE,
				AppleEvent *reply, long handlerRefcon)
{
	OSErr err;
	short msgLen;
	Ptr msgBuf;
	AEDescList paramList;
	Boolean succOrFail = FALSE;
	int herr;

	/*
	** Get the parameters out:  the string length (typeShortInteger)
	** and the string's bytes.  (typeChar).
	**
	** Put the string (converted to a C string) into msgBuf.
	** Put the length of the string into msgLen.
	*/

	err = ESGetRequestParams( theAE, &msgBuf, &msgLen);

	if (err != noErr)
	{
		if (msgBuf != NULL)
			DisposPtr(msgBuf);
		return err;
	}
	
	/*
	** At this point, we've received the request; msgBuf contains the C string 
	** which is the expression  "var=value" where var is the name of the
	** environment variable and value is the string value of it.
	*/
	
	herr = hash_add_entry( msgBuf);
	DisposPtr( msgBuf);

	if ( herr != H_NOERR )
		return errESNotFound;
	
	/*
	** OK, now add params to the reply event and return error code.
	*/
	succOrFail = TRUE;
	
	err = AEPutKeyPtr( reply, keyESFlag, typeBoolean, (Ptr)&succOrFail,
				sizeof(succOrFail));
	return err;

} /* PutEnvHandler */



/*******************
** ESGetRequestParams
**
** This is called by the getenv() and putenv() handlers;
** It extracts the request (a length short and a string)
** from the apple event pointed to by theAE.  Once
** finished processing, *msgBuf will contain a C string
** and *msgLen will be the string's length.
*******************/
OSErr ESGetRequestParams(AppleEvent *theAE, Ptr *msgBuf, short *msgLen)
{
	OSErr err;
	AEDescList paramList;
	DescType foo;
	Size actualSize;
	Ptr tempBuf;
	short tempLen;
	
	/** Fetch the length of the string from the Apple Event **/
	
	err = AEGetKeyPtr( theAE, keyESLen, typeShortInteger, &foo,
						(Ptr)&tempLen, sizeof(short), &actualSize);
	if (err != noErr)
		return err;
	
	
	tempBuf = NewPtr( tempLen);  /* get buffer space plus NULL char */
	if (tempBuf == NULL)
		return memFullErr;
	
	
	/** Now fetch the actual string, converting it into a C string **/
	
	err = AEGetKeyPtr( theAE, keyESString, typeChar, &foo,
				tempBuf, tempLen, &actualSize);
	if (err != noErr)
		return err;
	tempBuf[ tempLen] = 0;  /* terminate string */
	
	*msgBuf = tempBuf;
	*msgLen = tempLen;
	
	return err;
} /* ESGetRequestParams */


