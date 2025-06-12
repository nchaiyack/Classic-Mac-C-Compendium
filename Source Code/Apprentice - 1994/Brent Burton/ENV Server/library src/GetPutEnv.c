/*
** GetPutEnv.h
**
** Contains the source for the Getenv() and Putenv() calls.
** Implements the server-based environment variables on the Macintosh.
**
** The code is freely distributable and modifiable, but you must keep
** the following information in the code and any derived works.
**
** Copyright (C) 1993 by Brent Burton
** Brent Burton
** brentb@tamsun.tamu.edu
**
** 5/12/93 - created
** 5/31/93 - slightly modified for distributing.
*/

#include "es_headers"
#include <Traps.h>
#include <Errors.h>		/* Macintosh error codes */
#include <BDC.h>
#include <pascal.h>		/* for pascal-C string conversions */
#include "GetPutEnv.h"
#include "GetPutEnvP.h"
#include "utils.h"


static Boolean FindAProcess( OSType sig, ProcessSerialNumber *process);
static Boolean Check4Traps(void);
static OSErr ESFindServer(AEAddressDesc *target);
static OSErr ESGetReply( AppleEvent *theReply, Ptr *result);
static OSErr ESPutReply( AppleEvent *theReply, int *result);


/***********************************************************
** Getenv
**
** Looks up the value of varname's string and returns:
**  NULL:  if name doesn't exist or an error occurred.
**  Ptr: if name existed and its value could be fetched.
**
** Note that normal getenv() returns a pointer to a static
** area.  THIS Getenv() returns a pointer created by NewPtr.
** The caller will need to DisposPtr for cleanliness, but most likely,
** the pointer will not be large and can be ignored...(not suggested).
*****************/

char *Getenv (const char *varname)
{
	OSErr err, sendErr;
	AppleEvent theAE, theReply;
	AEAddressDesc target;
	short msgLen;
	Ptr result = NULL;

	if (!Check4Traps())
		return NULL;
	
	msgLen = (short)mystrlen( (Ptr)varname);
	err = ESFindServer( &target);
	
	if (err == noErr)
	{
		err = AECreateAppleEvent( kESEventClass, kESGetEnvMsg, &target,
				kAutoGenerateReturnID, kAnyTransactionID, &theAE);
		
		if ( err == noErr)
		{
			/* now the apple event has been created, add the length
			** of the string, and the string as two parameters.
			*/
			err = AEPutKeyPtr( &theAE, keyESLen, typeShortInteger, (Ptr)&msgLen,
					sizeof(short));
			
			if (err == noErr)
			{
				/* Just send string's bytes, no nul end nor length prefix */
				err = AEPutKeyPtr(&theAE, keyESString, typeChar, (Ptr)varname,
						(Size)msgLen);
				/*
				** Now send the request to the server & expect a
				** reply.
				*/
				if (err == noErr)
					sendErr = AESend( &theAE, &theReply, kAEWaitReply,
								kAENormalPriority, 600L /* 10 secs */,
								NULL, NULL);
				
				if (sendErr == noErr)
				{
					/* We sent and received a decent message.  Now extract
					** the string returned from the server if successful.
					*/
					err = ESGetReply( &theReply, &result);
				}
			}
			err = AEDisposeDesc( &theAE);
		}
		err = AEDisposeDesc( &target);
	}

	return result;
} /* Getenv */


/***********************************************************/

