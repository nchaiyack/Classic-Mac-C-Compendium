/* FixedPoint.h */
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

#ifndef Included_FixedPoint_h
#define Included_FixedPoint_h

/* FixedPoint module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */

#define smallfixed_precision (12)
typedef unsigned short smallfixedunsigned;
typedef signed short smallfixedsigned;
typedef signed long smallfixedextended;
/* if ANSI hadn't been so stingy, then we might have had proper arithmetic (signed) */
/* right shifting, but NOOOOO, that might be to INEFFICIENT if they required that... */
#define smallfixed2int(x) (x / (/*NB long unnecessary*/1 << smallfixed_precision))
#define int2smallfixed(x) (x * (1 << smallfixed_precision))
#define smallfixed2double(x) (((double)x) / (1 << smallfixed_precision))
#define double2smallfixed(x) ((smallfixedsigned)((x) * (1 << smallfixed_precision)\
					+ ((x >= 0) ? 0.5 : -0.5)))

#define smallfixedmult(x,y) ((smallfixed)(((x) * (y)) + \
					(1 << (smallfixed_precision - 1)) / (1 << smallfixed_precision)))


#define largefixed_precision (24)
typedef unsigned long largefixedunsigned;
typedef signed long largefixedsigned;
#define largefixed2int(x) (x / (1L << largefixed_precision))
#define int2largefixed(x) (x * (1L << largefixed_precision))
#define largefixed2double(x) (((double)x) / (1L << largefixed_precision))
#define largefixed2single(x) (((float)x) / (1L << largefixed_precision))
#define double2largefixed(x) ((largefixedsigned)((x) * (1L << largefixed_precision)\
					+ ((x >= 0) ? 0.5 : -0.5)))

#define largefixedmult(x,y) (double2largefixed(largefixed2double(x)\
					* largefixed2double(y)))


#define smallfixed2largefixed(x) ((largefixedsigned)x\
					* (1L << (largefixed_precision - smallfixed_precision)))
#define largefixed2smallfixed(x) ((smallfixed)(x\
					/ (1L << (largefixed_precision - smallfixed_precision))))


#define roundtonearest(x) ((long)((x >= 0) ? (x + 0.5) : (x - 0.5)))

#endif
