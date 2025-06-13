/* Frequency.h */
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

#ifndef Included_Frequency_h
#define Included_Frequency_h

/* Frequency module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */

/* number of notes.  this should be a whole number of octaves */
#define NUMNOTES (32 * 12)  /* should make CENTERNOTE a multiple of 12 */
#define CENTERNOTE (NUMNOTES / 2)  /* middle C -- should be a multiple of 12 */
#define MIDDLEC (261.625565300598635)
#define LOG2 (0.693147180559945309)

#endif