int Putenv (const char *s)
{
	OSErr err, sendErr;
	AppleEvent theAE, theReply;
	AEAddressDesc target;
	short msgLen;
	int result = 1;		/* assume failure */

	if (!Check4Traps())
		return 1;

	msgLen = (short)mystrlen( (Ptr)s);
	err = ESFindServer( &target);
	
	if (err == noErr)
	{
		err = AECreateAppleEvent( kESEventClass, kESPutEnvMsg, &target,
				kAutoGenerateReturnID, 0L, &theAE);
		
		if ( err == noErr)
		{
			/* now the apple event has been created, add the length
			** of the string, and the string as two parameters.
			*/
			err = AEPutKeyPtr( &theAE, keyESLen, typeShortInteger, (Ptr)&msgLen,
					sizeof(short));
			
			if (err == noErr)
			{
				/* Just send string's bytes, no nul end nor length prefix */
				err = AEPutKeyPtr(&theAE, keyESString, typeChar, (Ptr)s,
						(Size)msgLen);
				/*
				** Now send the request to the server & expect a
				** reply.
				*/
				if (err == noErr)
					sendErr = AESend( &theAE, &theReply, kAEWaitReply,
								kAENormalPriority, 600L /* 10 secs */,
								NULL, NULL);
				
				if (sendErr == noErr)
				{
					/* We sent and received a decent message.  Now extract
					** the string returned from the server if successful.
					*/
					err = ESPutReply( &theReply, &result);
				}
			}
			err = AEDisposeDesc( &theAE);
		}
		err = AEDisposeDesc( &target);
	}

	return result;
} /* Putenv */


/***********************************************************/

static OSErr ESGetReply( AppleEvent *theReply, Ptr *result)
{
	short msgLen;
	Ptr msgBuf=NULL;
	OSErr err;
	DescType foo;
	Size actualSize;
	Boolean flag;
	
	*result = NULL;
	
	err = AEGetKeyPtr( theReply, keyESFlag, typeBoolean, &foo, (Ptr)&flag,
			sizeof(Boolean), &actualSize);

	if (err == noErr && flag)
	{
		err = AEGetKeyPtr( theReply, keyESLen, typeShortInteger, &foo,
				(Ptr)&msgLen, sizeof(short), &actualSize);
				
		if (err == noErr)  /* then we have the string length */
		{
			msgBuf = NewPtrClear( msgLen + 1);  /* make +1 for nul end */
			if (msgBuf != NULL)
			{
				err = AEGetKeyPtr( theReply, keyESString, typeChar, &foo, msgBuf,
						msgLen, &actualSize);

				if (err == noErr)
				{
					*result = msgBuf;
				}
			}
			else
				return memFullErr;
		}
	}
	return err;
} /* ESGetReply */


/***********************************************************/

static OSErr ESPutReply( AppleEvent *theReply, int *result)
{
	short msgLen;
	Ptr msgBuf=NULL;
	OSErr err;
	DescType foo;
	Size actualSize;
	Boolean flag;
	
	err = AEGetKeyPtr( theReply, keyESFlag, typeBoolean, &foo, (Ptr)&flag,
			sizeof(Boolean), &actualSize);

	 /* Putenv()'s success(0) if no error & flag is true */
	*result = !(err == noErr && flag);
	
	return err;
} /* ESPuttReply */


/***********************************************************/

static Boolean FindAProcess( OSType sig, ProcessSerialNumber *process)
{
	Boolean found = FALSE;
	ProcessInfoRec info;

	process->highLongOfPSN = 0L;
	process->lowLongOfPSN = kNoProcess;
	info.processInfoLength = sizeof(ProcessInfoRec);
	info.processName = NULL;
	info.processAppSpec = NULL;
	
	while (GetNextProcess(process) == noErr)
	{
		if (GetProcessInformation(process, &info) == noErr)
		{
			if ( (info.processType == (long)'APPL') &&
				 (info.processSignature == sig))
			{
				found = TRUE;  /* we found it! */
				break;
			}
		}
	}
	return found;
} /* FindAProcess */


/***********************************************************/

static OSErr ESFindServer(AEAddressDesc *target)
{
	OSErr err = noErr;
	ProcessSerialNumber PSN;
	
	if ( FindAProcess( kESSignature, &PSN))
	{
		err = AECreateDesc( typeProcessSerialNumber, (Ptr)&PSN,
					sizeof(PSN), target);
		return err;
	}
	else
		return !noErr;  /* return something other than noErr */
} /* ESFindServer */


/***********************************************************/


static Boolean Check4Traps(void)
{
	Boolean result;
	long    r;
	
	result = (TrapAvailable(_WaitNextEvent) &&
				TrapAvailable(_GestaltDispatch) &&
				(Gestalt(gestaltAppleEventsAttr, &r) == noErr) &&
				((r & (1 << gestaltAppleEventsPresent)) != 0));
	
	return (result);
}


