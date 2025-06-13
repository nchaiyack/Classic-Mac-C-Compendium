/* TextView.c */
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

#include "TextView.h"
#include "Memory.h"
#include "EventLoop.h"
#include "TextStorage.h"


/* width of the edit area in pixels */
#define HARDCODEDMAXWIDTH (5000)


struct TextViewRec
	{
		TextStorageRec*			Storage;
		WinType*						Window;
		OrdType							X;
		OrdType							Y;
		OrdType							Width;
		OrdType							Height;
		FontType						FontID;
		FontSizeType				FontSize;
		OrdType							LineHeight;
		long								CurrentTopLine;
		OrdType							PixelIndent;
		long								SpacesPerTab;
		long								SelectStartLine;
		long								SelectStartChar;
		long								SelectEndLine;
		long								SelectEndCharPlusOne;
		MyBoolean						CursorEnabledFlag;
		MyBoolean						CursorDrawnFlag;
		double							LastCursorFlashTime;
	};


/* create a new text view object */
TextViewRec*			NewTextView(WinType* Window, OrdType X, OrdType Y,
										OrdType Width, OrdType Height, long SpacesPerTab,
										FontType Font, FontSizeType PointSize)
	{
		TextViewRec*		View;

		View = (TextViewRec*)AllocPtrCanFail(sizeof(TextViewRec),"TextViewRec");
		if (View == NIL)
			{
			 MemOut1:
				return NIL;
			}
		View->Storage = NewTextStorage();
		if (View->Storage == NIL)
			{
			 MemOut2:
				ReleasePtr((char*)View);
				goto MemOut1;
			}
		View->Window = Window;
		View->X = X;
		View->Y = Y;
		View->Width = Width;
		View->Height = Height;
		View->FontID = Font;
		View->FontSize = PointSize;
		View->LineHeight = GetFontHeight(Font,PointSize);
		View->CurrentTopLine = 0;
		View->PixelIndent = 0;
		View->SpacesPerTab = SpacesPerTab;
		View->SelectStartLine = 0;
		View->SelectStartChar = 0;
		View->SelectEndLine = 0;
		View->SelectEndCharPlusOne = 0;
		View->CursorEnabledFlag = False;
		View->CursorDrawnFlag = True;
		View->LastCursorFlashTime = ReadTimer();
		TextViewFullRedraw(View);
		return View;
	}


/* dispose the text view object and all data it contains */
void							DisposeTextView(TextViewRec* View)
	{
		DisposeTextStorage(View->Storage);
		ReleasePtr((char*)View);
	}


/* find out where the text view object is located on the screen */
OrdType						GetTextViewXLoc(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return View->X;
	}


/* find out where the text view object is located on the screen */
OrdType						GetTextViewYLoc(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return View->Y;
	}


/* find out where the text view object is located on the screen */
OrdType						GetTextViewWidth(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return View->Width;
	}


/* find out where the text view object is located on the screen */
OrdType						GetTextViewHeight(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return View->Height;
	}


/* find out what font the text is being displayed in */
FontType					GetTextViewFont(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return View->FontID;
	}


/* find out what font the text is being displayed in */
FontSizeType			GetTextViewPointSize(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return View->FontSize;
	}


/* find out how many spaces are displayed for each tab */
long							GetTextViewSpacesPerTab(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return View->SpacesPerTab;
	}


/* find out what the index of the top line on the display is */
long							GetTextViewTopLine(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return View->CurrentTopLine;
	}


/* find out what the pixel position of the leftmost character displayed is */
OrdType						GetTextViewPixelIndent(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return View->PixelIndent;
	}


/* returns True if the selection contains more than 0 characters */
MyBoolean					TextViewIsThereValidSelection(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return (View->SelectStartLine != View->SelectEndLine)
			|| (View->SelectStartChar != View->SelectEndCharPlusOne);
	}


/* get the index of the starting line of the selection */
long							GetTextViewSelectStartLine(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return View->SelectStartLine;
	}


/* get the index of the last line of the selection */
long							GetTextViewSelectEndLine(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return View->SelectEndLine;
	}


/* get the index of the starting character of the selection */
long							GetTextViewSelectStartChar(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return View->SelectStartChar;
	}


/* get the index of the character after the end of the selection.  if the starting */
/* line == ending line, and starting char == last char + 1, then there is no */
/* selection, but instead an insertion point */
long							GetTextViewSelectEndCharPlusOne(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return View->SelectEndCharPlusOne;
	}


/* find out if display of inverted selection text is enabled */
MyBoolean					TextViewIsShowSelectionEnabled(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return View->CursorEnabledFlag;
	}


/* find out if any changes have been made to the text view object since */
/* the last call to TextViewHasBeenSaved */
MyBoolean					TextViewDoesItNeedToBeSaved(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return TextStorageHasDataChanged(View->Storage);
	}


