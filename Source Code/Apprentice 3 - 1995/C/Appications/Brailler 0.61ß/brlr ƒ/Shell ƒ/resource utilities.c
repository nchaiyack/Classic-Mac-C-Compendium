#include "resource utilities.h"
#include "util.h"
#include <Icons.h>

OSErr OpenTheResFile(FSSpec* fs, short* oldRefNum, short* newRefNum, Boolean* alreadyOpen,
	Boolean readOnly)
{
	Handle			oldTopMapHndl;
	OSErr			theResError;
	
	*oldRefNum=CurResFile();
	oldTopMapHndl=LMGetTopMapHndl();
	*newRefNum=HOpenResFile(fs->vRefNum, fs->parID, fs->name, readOnly ? fsRdPerm : fsRdWrPerm);
	theResError=ResError();
	*alreadyOpen=(oldTopMapHndl==LMGetTopMapHndl());
	if (theResError==noErr)
		UseResFile(*newRefNum);
	return theResError;
}

void CloseTheResFile(short oldRefNum, short newRefNum, Boolean alreadyOpen)
{
	if (!alreadyOpen)
	{
		CloseResFile(newRefNum);
	}
	UseResFile(oldRefNum);
}

Handle SafeReleaseResource(Handle resHandle)
{
	if (resHandle!=0L)
		ReleaseResource(resHandle);
	
	return 0L;
}

Handle SafeDisposeIconSuite(Handle iconHandle)
{
	if (iconHandle!=0L)
		DisposeIconSuite(iconHandle, TRUE);
	
	return 0L;
}

MenuHandle SafeDisposeMenu(MenuHandle theMenu)
{
	SignedByte		flags;
	
	if (theMenu!=0L)
	{
		flags=HGetState((Handle)theMenu);
		if (flags&0x20)
			ReleaseResource((Handle)theMenu);
		else
			DisposeMenu(theMenu);
	}
	
	return 0L;
}

OSErr AddIndString(StringPtr theStr, short resID)
/* adds new indexed string to end of STR# resource */
{
	Handle			resHandle;
	OSErr			oe;
	long			oldResSize;
	
	resHandle=Get1Resource('STR#', resID);
	if ((oe=ResError())!=noErr)
		return oe;
	
	oldResSize=GetHandleSize(resHandle);
	SetHandleSize(resHandle, oldResSize+theStr[0]+1);
	if ((oe=MemError())!=noErr)
	{
		ReleaseResource(resHandle);
		return oe;
	}
	
	HLock(resHandle);
	Mymemcpy((Ptr)((unsigned long)*resHandle+oldResSize), (Ptr)theStr, theStr[0]+1);
	(**((short**)resHandle))++;
	
	ChangedResource(resHandle);
	WriteResource(resHandle);
	HPurge(resHandle);
	ReleaseResource(resHandle);

   return ResError();
}

OSErr DeleteIndString(short resID, short index)
{
	Handle			resHandle;
	OSErr			oe;
	unsigned long	oldResSize;
	unsigned long	offset;
	unsigned char	*stringPtr;
	short			i;
	unsigned short	stringLength;
	
	resHandle=Get1Resource('STR#', resID);
	if ((oe=ResError())!=noErr)
		return oe;
	
	HLock(resHandle);
	HNoPurge(resHandle);
	
	(**((short**)resHandle))--;
	
	oldResSize=GetHandleSize(resHandle);
	offset=sizeof(short);
	stringPtr=(unsigned char*)*resHandle+offset;
	for (i=1; i<index; i++)
	{
		offset+=1+stringPtr[0];
		stringPtr+=1+stringPtr[0];
	}
	
	stringLength=stringPtr[0]+1;
	Mymemcpy((Ptr)stringPtr, (Ptr)((unsigned long)stringPtr+stringPtr[0]+1), oldResSize-offset);
	SetHandleSize(resHandle, oldResSize-stringLength);
	
	ChangedResource(resHandle);
	WriteResource(resHandle);
	HPurge(resHandle);
	ReleaseResource(resHandle);
	
	return ResError();
}

// the following is based on a snippet from Apple
OSErr SetIndString(StringPtr theStr,short resID,short strIndex)
{
   Handle theRes;               /* handle pointing to STR# resource */
   unsigned short numStrings;            /* number of strings in STR# */
   unsigned short ourString;             /* counter to index up to strIndex */
   unsigned char *resStr;                /* string pointer to STR# string to replace */
   unsigned long oldSize;                /* size of STR# resource before call */
   unsigned long newSize;                /* size of STR# resource after call */
   unsigned long offset;        /* resource offset to str to replace*/

   /* make sure index is in bounds */

   if (resID < 1)
   return -1;

   /* make sure resource exists */

   theRes = Get1Resource('STR#',resID);
   if (ResError()!=noErr)
   return ResError();
   if (!theRes || !(*theRes))
   return resNotFound;

   HLock(theRes);
   HNoPurge(theRes);

   /* get # of strings in STR# */

   BlockMove(*theRes,&numStrings,sizeof(short));
   if (strIndex > numStrings)
   return resNotFound;

   /* get a pointer to the string to replace */

   offset = sizeof(short);
   resStr = (unsigned char *) *theRes + sizeof(short);
   for (ourString=1; ourString<strIndex; ourString++) {
   offset += 1+resStr[0];
   resStr += 1+resStr[0];
   }

   /* grow/shrink resource handle to make room for new string */

   oldSize = GetHandleSize(theRes);
   newSize = oldSize - resStr[0] + theStr[0];
   HUnlock(theRes);
   SetHandleSize(theRes,newSize);
   if (MemError()!=noErr)
   {
  	 ReleaseResource(theRes);
  	 return MemError();
   }
   HLock(theRes);
   resStr = (unsigned char*)((unsigned long)*theRes + offset);

   /* move old data forward/backward to make room */

   BlockMove(resStr+resStr[0]+1, resStr+theStr[0]+1, oldSize-offset-resStr[0]-1);

   /* move new data in */

   BlockMove(theStr,resStr,theStr[0]+1);

   /* write resource out */

   ChangedResource(theRes);
   WriteResource(theRes);
   HPurge(theRes);
   ReleaseResource(theRes);

   return ResError();
}
