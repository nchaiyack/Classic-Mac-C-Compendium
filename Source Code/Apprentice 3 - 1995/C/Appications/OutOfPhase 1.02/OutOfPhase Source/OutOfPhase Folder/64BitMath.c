/* 64BitMath.c */
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

#include "64BitMath.h"


#ifdef ENABLEGENERICLONGLONG
/* add two longlongs:  *L = *L + *R */
void							LongLongAdd(LongLongRec* L, LongLongRec* R)
	{
		double					Overflow;
		double					LowTemp;

		ERROR(sizeof(long) == 8,PRERR(AllowResume,
			"LongLongAdd:  64-bit longs are available on this system"));

		/* low order stuff */
		LowTemp = (double)((L->Low32Bits + R->Low32Bits) & 0xffffffff);

		/* check for overflow */
		Overflow = 0;
		if (LowTemp < (double)L->Low32Bits + (double)R->Low32Bits)
			{
				Overflow = 1;
			}

		/* high order stuff */
		L->Low32Bits = LowTemp;
		L->High32Bits = L->High32Bits + R->High32Bits + Overflow;
	}
#endif