/* see if the specified position is within the text view box */
MyBoolean					TextViewHitTest(TextViewRec* View, OrdType X, OrdType Y)
	{
		CheckPtrExistence(View);
		return (X >= View->X) && (Y >= View->Y) && (X < View->X + View->Width)
			&& (Y < View->Y + View->Height);
	}


/* find out the total number of lines in the text view box */
long							GetTextViewNumLines(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return TextStorageGetLineCount(View->Storage);
	}


/* find out how many characters the specified line contains */
long							GetTextViewLineLength(TextViewRec* View, long LineIndex)
	{
		CheckPtrExistence(View);
		return TextStorageGetLineLength(View->Storage,LineIndex);
	}


/* get the height of a line of text (font height) */
OrdType						GetTextViewLineHeight(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return View->LineHeight;
	}


/* change where in the window the text view box is located */
void							SetTextViewPosition(TextViewRec* View, OrdType X, OrdType Y,
										OrdType Width, OrdType Height)
	{
		CheckPtrExistence(View);
		View->X = X;
		View->Y = Y;
		View->Width = Width;
		View->Height = Height;
		TextViewFullRedraw(View);
	}


/* change the font being used to display the text */
void							SetTextViewFontStuff(TextViewRec* View, FontType Font,
										FontSizeType Size)
	{
		CheckPtrExistence(View);
		View->FontID = Font;
		View->FontSize = Size;
		View->LineHeight = GetFontHeight(Font,Size);
		TextViewFullRedraw(View);
	}


/* change the number of spaces displayed for each tab */
void							SetTextViewTabSize(TextViewRec* View, long SpacesPerTab)
	{
		CheckPtrExistence(View);
		View->SpacesPerTab = SpacesPerTab;
		TextViewFullRedraw(View);
	}


/* set the index of the top line being displayed in the text box */
void							SetTextViewTopLine(TextViewRec* View, long NewTopLine)
	{
		long				NumLinesToShiftBy;

		CheckPtrExistence(View);
		if (NewTopLine > GetTextViewNumLines(View) - TextViewNumVisibleLines(View) + 1)
			{
				NewTopLine = GetTextViewNumLines(View) - TextViewNumVisibleLines(View) + 1;
			}
		if (NewTopLine < 0)
			{
				NewTopLine = 0;
			}
		NumLinesToShiftBy = View->CurrentTopLine - NewTopLine;
		View->CurrentTopLine = NewTopLine;
		/* positive == scroll down; negative == scroll up */
		if ((NumLinesToShiftBy > TextViewNumVisibleLines(View))
			|| (- NumLinesToShiftBy > TextViewNumVisibleLines(View)))
			{
				TextViewFullRedraw(View);
			}
		 else
			{
				SetClipRect(View->Window,View->X,View->Y,View->Width,View->Height);
				ScrollArea(View->Window,View->X,View->Y,View->Width,
					View->Height,0,NumLinesToShiftBy * View->LineHeight);
				if (NumLinesToShiftBy > 0)
					{
						/* scrolled down, opening new lines at top */
						TextViewRedrawRange(View,NewTopLine,NewTopLine + NumLinesToShiftBy);
					}
				 else
					{
						/* scrolled up, opening new lines at bottom */
						TextViewRedrawRange(View,NewTopLine + (TextViewNumVisibleLines(View) - 1)
							+ NumLinesToShiftBy/*which is negative*/,
							NewTopLine + (TextViewNumVisibleLines(View) - 1));
					}
			}
	}


/* set the index of the leftmost character in the text edit box */
void							SetTextViewPixelIndent(TextViewRec* View, OrdType NewPixelIndent)
	{
		long				OldPixelIndent;
		long				VirtualWindowWidth;

		/* this routine could be improved; but don't forget to SetClipRect! */
		CheckPtrExistence(View);
		OldPixelIndent = View->PixelIndent;
		VirtualWindowWidth = TextViewGetVirtualWindowWidth(View);
		if (NewPixelIndent > VirtualWindowWidth - View->Width)
			{
				NewPixelIndent = VirtualWindowWidth - View->Width;
			}
		if (NewPixelIndent < 0)
			{
				NewPixelIndent = 0;
			}
		if (OldPixelIndent != NewPixelIndent)
			{
				View->PixelIndent = NewPixelIndent;
				TextViewFullRedraw(View);
			}
	}


