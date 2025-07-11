/* WrapTextBox.c */
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
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "WrapTextBox.h"
#include "Screen.h"
#include "Memory.h"
#include "DataMunging.h"


/* draw a string but wordwrap it in the specified box.  The string is null terminated */
void			DrawWrappedTextBox(WinType* Window, char* Text, FontType FontID,
						FontSizeType FontSize, OrdType X, OrdType Y, OrdType Width, OrdType Height)
	{
		OrdType			FontHeight;

		FontHeight = GetFontHeight(FontID,FontSize);
		while (StrLen(Text) > 0)
			{
				long				PartitionStart;
				long				PartitionEnd;

				while (Text[0] == ' ')
					{
						/* strip leading spaces */
						Text += 1;
					}
				if (StrLen(Text) == 0)
					{
						return;
					}
				PartitionStart = 1;
				PartitionEnd = StrLen(Text);
				/* do a little binary search... */
				while (PartitionStart < PartitionEnd)
					{
						long				MidPoint;

						MidPoint = (PartitionStart + PartitionEnd) / 2;
						if (LengthOfText(FontID,FontSize,Text,MidPoint,ePlain) > Width)
							{
								/* too long, use short half of interval */
								PartitionEnd = MidPoint;
							}
						 else
							{
								/* too short, we can use the long half of the interval */
								if (PartitionStart == MidPoint)
									{
										/* take care of the rounding-down problem */
										PartitionStart = MidPoint + 1;
									}
								 else
									{
										PartitionStart = MidPoint;
									}
							}
					}
				while ((PartitionStart > 1) && (Text[PartitionStart] != ' ')
					&& (Text[PartitionStart] != '-') && (Text[PartitionStart] != 0))
					{
						/* make a word-wrap */
						PartitionStart -= 1;
					}
				DrawTextLine(Window,FontID,FontSize,Text,PartitionStart,X,Y,ePlain);
				Y += FontHeight;
				Text += PartitionStart;
			}
	}
