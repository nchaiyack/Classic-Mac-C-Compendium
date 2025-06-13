/**********************************************************************\

File:		integrity.c

Purpose:	This module implements a quick-and-dirty integrity check:
			compare the resource fork and map length to stored values.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

#include "integrity.h"
#include "util.h"

#define RESOURCE_FORK_DATA_LENGTH_OFFSET		8L
#define RESOURCE_FORK_MAP_LENGTH_OFFSET			12L
#define STORED_DATA_LENGTH_OFFSET				128L
#define STORED_MAP_LENGTH_OFFSET				132L
#define TAG_OFFSET								136L
#define THE_TAG									0x16435934

Boolean DoIntegrityCheck(Boolean *programIntegritySet)
/* called first thing; can be used either to check program integrity (if it has
   been installed) or to install the integrity checker (if the SHIFT key is
   help down during program launch).  Returns FALSE if program integrity is
   not verified; programIntegritySet is TRUE if this procedure installs the
   integrity checker. */
{
	short			thisFile;
	long			count;
	long			resDataLength, checkData;
	long			resMapLength, checkMap;
	long			resMapOffset;
	short			resAttributes;
	long			tag;
	KeyMap			rawKeys;
	unsigned short	theKeys[8];
	
	*programIntegritySet=FALSE;
	GetKeys(rawKeys);
	Mymemcpy((Ptr)theKeys, (Ptr)rawKeys, sizeof(rawKeys));
	FlushVol(0L, 0);		/* just to be on the safe side */
	
	/* interestingly enough, the new-and-improved functions to open a resource fork
	   (HOpenRF & FSpOpenRF) won't work here.  They check permissions and won't
	   allow write permission to an already-open resource fork -- so we won't be
	   able to install the integrity checker on the fly if we Do The Right Thingª.
	   Luckily, the old-and-inferior function (OpenRF) doesn't ask permission... */
	OpenRF(CurApName, 0, &thisFile);
	
	SetFPos(thisFile, 1, RESOURCE_FORK_DATA_LENGTH_OFFSET);
	count=4L;
	/* get length of resource data */
	if (FSRead(thisFile, &count, (Ptr)(&resDataLength))!=noErr)
		return FALSE;
	
	SetFPos(thisFile, 1, RESOURCE_FORK_MAP_LENGTH_OFFSET);
	count=4L;
	/* get length of resource map */
	if (FSRead(thisFile, &count, (Ptr)(&resMapLength))!=noErr)
		return FALSE;
	
	SetFPos(thisFile, 1, TAG_OFFSET);
	count=4L;
	FSRead(thisFile, &count, (Ptr)(&tag));	/* read long-word tag */
	if (tag!=THE_TAG)	/* if not equal to our tag, integrity checker not installed */
	{
		if (theKeys[3]&1)	/* if SHIFT key down, install integrity checker */
		{
			count=4L;
			SetFPos(thisFile, 1, 4L);
			/* get the offset of the start of the resource map in the resource fork */
			if (FSRead(thisFile, &count, (Ptr)(&resMapOffset))!=noErr)
				return FALSE;
			
			resMapOffset+=22L;	/* resource fork attributes at map+22 */
			count=2L;
			SetFPos(thisFile, 1, resMapOffset);
			/* get old resource fork attributes */
			if (FSRead(thisFile, &count, (Ptr)(&resAttributes))!=noErr)
				return FALSE;
			
			resAttributes|=0x8000;	/* lock resource fork */
			count=2L;
			SetFPos(thisFile, 1, resMapOffset);
			/* rewrite new resource fork attributes */
			/* It is a little-known fact that this trick makes the application
			   immune to all known viruses -- no known virus bothers to unlock the
			   resource fork before attempting to infect, and ABSOLUTELY NO changes
			   can be made to the resource fork when it's locked like this.  On the
			   other hand, you better be damn sure that the application has no
			   viruses in it when you install the integrity checker -- if it's
			   infected when this code is run, you won't be able to disinfect it
			   until you unlock the resource fork. */
			if (FSWrite(thisFile, &count, (Ptr)(&resAttributes))!=noErr)
				return FALSE;
			
			SetFPos(thisFile, 1, STORED_DATA_LENGTH_OFFSET);
			count=4L;
			/* store length of resource data */
			if (FSWrite(thisFile, &count, (Ptr)(&resDataLength))!=noErr)
				return FALSE;
			
			SetFPos(thisFile, 1, STORED_MAP_LENGTH_OFFSET);
			count=4L;
			/* store length of resource map */
			if (FSWrite(thisFile, &count, (Ptr)(&resMapLength))!=noErr)
				return FALSE;
			
			SetFPos(thisFile, 1, TAG_OFFSET);
			count=4L;
			tag=THE_TAG;
			/* store tag so we know integrity checker is installed */
			if (FSWrite(thisFile, &count, (Ptr)(&tag))!=noErr)
				return FALSE;
			
			*programIntegritySet=TRUE;		/* so we know we've installed it */
		}
		
		return TRUE;
	}
	else
	{
		SetFPos(thisFile, 1, STORED_DATA_LENGTH_OFFSET);
		count=4L;
		/* get stored resource data length */
		if (FSRead(thisFile, &count, (Ptr)(&checkData))!=noErr)
			return FALSE;
		
		SetFPos(thisFile, 1, STORED_MAP_LENGTH_OFFSET);
		count=4L;
		/* get stored resource map length */
		if (FSRead(thisFile, &count, (Ptr)(&checkMap))!=noErr)
			return FALSE;
		
		/* check real resource data/map lengths to stored values */
		return ((resDataLength==checkData) && (resMapLength==checkMap));
	}
}