/* set the selection to the specified area */
void							SetTextViewSelection(TextViewRec* View, long StartLine,
										long StartChar, long EndLine, long EndCharPlusOne)
	{
		long				OldSelectStartLine;
		long				OldSelectEndLine;

		CheckPtrExistence(View);
		ERROR((StartLine > EndLine) || ((StartLine == EndLine)
			&& (StartChar > EndCharPlusOne)),
			PRERR(AllowResume,"SetTextViewSelection:  Start line after end line"));
		if (StartLine > GetTextViewNumLines(View) - 1)
			{
				StartLine = GetTextViewNumLines(View) - 1;
				StartChar = GetTextViewLineLength(View,StartLine);
			}
		if (EndLine > GetTextViewNumLines(View) - 1)
			{
				EndLine = GetTextViewNumLines(View) - 1;
				EndCharPlusOne = GetTextViewLineLength(View,EndLine);
			}
		if (StartChar > GetTextViewLineLength(View,StartLine))
			{
				StartChar = GetTextViewLineLength(View,StartLine);
			}
		if (EndCharPlusOne > GetTextViewLineLength(View,EndLine))
			{
				EndCharPlusOne = GetTextViewLineLength(View,EndLine);
			}
		if ((StartLine == EndLine) && (StartChar == EndCharPlusOne))
			{
				/* make an insertion point always show up right away in it's new location */
				View->CursorDrawnFlag = True;
				View->LastCursorFlashTime = ReadTimer();
			}
		OldSelectStartLine = View->SelectStartLine;
		OldSelectEndLine = View->SelectEndLine;
		View->SelectStartLine = StartLine;
		View->SelectEndLine = EndLine;
		View->SelectStartChar = StartChar;
		View->SelectEndCharPlusOne = EndCharPlusOne;
		if ((StartLine > OldSelectEndLine) || (EndLine < OldSelectStartLine))
			{
				/* old and new selection ranges are disjoint */
				TextViewRedrawRange(View,OldSelectStartLine,OldSelectEndLine);
				TextViewRedrawRange(View,StartLine,EndLine);
			}
		else if ((StartLine >= OldSelectStartLine) && (StartLine <= OldSelectEndLine))
			{
				long				Begin;
				long				End;

				/* |  old selection   |.......|  */
				/*    |   new selection   | */
				/* we want to draw the line that overlaps due to partial selection */
				TextViewRedrawRange(View,OldSelectStartLine,StartLine);
				if (OldSelectEndLine < EndLine)
					{
						Begin = OldSelectEndLine;
						End = EndLine;
					}
				 else
					{
						Begin = EndLine;
						End = OldSelectEndLine;
					}
				TextViewRedrawRange(View,Begin,End);
			}
		else if ((OldSelectStartLine >= StartLine) && (OldSelectStartLine <= EndLine))
			{
				long				Begin;
				long				End;

				/*    |   old selection  | */
				/* |  new selection   |.....| */
				TextViewRedrawRange(View,StartLine,OldSelectStartLine);
				if (OldSelectEndLine < EndLine)
					{
						Begin = OldSelectEndLine;
						End = EndLine;
					}
				 else
					{
						Begin = EndLine;
						End = OldSelectEndLine;
					}
				TextViewRedrawRange(View,Begin,End);
			}
		else
			{
				long				Minimum;
				long				Maximum;

				/* we don't know what the hell is going on */
				if (OldSelectStartLine < StartLine)
					{
						Minimum = OldSelectStartLine;
					}
				 else
					{
						Minimum = StartLine;
					}
				if (OldSelectEndLine > EndLine)
					{
						Maximum = OldSelectEndLine;
					}
				 else
					{
						Maximum = EndLine;
					}
				TextViewRedrawRange(View,Minimum,Maximum);
			}
	}


/* set the selection to an insertion point at the specified position */
void							SetTextViewInsertionPoint(TextViewRec* View, long Line, long Char)
	{
		CheckPtrExistence(View);
		SetTextViewSelection(View,Line,Char,Line,Char);
	}


/* enable the display of inverted selected text */
void							EnableTextViewSelection(TextViewRec* View)
	{
		CheckPtrExistence(View);
		View->CursorEnabledFlag = True;
		TextViewRedrawRange(View,View->SelectStartLine,View->SelectEndLine);
	}


/* disable the display of inverted selected text */
void							DisableTextViewSelection(TextViewRec* View)
	{
		CheckPtrExistence(View);
		View->CursorEnabledFlag = False;
		TextViewRedrawRange(View,View->SelectStartLine,View->SelectEndLine);
	}


/* indicate that any changes to the text have been recognized */
void							TextViewHasBeenSaved(TextViewRec* View)
	{
		CheckPtrExistence(View);
		TextStorageDataIsUpToDate(View->Storage);
	}


/* break the specified line at the specified character index.  this is used */
/* for when they hit carriage return in the middle of a line */
MyBoolean					TextViewBreakLine(TextViewRec* View, long LineIndex, long CharIndex)
	{
		MyBoolean				ReturnValue;

		CheckPtrExistence(View);
		ReturnValue = TextStorageBreakLine(View->Storage,LineIndex,CharIndex);
		TextViewRedrawRange(View,LineIndex,TextStorageGetLineCount(View->Storage)
			+ TextViewNumVisibleLines(View));
		return ReturnValue;
	}


