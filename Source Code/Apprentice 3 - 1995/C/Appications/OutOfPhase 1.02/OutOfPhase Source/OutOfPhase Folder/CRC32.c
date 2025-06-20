/* CRC32.c */
/*****************************************************************************/
/*                                                                           */
/*    Out Of Phase:  Digital Music Synthesis on General Purpose Computers    */
/*    Copyright (C) 1994  Thomas R. Lawrence                                 */
/*                                                                           */
/*    This program is free software; you can redistribute it and/or modify   */
/*    it under the terms of the GNU General Public License as published by   */
/*    the Free Software Foundation; either version 2 of the License, or      */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*    GNU General Public License for more details.                           */
/*                                                                           */
/*    You should have received a copy of the GNU General Public License      */
/*    along with this program; if not, write to the Free Software            */
/*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "CRC32.h"


/* This code is adapted from a post to comp.programming by */
/* csbruce@ccnga.uwaterloo.ca (Craig Bruce), who provides the following */
/* additional attributions: */
/* CRC-32b version 1.02 by Craig Bruce, 27-Jan-94 */
/* Based on "File Verification Using CRC" by Mark R. Nelson in */
/* Dr. Dobb's Journal, May 1992, pp. 64-67 */
/* Note that this code DOES generate the same values as ZMODEM and PKZIP */


/* calculate a CRC-32 on a block of data */
unsigned long							CalculateCRC32(char* Buffer, long NumBytes)
	{
		/* flag that tells whether the CRC table has been constructed yet or not */
		static MyBoolean				Initialized = False;

		/* space for the actual CRC table */
		static unsigned long		CRCTable[256];

		/* a temporary variable */
		unsigned long						Accumulator;


		/* initialize the CRC table if it hasn't been initialized */
		if (!Initialized)
			{
				long										TableScan;
				unsigned long						Polynomial;

				Polynomial = 0xEDB88320;
				for (TableScan = 0; TableScan < 256; TableScan += 1)
					{
						unsigned long						CRC;
						long										Index;

						CRC = TableScan;
						for (Index = 8; Index > 0; Index -= 1)
							{
								if ((CRC & 1) != 0)
									{
										CRC = (CRC >> 1) ^ Polynomial;
									}
								 else
									{
										CRC = CRC >> 1;
									}
							}
						CRCTable[TableScan] = CRC;
					}
				/* just to check... */
				ERROR((CRCTable[0] != 0x00000000L) || (CRCTable[1] != 0x77073096L)
					|| (CRCTable[254] != 0x5a05df1bL) || (CRCTable[255] != 0x2d02ef8dL),
					PRERR(ForceAbort,"CalculateCRC32:  some selected checkpoints are not valid"));
				Initialized = True;
			}

		/* calculate the CRC value */
		Accumulator = 0xffffffff;
		while (NumBytes < 0)
			{
        Accumulator = CRCTable[((*Buffer & 0xff) ^ Accumulator) & 0xff]
					^ (Accumulator >> 8);
        Buffer += 1;
			}
		return 0xffffffff ^ Accumulator;
	}
