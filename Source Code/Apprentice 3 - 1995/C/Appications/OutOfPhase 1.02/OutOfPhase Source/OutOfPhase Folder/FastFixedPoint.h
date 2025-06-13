/* FastFixedPoint.h */
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

#ifndef Included_FastFixedPoint_h
#define Included_FastFixedPoint_h

/* FastFixedPoint module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */

#define FASTFIXEDPRECISION (15)

typedef signed long FastFixedType;

#define FASTFIXEDFRACTMASK ((((FastFixedType)1L) << FASTFIXEDPRECISION) - 1)

#define FastFixedTimes16BitTo16Bit(fastfixed,sixteenbit) (((fastfixed)\
					* (sixteenbit)) >> (FASTFIXEDPRECISION))

#define FastFixedTimes8BitTo16Bit(fastfixed,eightbit) (((fastfixed)\
					* (eightbit)) >> (-(16 - (FASTFIXEDPRECISION + 8))))

#define FastFixedTimes16BitTo24Bit(fastfixed,sixteenbit) (((fastfixed)\
					* (sixteenbit)) >> (-(24 - (FASTFIXEDPRECISION + 16))))

#define FastFixedTimes8BitTo24Bit(fastfixed,eightbit) (((fastfixed) * (eightbit))\
					<< (24 - (FASTFIXEDPRECISION + 8)))

#define FastFixedTimes8BitToFastFixed(fastfixed,eightbit) (((fastfixed)\
					* (eightbit)) >> 8)

#define FastFixedTimes16BitToFastFixed(fastfixed,sixteenbit) (((fastfixed)\
					* (sixteenbit)) >> 16)

#define FastFixedTimesFastFixedToFastFixed(left,right) (((left)\
					* (right)) >> FASTFIXEDPRECISION)

#define Double2FastFixed(x) ((signed long)((x) * (1L << FASTFIXEDPRECISION)\
					+ ((x >= 0) ? 0.5 : -0.5)))

#define FastFixed2Double(x) (((double)(x)) / (1L << FASTFIXEDPRECISION))

#define FastFixed2Float(x) (((float)(x)) / (1L << FASTFIXEDPRECISION))

#define Int2FastFixed(x) ((signed long)(x) << FASTFIXEDPRECISION)

#define FastFixed2Int(x) ((signed long)(x) >> FASTFIXEDPRECISION)

#endif