/* concatenate the line after the specified line onto the end of the */
/* specified line.  Used for hitting delete at the beginning of a line */
MyBoolean					TextViewFoldLines(TextViewRec* View, long LineIndex)
	{
		MyBoolean				ReturnValue;

		CheckPtrExistence(View);
		ReturnValue = TextStorageFoldLines(View->Storage,LineIndex);
		TextViewRedrawRange(View,LineIndex,TextStorageGetLineCount(View->Storage)
			+ TextViewNumVisibleLines(View) + 1);
		return ReturnValue;
	}


/* do a complete redraw of the text view box */
void							TextViewFullRedraw(TextViewRec* View)
	{
		long				Scan;
		long				Limit;

		CheckPtrExistence(View);
		Limit = View->CurrentTopLine + TextViewNumVisibleLines(View);
		for (Scan = View->CurrentTopLine; Scan < Limit; Scan += 1)
			{
				TextViewRedrawLine(View,Scan);
			}
	}


/* redraw the specified line */
void							TextViewRedrawLine(TextViewRec* View, long LineIndex)
	{
		OrdType			YOffset;
		char*				Line;

		CheckPtrExistence(View);
		if ((LineIndex < View->CurrentTopLine)
			|| (LineIndex >= View->CurrentTopLine + TextViewNumVisibleLines(View)))
			{
				/* don't draw lines that aren't on screen */
				return;
			}
		YOffset = (LineIndex - View->CurrentTopLine) * View->LineHeight;
		if (!IsRectVisible(View->Window,View->X,View->Y + YOffset,
			View->Width,View->LineHeight))
			{
				/* don't bother redrawing if the line isn't visible */
				return;
			}
		SetClipRect(View->Window,View->X,View->Y,View->Width,View->Height);
		if ((LineIndex < 0) || (LineIndex >= TextStorageGetLineCount(View->Storage)))
			{
				/* degenerate blank line draw routine for nonexistent lines */
			 NopePoint:
				DrawBoxErase(View->Window,View->X,View->Y + YOffset,
					View->Width,View->LineHeight);
				return;
			}
		Line = GetTextViewSpaceFromTabLine(View,LineIndex);
		if (Line == NIL)
			{
				goto NopePoint;
			}
		if ((LineIndex < View->SelectStartLine) || (LineIndex > View->SelectEndLine)
			|| !View->CursorEnabledFlag)
			{
				/* normal draw -- no part of the line is selected */
				DrawTextLine(View->Window,View->FontID,View->FontSize,
					&(Line[0]),PtrSize(Line),View->X - View->PixelIndent,
					View->Y + YOffset,ePlain);
				DrawBoxErase(View->Window,View->X - View->PixelIndent
					+ LengthOfText(View->FontID,View->FontSize,&(Line[0]),
					PtrSize(Line),ePlain),View->Y + YOffset,
					View->Width + View->PixelIndent,View->LineHeight);
			}
		 else
			{
				if ((View->SelectStartLine == View->SelectEndLine)
					&& (View->SelectStartChar == View->SelectEndCharPlusOne))
					{
						/* it's just an insertion point */
						DrawTextLine(View->Window,View->FontID,View->FontSize,
							&(Line[0]),PtrSize(Line),View->X - View->PixelIndent,
							View->Y + YOffset,ePlain);
						/* erase white space at end of line */
						DrawBoxErase(View->Window,View->X - View->PixelIndent
							+ LengthOfText(View->FontID,View->FontSize,&(Line[0]),
							PtrSize(Line),ePlain),View->Y + YOffset,
							View->Width + View->PixelIndent,View->LineHeight);
						if (View->CursorDrawnFlag)
							{
								DrawLine(View->Window,eBlack,View->X - View->PixelIndent
									+ TextViewScreenXFromCharIndex(View,LineIndex,
									View->SelectStartChar),
									View->Y + YOffset,0,View->LineHeight - 1);
							}
					}
				 else
					{
						long				SelectStartColumn;
						long				SelectEndColumnPlusOne;

						/* real live selection */
						/* since SelectStart/End deals in chars, but we deal in columns */
						/* (i.e. tabs expanded), we have to do a conversion: */
						SelectStartColumn = TextViewCalculateColumnFromCharIndex(View,
							View->SelectStartLine,View->SelectStartChar);
						SelectEndColumnPlusOne = TextViewCalculateColumnFromCharIndex(View,
							View->SelectEndLine,View->SelectEndCharPlusOne);
						if ((View->SelectStartLine == LineIndex)
							&& (View->SelectEndLine == LineIndex))
							{
								OrdType				StepWidth;

								/* it's all on the same line, but not an insertion point */
								/* draw leading white area */
								DrawTextLine(View->Window,View->FontID,View->FontSize,
									&(Line[0]),SelectStartColumn,
									View->X - View->PixelIndent,View->Y + YOffset,ePlain);
								/* find width of leading white area */
								StepWidth = LengthOfText(View->FontID,View->FontSize,
									&(Line[0]),SelectStartColumn,ePlain);
								/* draw middle black area */
								InvertedTextLine(View->Window,View->FontID,View->FontSize,
									&(Line[SelectStartColumn]),
									SelectEndColumnPlusOne - SelectStartColumn,
									View->X - View->PixelIndent + StepWidth,
									View->Y + YOffset,ePlain);
								/* find width of middle black area */
								StepWidth += LengthOfText(View->FontID,View->FontSize,
									&(Line[SelectStartColumn]),
									SelectEndColumnPlusOne - SelectStartColumn,ePlain);
								/* draw trailing white area */
								DrawTextLine(View->Window,View->FontID,View->FontSize,
									&(Line[SelectEndColumnPlusOne]),
									PtrSize(Line) - SelectEndColumnPlusOne,
									View->X - View->PixelIndent + StepWidth,
									View->Y + YOffset,ePlain);
								/* calculate start of white space after eoln */
								StepWidth += LengthOfText(View->FontID,View->FontSize,
									&(Line[SelectEndColumnPlusOne]),
									PtrSize(Line) - SelectEndColumnPlusOne,ePlain);
								/* erase white space after end of line */
								DrawBoxErase(View->Window,View->X - View->PixelIndent
									+ StepWidth,View->Y + YOffset,View->Width
									+ View->PixelIndent,View->LineHeight);
							}
						else if ((View->SelectStartLine < LineIndex)
							&& (View->SelectEndLine > LineIndex))
							{
								/* whole line is inverted */
								InvertedTextLine(View->Window,View->FontID,View->FontSize,
									&(Line[0]),PtrSize(Line),View->X - View->PixelIndent,
									View->Y + YOffset,ePlain);
								DrawBoxPaint(View->Window,eBlack,View->X - View->PixelIndent
									+ LengthOfText(View->FontID,View->FontSize,&(Line[0]),
									PtrSize(Line),ePlain),View->Y + YOffset,
									View->Width + View->PixelIndent,View->LineHeight);
							}
						else if (View->SelectStartLine == LineIndex)
							{
								OrdType				StepWidth;

								/* end of line is inverted */
								/* draw leading white area */
								DrawTextLine(View->Window,View->FontID,View->FontSize,
									&(Line[0]),SelectStartColumn,
									View->X - View->PixelIndent,View->Y + YOffset,ePlain);
								/* find width of leading white area */
								StepWidth = LengthOfText(View->FontID,View->FontSize,
									&(Line[0]),SelectStartColumn,ePlain);
								/* draw ending black area */
								InvertedTextLine(View->Window,View->FontID,View->FontSize,
									&(Line[SelectStartColumn]),
									PtrSize(Line) - SelectStartColumn,
									View->X - View->PixelIndent + StepWidth,
									View->Y + YOffset,ePlain);
								/* find width of ending black area */
								StepWidth += LengthOfText(View->FontID,View->FontSize,
									&(Line[SelectStartColumn]),
									PtrSize(Line) - SelectStartColumn,ePlain);
								/* draw black space after end of line */
								DrawBoxPaint(View->Window,eBlack,View->X - View->PixelIndent
									+ StepWidth,View->Y + YOffset,View->Width
									+ View->PixelIndent,View->LineHeight);
							}
						else if (View->SelectEndLine == LineIndex)
							{
								OrdType				StepWidth;

								/* start of line is inverted */
								/* draw leading black area */
								InvertedTextLine(View->Window,View->FontID,View->FontSize,
									&(Line[0]),SelectEndColumnPlusOne,
									View->X - View->PixelIndent,View->Y + YOffset,ePlain);
								/* find width of leading black area */
								StepWidth = LengthOfText(View->FontID,View->FontSize,
									&(Line[0]),SelectEndColumnPlusOne,ePlain);
								/* draw ending white area */
								DrawTextLine(View->Window,View->FontID,View->FontSize,
									&(Line[SelectEndColumnPlusOne]),
									PtrSize(Line) - SelectEndColumnPlusOne,
									View->X - View->PixelIndent + StepWidth,
									View->Y + YOffset,ePlain);
								/* find width of ending white area */
								StepWidth += LengthOfText(View->FontID,View->FontSize,
									&(Line[SelectEndColumnPlusOne]),
									PtrSize(Line) - SelectEndColumnPlusOne,ePlain);
								/* draw white space after end of line */
								DrawBoxErase(View->Window,View->X - View->PixelIndent
									+ StepWidth,View->Y + YOffset,View->Width
									+ View->PixelIndent,View->LineHeight);
							}
						else
							{
								EXECUTE(PRERR(AllowResume,
									"Internal conditional hole in TextViewRedrawLine"));
							}
					}
			}
		ReleasePtr(Line);
	}


