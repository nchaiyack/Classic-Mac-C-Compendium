/* DrawCommand.c */
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

#include "DrawCommand.h"
#include "DataMunging.h"
#include "Memory.h"


#define BORDER (3)


/* draw a command with no parameters.  the string is null terminated */
OrdType						DrawCommandNoParams(WinType* Window, OrdType X, OrdType Y,
										FontType Font, FontSizeType FontSize, MyBoolean ActuallyDraw,
										MyBoolean GreyedOut, char* String)
	{
		OrdType					StrWidth;
		long						StrLength;

		ERROR(String == NIL,PRERR(ForceAbort,"DrawCommandNoParams:  NIL string"));
		StrLength = StrLen(String);
		StrWidth = LengthOfText(Font,FontSize,String,StrLength,ePlain);
		if (ActuallyDraw)
			{
				OrdType					FontHeight;

				FontHeight = GetFontHeight(Font,FontSize);
				DrawBoxFrame(Window,eBlack,X,Y,StrWidth + 2 * BORDER,FontHeight + 2 * BORDER);
				DrawBoxErase(Window,X + 1,Y + 1,StrWidth + 2 * BORDER - 1 - 1,
					FontHeight + 2 * BORDER - 1 - 1);
				DrawTextLine(Window,Font,FontSize,String,StrLength,X + BORDER,Y + BORDER,ePlain);
			}
		return StrWidth + 2 * BORDER;
	}


/* draw a command with one parameter.  first string is null terminated and not */
/* disposed, but the second string is a non-null-terminated heap block which is */
/* disposed.  if the second one is NIL, then it is ignored */
OrdType						DrawCommand1Param(WinType* Window, OrdType X, OrdType Y,
										FontType Font, FontSizeType FontSize, MyBoolean ActuallyDraw,
										MyBoolean GreyedOut, char* String, char* Argument1)
	{
		OrdType					TotalWidth;
		long						FirstLength;
		long						SecondLength;

		ERROR(String == NIL,PRERR(ForceAbort,"DrawCommand1Param:  NIL string"));
		FirstLength = StrLen(String);
		TotalWidth = LengthOfText(Font,FontSize,String,FirstLength,ePlain);
		if (Argument1 != NIL)
			{
				OrdType					OtherWidth;

				SecondLength = PtrSize(Argument1);
				OtherWidth = LengthOfText(Font,FontSize,Argument1,SecondLength,ePlain);
				if (OtherWidth > TotalWidth)
					{
						TotalWidth = OtherWidth;
					}
			}
		if (ActuallyDraw)
			{
				OrdType					FontHeight;

				FontHeight = GetFontHeight(Font,FontSize);
				DrawBoxFrame(Window,eBlack,X,Y,TotalWidth + 2 * BORDER,
					2 * FontHeight + 2 * BORDER);
				DrawBoxErase(Window,X + 1,Y + 1,TotalWidth + 2 * BORDER - 1 - 1,
					2 * FontHeight + 2 * BORDER - 1 - 1);
				DrawTextLine(Window,Font,FontSize,String,FirstLength,
					X + BORDER,Y + BORDER,ePlain);
				if (Argument1 != NIL)
					{
						DrawTextLine(Window,Font,FontSize,Argument1,SecondLength,X + BORDER,
							Y + BORDER + FontHeight,ePlain);
					}
			}
		if (Argument1 != NIL)
			{
				ReleasePtr(Argument1);
			}
		return TotalWidth + 2 * BORDER;
	}


