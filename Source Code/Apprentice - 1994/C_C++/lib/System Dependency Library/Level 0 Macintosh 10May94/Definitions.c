/* Definitions.c */
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
#include "Definitions.h"
#include "Audit.h"
#include "Debug.h"

#pragma options(pack_enums)
#include <Memory.h>
#pragma options(!pack_enums)


/* move data.  The data can be overlapping. */
void				MoveData(char* Source, char* Destination, long NumBytes)
	{
#if DEBUG
		Zone*				Zone;
		char*				ZoneBeginning;
		char*				ZoneEnd;

		/* a handy error check:  make sure the source and destination are actually */
		/* in the bounds of the heap.  (debugging suppresses allocation of data */
		/* from temporary memory) */
		Zone = GetZone();
		ZoneBeginning = (char*)Zone;
		asm
			{
				move.l a5,ZoneEnd
			}
		if ((ZoneBeginning > (char*)Source)
			|| (ZoneEnd <= (char*)Source + NumBytes)
			|| (ZoneBeginning > (char*)Destination)
			|| (ZoneEnd <= (char*)Destination + NumBytes))
			{
				APRINT(("!MoveData %r->%r(+%l) [%r..%r]",Source,Destination,NumBytes,
					ZoneBeginning,ZoneEnd));
				PRERR(ForceAbort,"MoveData:  source or destination beyond heap limits!");
			}
#endif

		ERROR(NumBytes < 0,PRERR(ForceAbort,
			"MoveData:  Request to move negative number of bytes"));
		BlockMove(Source,Destination,NumBytes);
	}


/* copy data.  the regions can NOT be overlapping and an error will result */
/* if they are */
void				CopyData(char* Source, char* Destination, long NumBytes)
	{
#if DEBUG
		Zone*				Zone;
		char*				ZoneBeginning;
		char*				ZoneEnd;

		Zone = GetZone();
		ZoneBeginning = (char*)Zone;
		asm {move.l a5,ZoneEnd}
		if ((ZoneBeginning > (char*)Source)
			|| (ZoneEnd <= (char*)Source + NumBytes)
			|| (ZoneBeginning > (char*)Destination)
			|| (ZoneEnd <= (char*)Destination + NumBytes))
			{
				APRINT(("!CopyData %r->%r(+%l) [%r..%r]",Source,Destination,NumBytes,
					ZoneBeginning,ZoneEnd));
				PRERR(ForceAbort,"CopyData:  source or destination beyond heap limits!");
			}
#endif

		ERROR(NumBytes < 0,PRERR(ForceAbort,
			"CopyData:  Request to move negative number of bytes"));
		ERROR(((Source <= Destination) && (Source + NumBytes > Destination))
			|| ((Destination <= Source) && (Destination + NumBytes > Source)),
			PRERR(ForceAbort,"CopyData:  Request to copy overlapping blocks"));
		BlockMove(Source,Destination,NumBytes);
	}