/* redraw a series of lines */
void							TextViewRedrawRange(TextViewRec* View, long StartLine, long EndLine)
	{
		long				Scan;

		for (Scan = StartLine; Scan <= EndLine; Scan += 1)
			{
				TextViewRedrawLine(View,Scan);
			}
	}


/* update the cursor.  this should be called every time there is an idle */
/* event.  periodically, it will blink the insertion point state (if there */
/* is no valid selection) */
void							TextViewUpdateCursor(TextViewRec* View)
	{
		CheckPtrExistence(View);
		if (View->CursorEnabledFlag && !TextViewIsThereValidSelection(View)
			&& (TimerDifference(ReadTimer(),View->LastCursorFlashTime)
				> GetCursorBlinkRate()))
			{
				View->CursorDrawnFlag = !View->CursorDrawnFlag;
				TextViewRedrawLine(View,View->SelectStartLine);
				View->LastCursorFlashTime = ReadTimer();
			}
	}


/* find out the pixel index of the left edge of the specified character */
OrdType						TextViewScreenXFromCharIndex(TextViewRec* View, long LineIndex,
										long CharIndex)
	{
		char*					FixedLine;
		OrdType				Indent;
		long					ColumnIndex;

		CheckPtrExistence(View);
		FixedLine = GetTextViewSpaceFromTabLine(View,LineIndex);
		if (FixedLine == NIL)
			{
				return 0;
			}
		ColumnIndex = TextViewCalculateColumnFromCharIndex(View,LineIndex,CharIndex);
		Indent = LengthOfText(View->FontID,View->FontSize,&(FixedLine[0]),
			ColumnIndex,ePlain);
		ReleasePtr(FixedLine);
		return Indent;
	}


