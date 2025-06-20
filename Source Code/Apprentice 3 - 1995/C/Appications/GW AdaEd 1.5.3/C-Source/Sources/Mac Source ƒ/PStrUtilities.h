/* PStrUtilities.h
 *
 * Pascal copy and concat strings, borrowed from TBUtilities.cp,
 * part of TCL.  Renamed to avoid conflicts.
 *
 */


#pragma once


		/* Copy from => to */
#define	PCopyString(srcStr, destStr) PLstrcpy(destStr, srcStr)

		/* Concat here <= from */
//void	PConcatStrings(Str255 first, ConstStr255Param second);
#define	PConcatStrings(firstInOut, secondIn)	PLstrcat(firstInOut, secondIn)
