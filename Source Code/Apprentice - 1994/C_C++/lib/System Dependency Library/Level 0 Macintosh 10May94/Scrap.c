/* Scrap.c */
/*****************************************************************************/
/*                                                                           */
/*    System Dependency Library for Building Portable Software               */
/*    Macintosh Version                                                      */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This file is Public Domain; it may be used for any purpose whatsoever  */
/*    without restriction.                                                   */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Debug.h"
#include "Audit.h"
#include "Definitions.h"

#pragma options(pack_enums)
#include <Scrap.h>
#include <Memory.h>
#include <Errors.h>
#pragma options(!pack_enums)

#include "Scrap.h"
#include "Memory.h"


#define ScrapFlushThreshHold (4096L) /* max size that's kept in memory */


EXECUTE(static MyBoolean					ScrapInitialized = False;)


/* initialize scrap handler by zeroing the scrap */
MyBoolean		Eep_InitializeScrapHandler(void)
	{
		ERROR(ScrapInitialized,PRERR(ForceAbort,
			"InitializeScrapHandler called multiple times"));
		EXECUTE(ScrapInitialized = True;)
		if (InfoScrap()->scrapState < 0)
			{
				ZeroScrap(); /* initialize (probably not needed under multifinder) */
			}
		return True;
	}


/* shut down the scrap handler.  this doesn't do anything right now */
void				Eep_ShutdownScrapHandler(void)
	{
		ERROR(!ScrapInitialized,PRERR(ForceAbort,"Scrap handler not initialized"));
	}


/* get a block containing a copy of the scrap */
/* if a block couldn't be allocated, then it returns NIL */
char*				GetCopyOfScrap(void)
	{
		char*				TheData;
		long				Offset;
		char**			TheHandle;
		long				ScrapLength;

		ERROR(!ScrapInitialized,PRERR(ForceAbort,"Scrap handler not initialized"));

		/* find out if the scrap has a TEXT on it */
		ScrapLength = GetScrap(NIL,'TEXT',&Offset); /* we can only handle text */
		if (ScrapLength < 0)
			{
				return NIL; /* no scrap */
			}

		/* allocate a place to put the scrap */
		TheHandle = NewHandle(ScrapLength);
		if (TheHandle == NIL)
			{
				OSErr			Error;

				TheHandle = TempNewHandle(ScrapLength,&Error);
			}
		if (TheHandle == NIL)
			{
				return NIL;
			}

		/* save the scrap in a local heap block for them to use */
		GetScrap(TheHandle,'TEXT',&Offset);
		TheData = AllocPtrCanFail(GetHandleSize(TheHandle),"Scrap");
		if (TheData != NIL)
			{
				CopyData(*TheHandle,TheData,GetHandleSize(TheHandle));
			}
		DisposeHandle(TheHandle);

		return TheData;
	}


/* make a copy of the block and put the data into the scrap */
/* returns True if successful */
MyBoolean		SetScrapToThis(char* DataToCopy)
	{
		long			ScrapSize;
		EXECUTE(long ErrorCode;)

		ERROR(!ScrapInitialized,PRERR(ForceAbort,"Scrap handler not initialized"));
		CheckPtrExistence(DataToCopy);
		ScrapSize = PtrSize(DataToCopy);
		ZeroScrap();
		EXECUTE(ErrorCode = ) PutScrap(ScrapSize,'TEXT',&(DataToCopy[0]));
		ERROR(ErrorCode != noErr,PRERR(AllowResume,"SetScrapToThis:  non-zero error code"));
		if (ScrapSize >= ScrapFlushThreshHold)
			{
				UnloadScrap();
			}
		/* how can we tell if it failed? */
		return True;
	}