/* convert a pixel position into the nearest character */
long							TextViewCharIndexFromScreenX(TextViewRec* View, long LineIndex,
										OrdType ScreenX)
	{
		char*					Line;
		long					ColumnIndex;
		long					CharIndexLowBound;
		long					CharIndexHighBound;
		long					Scan;
		long					Limit;
		OrdType				Length;

		CheckPtrExistence(View);
		if ((LineIndex < 0) || (LineIndex >= GetTextViewNumLines(View)))
			{
				return 0;
			}
		Line = GetTextViewSpaceFromTabLine(View,LineIndex);
		if (Line == NIL)
			{
				return 0;
			}
		Limit = PtrSize(Line);
		Scan = 0;
		Length = 0;
		while (Scan < Limit)
			{
				OrdType			Length2;
				OrdType			Center;

				Length2 = LengthOfText(View->FontID,View->FontSize,&(Line[0]),
					Scan + 1,ePlain);
				Center = (Length + Length2) / 2;
				if (ScreenX <= Length2)
					{
						if (ScreenX - Center < 0)
							{
								ColumnIndex = Scan;
								goto OutPoint;
							}
						 else
							{
								ColumnIndex = Scan + 1;
							}
					}
				Scan += 1;
				Length = Length2;
			}
		ColumnIndex = Limit;
		/* now we have the column index, with tabs expanded; we have to figure */
		/* out what the character index is, with tabs left intact */
	 OutPoint:
		CharIndexLowBound = 0;
		CharIndexHighBound = ColumnIndex; /* tabs never reduce to FEWER spaces! */
		/* test for convergence again... */
		while (CharIndexLowBound < CharIndexHighBound)
			{
				long				MidPoint;
				long				MidColumnIndex;

				MidPoint = (CharIndexLowBound + CharIndexHighBound) / 2;
				MidColumnIndex = TextViewCalculateColumnFromCharIndex(View,LineIndex,MidPoint);
				if (MidColumnIndex < ColumnIndex)
					{
						if (CharIndexLowBound == MidPoint)
							{
								CharIndexLowBound = MidPoint + 1;
							}
						 else
							{
								CharIndexLowBound = MidPoint;
							}
					}
				else if (MidColumnIndex > ColumnIndex)
					{
						CharIndexHighBound = MidPoint;
					}
				else /* they're the same */
					{
						CharIndexLowBound = MidPoint;
						CharIndexHighBound = MidPoint;
					}
			}
		ReleasePtr(Line);
		return CharIndexLowBound;
	}


/* find out how many lines are visible on the screen */
long							TextViewNumVisibleLines(TextViewRec* View)
	{
		long						Temp;

		CheckPtrExistence(View);
		Temp = View->Height / View->LineHeight;
		if ((View->Height % View->LineHeight) != 0)
			{
				/* part of a line is visible */
				Temp += 1;
			}
		return Temp;
	}