/* draw a command with 2 parameters */
OrdType						DrawCommand2Params(WinType* Window, OrdType X, OrdType Y,
										FontType Font, FontSizeType FontSize, MyBoolean ActuallyDraw,
										MyBoolean GreyedOut, char* String, char* Argument1,
										char* Argument2)
	{
		OrdType					TotalWidth;
		long						FirstLength;
		long						SecondLength;
		long						ThirdLength;

		ERROR(String == NIL,PRERR(ForceAbort,"DrawCommand2Params:  NIL string"));
		FirstLength = StrLen(String);
		TotalWidth = LengthOfText(Font,FontSize,String,FirstLength,ePlain);
		if (Argument1 != NIL)
			{
				OrdType					OtherWidth;

				SecondLength = PtrSize(Argument1);
				OtherWidth = LengthOfText(Font,FontSize,Argument1,SecondLength,ePlain);
				if (OtherWidth > TotalWidth)
					{
						TotalWidth = OtherWidth;
					}
			}
		if (Argument2 != NIL)
			{
				OrdType					OtherWidth;

				ThirdLength = PtrSize(Argument2);
				OtherWidth = LengthOfText(Font,FontSize,Argument2,ThirdLength,ePlain);
				if (OtherWidth > TotalWidth)
					{
						TotalWidth = OtherWidth;
					}
			}
		if (ActuallyDraw)
			{
				OrdType					FontHeight;

				FontHeight = GetFontHeight(Font,FontSize);
				DrawBoxFrame(Window,eBlack,X,Y,TotalWidth + 2 * BORDER,
					3 * FontHeight + 2 * BORDER);
				DrawBoxErase(Window,X + 1,Y + 1,TotalWidth + 2 * BORDER - 1 - 1,
					3 * FontHeight + 2 * BORDER - 1 - 1);
				DrawTextLine(Window,Font,FontSize,String,FirstLength,X + BORDER,
					Y + BORDER,ePlain);
				if (Argument1 != NIL)
					{
						DrawTextLine(Window,Font,FontSize,Argument1,SecondLength,X + BORDER,
							Y + BORDER + FontHeight,ePlain);
					}
				if (Argument2 != NIL)
					{
						DrawTextLine(Window,Font,FontSize,Argument2,ThirdLength,X + BORDER,
							Y + BORDER + 2 * FontHeight,ePlain);
					}
			}
		if (Argument1 != NIL)
			{
				ReleasePtr(Argument1);
			}
		if (Argument2 != NIL)
			{
				ReleasePtr(Argument2);
			}
		return TotalWidth + 2 * BORDER;
	}


/* draw a command that has one parameter, but has line feeds in it. */
/* first string is null terminated and not disposed, but the second string is */
/* a non-null-terminated heap block which is disposed.  if the second one is NIL, */
/* then it is ignored */
OrdType						DrawCommand1ParamWithLineFeeds(WinType* Window, OrdType X, OrdType Y,
										FontType Font, FontSizeType FontSize, MyBoolean ActuallyDraw,
										MyBoolean GreyedOut, char* String, char* Argument1)
	{
		OrdType					TotalWidth;
		long						FirstLength;
		long						SecondLength;
		long						NumberOfLines;

		ERROR(String == NIL,PRERR(ForceAbort,"DrawCommand1ParamWithLineFeeds:  NIL string"));
		FirstLength = StrLen(String);
		TotalWidth = LengthOfText(Font,FontSize,String,FirstLength,ePlain);
		NumberOfLines = 1;
		if (Argument1 != NIL)
			{
				long						CharScan;

				SecondLength = PtrSize(Argument1);
				CharScan = 0;
				while (CharScan < SecondLength)
					{
						OrdType					OtherWidth;
						long						StartIndex;

						StartIndex = CharScan;
						while ((Argument1[CharScan] != '\x0d')
							&& (Argument1[CharScan] != '\x0a') && (CharScan < SecondLength))
							{
								CharScan += 1;
							}
						OtherWidth = LengthOfText(Font,FontSize,&(Argument1[StartIndex]),
							CharScan - StartIndex,ePlain);
						if (OtherWidth > TotalWidth)
							{
								TotalWidth = OtherWidth;
							}
						if (CharScan < SecondLength)
							{
								CharScan += 1;
							}
						NumberOfLines += 1;
					}
			}
		if (ActuallyDraw)
			{
				OrdType					FontHeight;

				FontHeight = GetFontHeight(Font,FontSize);
				DrawBoxFrame(Window,eBlack,X,Y,TotalWidth + 2 * BORDER,
					NumberOfLines * FontHeight + 2 * BORDER);
				DrawBoxErase(Window,X + 1,Y + 1,TotalWidth + 2 * BORDER - 1 - 1,
					NumberOfLines * FontHeight + 2 * BORDER - 1 - 1);
				DrawTextLine(Window,Font,FontSize,String,FirstLength,
					X + BORDER,Y + BORDER,ePlain);
				if (Argument1 != NIL)
					{
						long						CharScan;
						long						LineCount;

						CharScan = 0;
						LineCount = 1;
						while (CharScan < SecondLength)
							{
								long						StartIndex;

								StartIndex = CharScan;
								while ((Argument1[CharScan] != '\x0d')
									&& (Argument1[CharScan] != '\x0a') && (CharScan < SecondLength))
									{
										CharScan += 1;
									}
								DrawTextLine(Window,Font,FontSize,&(Argument1[StartIndex]),
									CharScan - StartIndex,X + BORDER,Y + BORDER + FontHeight * LineCount,
									ePlain);
								if (CharScan < SecondLength)
									{
										CharScan += 1;
									}
								LineCount += 1;
							}
					}
			}
		if (Argument1 != NIL)
			{
				ReleasePtr(Argument1);
			}
		return TotalWidth + 2 * BORDER;
	}