/* get the width of the horizontal scrollable area */
OrdType						TextViewGetVirtualWindowWidth(TextViewRec* View)
	{
#if 0
		/* this stuff doesn't quite work (too slow) so it's been commented out */
		long				Scan;
		long				Limit;
		OrdType			CurrentMaxWidth;
		long				LineCount;

		CheckPtrExistence(View);
		Limit = View->CurrentTopLine + TextViewNumVisibleLines(View);
		CurrentMaxWidth = View->Width;
		LineCount = TextStorageGetLineCount(View->Storage);
		if (Limit >= LineCount)
			{
				Limit = LineCount - 1;
			}
		for (Scan = View->CurrentTopLine; Scan < Limit; Scan += 1)
			{
				char*			Line;

				Line = GetTextViewSpaceFromTabLine(View,Scan);
				if (Line != NIL)
					{
						OrdType			Length;

						Length = LengthOfText(View->FontID,View->FontSize,&(Line[0]),
							PtrSize(Line),ePlain);
						if (CurrentMaxWidth < Length)
							{
								CurrentMaxWidth = Length;
							}
						ReleasePtr(Line);
					}
			}
		if (CurrentMaxWidth > View->Width)
			{
				/* make some space so that the cursor is visible */
				CurrentMaxWidth += 3;
			}
		return CurrentMaxWidth;
#else
		return HARDCODEDMAXWIDTH;
#endif
	}


/* get a copy of the specified line */
char*							GetTextViewLine(TextViewRec* View, long LineIndex)
	{
		CheckPtrExistence(View);
		return TextStorageGetLineCopy(View->Storage,LineIndex);
	}


/* get a copy of the specified line, but first convert all tabs to the */
/* appropriate number of spaces */
char*							GetTextViewSpaceFromTabLine(TextViewRec* View, long LineIndex)
	{
		long				Length;
		long				TotalExtraSpace;
		char*				Temp2;
		long				CharScan;
		long				TargetIndex;
		MyBoolean		TabsFound;
		char*				BasePtr;

		CheckPtrExistence(View);
		BasePtr = TextStorageGetActualLine(View->Storage,LineIndex);
		Length = PtrSize(BasePtr);
		TotalExtraSpace = 0;
		TargetIndex = 0;
		TabsFound = False;
		for (CharScan = 0; CharScan < Length; CharScan += 1)
			{
				if (BasePtr[CharScan] == 0x09)
					{
						TotalExtraSpace += View->SpacesPerTab
							- (TargetIndex % View->SpacesPerTab) - 1;
						TargetIndex += View->SpacesPerTab
							- (TargetIndex % View->SpacesPerTab);
						TabsFound = True;
					}
				 else
					{
						TargetIndex += 1;
					}
			}
		if ((TotalExtraSpace != 0) || TabsFound)
			{
				TargetIndex = 0;
				Temp2 = AllocPtrCanFail(Length + TotalExtraSpace,"TabExpandedLine");
				if (Temp2 == NIL)
					{
						return NIL;
					}
				for (CharScan = 0; CharScan < Length; CharScan += 1)
					{
						if (BasePtr[CharScan] == 0x09)
							{
								long							Index;

								Index = View->SpacesPerTab - (TargetIndex % View->SpacesPerTab);
								while (Index > 0)
									{
										PRNGCHK(Temp2,&(Temp2[TargetIndex]),sizeof(char));
										Temp2[TargetIndex] = 0x20;
										TargetIndex += 1;
										Index -= 1;
									}
							}
						 else
							{
								PRNGCHK(Temp2,&(Temp2[TargetIndex]),sizeof(char));
								Temp2[TargetIndex] = BasePtr[CharScan];
								TargetIndex += 1;
							}
					}
				ERROR(PtrSize(Temp2) != TargetIndex,PRERR(AllowResume,
					"GetTextViewSpaceFromTabLine:  Internal error converting tabs to spaces"));
			}
		 else
			{
				Temp2 = AllocPtrCanFail(PtrSize(BasePtr),"TabExpandedLine");
				if (Temp2 == NIL)
					{
						return NIL;
					}
				PRNGCHK(Temp2,&(Temp2[0]),PtrSize(BasePtr));
				CopyData(BasePtr,Temp2,PtrSize(BasePtr));
			}
		return Temp2;
	}


/* given a character index, calculate where the corresponding position is */
/* when tabs have been converted into spaces */
long							TextViewCalculateColumnFromCharIndex(TextViewRec* View,
										long LineIndex, long CharIndex)
	{
		long				CharScan;
		long				TargetIndex;
		char*				BasePtr;

		CheckPtrExistence(View);
		BasePtr = TextStorageGetActualLine(View->Storage,LineIndex);
		if (CharIndex > PtrSize(BasePtr))
			{
				CharIndex = PtrSize(BasePtr);
			}
		ERROR((CharIndex < 0) || (CharIndex > PtrSize(BasePtr)),
			PRERR(ForceAbort,"TextViewCalculateColumnFromCharIndex char range exceeded"));
		TargetIndex = 0;
		for (CharScan = 0; CharScan < CharIndex; CharScan += 1)
			{
				if (BasePtr[CharScan] == 0x09)
					{
						TargetIndex += View->SpacesPerTab
							- (TargetIndex % View->SpacesPerTab);
					}
				 else
					{
						TargetIndex += 1;
					}
			}
		return TargetIndex;
	}


/* put a new line of data in.  data on the old line is replaced */
MyBoolean					SetTextViewLine(TextViewRec* View, long LineIndex, char* LineToCopy)
	{
		MyBoolean				ReturnValue;

		CheckPtrExistence(View);
		ReturnValue = TextStorageChangeLine(View->Storage,LineIndex,LineToCopy);
		if ((View->SelectStartLine == LineIndex) && (View->SelectStartChar
		> TextStorageGetLineLength(View->Storage,LineIndex)))
			{
				View->SelectStartChar = TextStorageGetLineLength(View->Storage,LineIndex);
			}
		if ((View->SelectEndLine == LineIndex) && (View->SelectEndCharPlusOne
			> TextStorageGetLineLength(View->Storage,LineIndex)))
			{
				View->SelectEndCharPlusOne = TextStorageGetLineLength(View->Storage,
					LineIndex);
			}
		TextViewRedrawLine(View,LineIndex);
		return ReturnValue;
	}


/* get a copy of the selected area */
TextStorageRec*		TextViewGetSelection(TextViewRec* View)
	{
		CheckPtrExistence(View);
		return TextStorageExtractSection(View->Storage,View->SelectStartLine,
			View->SelectStartChar,View->SelectEndLine,View->SelectEndCharPlusOne);
	}


/* delete the selection.  if this fails, the selection may have been partially */
/* deleted */
MyBoolean					TextViewDeleteSelection(TextViewRec* View)
	{
		MyBoolean				ReturnValue;

		CheckPtrExistence(View);
		ReturnValue = TextStorageDeleteSection(View->Storage,View->SelectStartLine,
			View->SelectStartChar,View->SelectEndLine,View->SelectEndCharPlusOne);
		View->SelectEndLine = View->SelectStartLine;
		View->SelectEndCharPlusOne = View->SelectStartChar;
		TextViewFullRedraw(View);
		return ReturnValue;
	}


/* insert a new block of data at the insertion point, deleting any existing */
/* selection first.  if this fails, the block may have been partially inserted */
MyBoolean					TextViewInsertBlock(TextViewRec* View, struct TextStorageRec* Data)
	{
		MyBoolean				ReturnValue;

		CheckPtrExistence(View);
		if (TextViewIsThereValidSelection(View))
			{
				TextViewDeleteSelection(View);
			}
		ReturnValue = TextStorageInsertSection(View->Storage,View->SelectStartLine,
			View->SelectStartChar,Data);
		if (ReturnValue)
			{
				if (TextStorageGetLineCount(Data) == 1)
					{
						View->SelectStartChar += TextStorageGetLineLength(Data,0);
					}
				 else
					{
						View->SelectStartChar = TextStorageGetLineLength(Data,
							TextStorageGetLineCount(Data) - 1);
					}
				View->SelectStartLine += TextStorageGetLineCount(Data) - 1;
				View->SelectEndCharPlusOne = View->SelectStartChar;
				View->SelectEndLine = View->SelectStartLine;
			}
		TextViewFullRedraw(View);
		return ReturnValue;
	}


/* convert the text view box into a packed buffer of lines. */
char*							TextViewGetRawData(TextViewRec* View, char* EOLN)
	{
		CheckPtrExistence(View);
		return TextStorageMakeRawBuffer(View->Storage,EOLN);
	}


/* delete the old data in the text view object and replace it with the */
/* specified block of data */
MyBoolean					TextViewNewRawData(TextViewRec* View, char* EOLN, char* RawData)
	{
		TextStorageRec*			Temp;

		CheckPtrExistence(View);
		Temp = TextStorageFromRawBuffer(RawData,EOLN);
		if (Temp == NIL)
			{
				return False;
			}
		DisposeTextStorage(View->Storage);
		View->Storage = Temp;
		View->SelectStartLine = 0;
		View->SelectStartChar = 0;
		View->SelectEndLine = 0;
		View->SelectEndCharPlusOne = 0;
		TextViewFullRedraw(View);
		return True;
	}


/* write the data to a file.  returns True if successful */
MyBoolean					TextViewWriteDataToFile(TextViewRec* View,
										struct FileType* FileRefNum, char* EOLN)
	{
		CheckPtrExistence(View);
		return TextStorageWriteDataToFile(View->Storage,FileRefNum,EOLN);
	}
