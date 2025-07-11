/* TextEdit.c */
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
#include "Debug.h"
#include "Audit.h"
#include "Definitions.h"

#include "TextEdit.h"
#include "Memory.h"
#include "Scroll.h"
#include "TextView.h"
#include "Scrap.h"
#include "TextStorage.h"
#include "DataMunging.h"
#include "EventLoop.h"


#define BorderWidth (3)

/* click phases for selecting things */
typedef enum {eNoClick, eSingleClick, eDoubleClick, eTripleClick} ClickStates;


typedef struct
	{
		/* this flag indicates if the undo record is valid */
		MyBoolean						CanUndoFlag;
		/* this info remembers what was deleted, if any */
		MyBoolean						DeletedValidFlag;
		long								DeletedLine; /* where it came from */
		long								DeletedChar;
		TextStorageRec*			DeletedStuff;
		/* this info remembers what was/is replacing the deleted stuff, if any */
		MyBoolean						ReplacingValidFlag;
		long								ReplacingStartLine;
		long								ReplacingStartChar;
		long								ReplacingEndLine;
		long								ReplacingEndCharPlusOne;
	} UndoRec;


/* prototypes for undo routines */
static void				TextEditBlockRemovedUndoSave(TextEditRec* Edit,
										TextStorageRec* Stuff, long WhereLine, long WhereChar);
static void				TextEditPurgeUndoRecord(TextEditRec* Edit);
static void				TextEditKeyPressedUndoSave(TextEditRec* Edit);
static void				TextEditRememberUndoDeletedCR(TextEditRec* Edit);
static void				TextEditRememberUndoDeletedChar(TextEditRec* Edit, char What);


struct TextEditRec
	{
		WinType*						Window;
		OrdType							X;
		OrdType							Y;
		OrdType							TextAreaWidth;
		OrdType							TextAreaHeight;
		OrdType							TotalWidth;
		OrdType							TotalHeight;
		ScrollRec*					HorizontalScroll;
		ScrollRec*					VerticalScroll;
		TextViewRec*				View;
		MyBoolean						AutoIndent;
		ClickStates					ClickPhase;
		OrdType							LastClickX;
		OrdType							LastClickY;
		double							LastClickTime;
		UndoRec							Undo;
	};


/* create a new, empty text edit */
TextEditRec*			NewTextEdit(WinType* Window, TEScrollType ScrollStuff,
										FontType FontID, FontSizeType FontSize, OrdType X, OrdType Y,
										OrdType Width, OrdType Height)
	{
		TextEditRec*		Edit;

		Edit = (TextEditRec*)AllocPtrCanFail(sizeof(TextEditRec),"TextEditRec");
		if (Edit == NIL)
			{
			 MemOut1:
				return NIL;
			}
		Edit->Window = Window;
		Edit->X = X;
		Edit->Y = Y;
		Edit->TotalWidth = Width;
		Edit->TotalHeight = Height;
		if ((ScrollStuff & eTEVScrollBar) != 0)
			{
				Width = Width - 16 + 1;
			}
		if ((ScrollStuff & eTEHScrollBar) != 0)
			{
				Height = Height - 16 + 1;
			}
		if ((ScrollStuff & eTEVScrollBar) != 0)
			{
				Edit->VerticalScroll = NewScrollBar(Window,eVScrollBar,
					X + Width - 1,Y,Height);
				if (Edit->VerticalScroll == NIL)
					{
					 MemOut2:
						ReleasePtr((char*)Edit);
						goto MemOut1;
					}
			}
		 else
			{
				Edit->VerticalScroll = NIL;
			}
		if ((ScrollStuff & eTEHScrollBar) != 0)
			{
				Edit->HorizontalScroll = NewScrollBar(Window,eHScrollBar,
					X,Y + Height - 1,Width);
				if (Edit->HorizontalScroll == NIL)
					{
					 MemOut3:
						if (Edit->VerticalScroll != NIL)
							{
								DisposeScrollBar(Edit->VerticalScroll);
							}
						goto MemOut2;
					}
			}
		 else
			{
				Edit->HorizontalScroll = NIL;
			}
		Edit->TextAreaWidth = Width;
		Edit->TextAreaHeight = Height;
		Edit->View = NewTextView(Window,X + BorderWidth,Y + BorderWidth,
			Width - (BorderWidth * 2), Height - (BorderWidth * 2),8,FontID,FontSize);
		if (Edit->View == NIL)
			{
			 MemOut4:
				if (Edit->HorizontalScroll != NIL)
					{
						DisposeScrollBar(Edit->HorizontalScroll);
					}
				goto MemOut3;
			}
		Edit->ClickPhase = eNoClick;
		Edit->LastClickTime = ReadTimer();
		Edit->AutoIndent = False;
		Edit->Undo.CanUndoFlag = False;
		TextEditRecalcVerticalScroll(Edit);
		TextEditRecalcHorizontalScroll(Edit);
		return Edit;
	}


/* dispose text edit and all text it contains */
void							DisposeTextEdit(TextEditRec* Edit)
	{
		TextEditPurgeUndoRecord(Edit);
		DisposeTextView(Edit->View);
		if (Edit->VerticalScroll != NIL)
			{
				DisposeScrollBar(Edit->VerticalScroll);
			}
		if (Edit->HorizontalScroll != NIL)
			{
				DisposeScrollBar(Edit->HorizontalScroll);
			}
		ReleasePtr((char*)Edit);
	}


/* get location of text edit */
OrdType						GetTextEditXLoc(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return Edit->X;
	}


/* get location of text edit */
OrdType						GetTextEditYLoc(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return Edit->Y;
	}


/* get location of text edit */
OrdType						GetTextEditWidth(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return Edit->TotalWidth;
	}


/* get location of text edit */
OrdType						GetTextEditHeight(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return Edit->TotalHeight;
	}


/* get font stuff for text edit */
FontType					GetTextEditFont(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return GetTextViewFont(Edit->View);
	}


/* get font stuff for text edit */
FontSizeType			GetTextEditPointSize(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return GetTextViewPointSize(Edit->View);
	}


/* get the number of spaces per tab character */
long							GetTextEditSpacesPerTab(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return GetTextViewSpacesPerTab(Edit->View);
	}


/* get the index of the top line in the window */
long							GetTextEditTopLine(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return GetTextViewTopLine(Edit->View);
	}


/* get the pixel index of the leftmost text of the text box */
OrdType						GetTextEditPixelIndent(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return GetTextViewPixelIndent(Edit->View);
	}


/* returns True if the selection is non-empty, or false if it's an insertion point */
MyBoolean					TextEditIsThereValidSelection(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return TextViewIsThereValidSelection(Edit->View);
	}


/* get the line number of the start of the selection */
long							GetTextEditSelectStartLine(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return GetTextViewSelectStartLine(Edit->View);
	}


/* get the line number of the end of the selection */
long							GetTextEditSelectEndLine(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return GetTextViewSelectEndLine(Edit->View);
	}


/* get the character index of the start of the selection */
long							GetTextEditSelectStartChar(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return GetTextViewSelectStartChar(Edit->View);
	}


/* get the character index of the character immediately after the end of the */
/* selection.  (if this == start char and startline == endline, then there is no */
/* space between them and therefore there is no selection */
long							GetTextEditSelectEndCharPlusOne(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return GetTextViewSelectEndCharPlusOne(Edit->View);
	}


/* find out if selection & scrollbar display is enabled */
MyBoolean					TextEditIsShowSelectionEnabled(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return TextViewIsShowSelectionEnabled(Edit->View);
	}


/* find out if the data has been modified since the last call to TextEditHasBeenSaved */
MyBoolean					TextEditDoesItNeedToBeSaved(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return TextViewDoesItNeedToBeSaved(Edit->View);
	}


/* get the total number of lines contained in the edit */
long							GetTextEditNumLines(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return GetTextViewNumLines(Edit->View);
	}


/* find out if auto-indent upon newline is enabled */
MyBoolean					TextEditIsAutoIndentEnabled(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return Edit->AutoIndent;
	}


/* find out if it is possible to undo the last operation (for enabling menu item) */
MyBoolean					TextEditCanWeUndo(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		return Edit->Undo.CanUndoFlag;
	}


/* change the screen location of the text edit box */
void							SetTextEditPosition(TextEditRec* Edit, OrdType X, OrdType Y,
										OrdType Width, OrdType Height)
	{
		CheckPtrExistence(Edit);
		Edit->X = X;
		Edit->Y = Y;
		Edit->TotalWidth = Width;
		Edit->TotalHeight = Height;
		if (Edit->VerticalScroll != NIL)
			{
				Width = Width - 16 + 1;
			}
		if (Edit->HorizontalScroll != NIL)
			{
				Height = Height - 16 + 1;
			}
		if (Edit->VerticalScroll != NIL)
			{
				SetScrollLocation(Edit->VerticalScroll,X + Width - 1,Y,Height);
			}
		if (Edit->HorizontalScroll != NIL)
			{
				SetScrollLocation(Edit->HorizontalScroll,X,Y + Height - 1,Width);
			}
		Edit->TextAreaWidth = Width;
		Edit->TextAreaHeight = Height;
		SetTextViewPosition(Edit->View,X + BorderWidth,Y + BorderWidth,
			Width - (BorderWidth * 2),Height - (BorderWidth * 2));
		TextEditRecalcVerticalScroll(Edit);
		TextEditRecalcHorizontalScroll(Edit);
		TextEditFullRedraw(Edit);
	}


/* change the font being used to display the text */
void							SetTextEditFontStuff(TextEditRec* Edit, FontType Font,
										FontSizeType Size)
	{
		CheckPtrExistence(Edit);
		SetTextViewFontStuff(Edit->View,Font,Size);
		TextEditRecalcVerticalScroll(Edit); /* line height may change */
	}


/* set the number of spaces displayed for a tab */
void							SetTextEditTabSize(TextEditRec* Edit, long SpacesPerTab)
	{
		CheckPtrExistence(Edit);
		SetTextViewTabSize(Edit->View,SpacesPerTab);
		TextEditRecalcHorizontalScroll(Edit); /* line length may change */
	}


/* change the top line being displayed in the exit box */
void							SetTextEditTopLine(TextEditRec* Edit, long NewTopLine)
	{
		CheckPtrExistence(Edit);
		SetTextViewTopLine(Edit->View,NewTopLine);
		TextEditRecalcVerticalScroll(Edit);
	}


/* change the pixel index of the left edge of the text box */
void							SetTextEditPixelIndent(TextEditRec* Edit, OrdType NewPixelIndent)
	{
		CheckPtrExistence(Edit);
		SetTextViewPixelIndent(Edit->View,NewPixelIndent);
		TextEditRecalcHorizontalScroll(Edit);
	}


/* set the selection to a specified range */
void							SetTextEditSelection(TextEditRec* Edit, long StartLine,
										long StartChar, long EndLine, long EndCharPlusOne)
	{
		CheckPtrExistence(Edit);
		SetTextViewSelection(Edit->View,StartLine,StartChar,EndLine,EndCharPlusOne);
	}


/* set the selection to an insertion point at the specified position */
void							SetTextEditInsertionPoint(TextEditRec* Edit, long Line, long Char)
	{
		CheckPtrExistence(Edit);
		SetTextViewInsertionPoint(Edit->View,Line,Char);
	}


/* enable display of selection and scrollbars */
void							EnableTextEditSelection(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		EnableTextViewSelection(Edit->View);
		if (Edit->HorizontalScroll != NIL)
			{
				EnableScrollBar(Edit->HorizontalScroll);
			}
		if (Edit->VerticalScroll != NIL)
			{
				EnableScrollBar(Edit->VerticalScroll);
			}
		TextEditRedrawFrame(Edit);
	}


/* disable display of selection and scrollbars */
void							DisableTextEditSelection(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		DisableTextViewSelection(Edit->View);
		if (Edit->HorizontalScroll != NIL)
			{
				DisableScrollBar(Edit->HorizontalScroll);
			}
		if (Edit->VerticalScroll != NIL)
			{
				DisableScrollBar(Edit->VerticalScroll);
			}
		TextEditRedrawFrame(Edit);
	}


/* indicate that any data in the text edit has been saved.  After this call, */
/* TextEditDoesItNeedToBeSaved will return False.  It will start returning true */
/* if any subsequent changes are made. */
void							TextEditHasBeenSaved(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		TextViewHasBeenSaved(Edit->View);
	}


/* enable or disable auto-indent on carriage return */
void							SetTextEditAutoIndent(TextEditRec* Edit, MyBoolean AutoIndentFlag)
	{
		CheckPtrExistence(Edit);
		Edit->AutoIndent = AutoIndentFlag;
	}


/* recalculate the position index of the vertical scrollbar */
void							TextEditRecalcVerticalScroll(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		if (Edit->VerticalScroll != NIL)
			{
				SetMaxScrollIndex(Edit->VerticalScroll,GetTextEditNumLines(Edit)
					- TextViewNumVisibleLines(Edit->View) + 2/*why 2?*/);
				SetScrollIndex(Edit->VerticalScroll,GetTextEditTopLine(Edit));
				RedrawScrollBar(Edit->VerticalScroll);
			}
	}


/* recalculate the position index of the horizontal scrollbar */
void							TextEditRecalcHorizontalScroll(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		if (Edit->HorizontalScroll != NIL)
			{
				SetMaxScrollIndex(Edit->HorizontalScroll,
					TextViewGetVirtualWindowWidth(Edit->View) - GetTextViewWidth(Edit->View));
				SetScrollIndex(Edit->HorizontalScroll,GetTextEditPixelIndent(Edit));
				RedrawScrollBar(Edit->HorizontalScroll);
			}
	}


/* redraw the entire text edit box */
void							TextEditFullRedraw(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		if (Edit->VerticalScroll != NIL)
			{
				RedrawScrollBar(Edit->VerticalScroll);
			}
		if (Edit->HorizontalScroll != NIL)
			{
				RedrawScrollBar(Edit->HorizontalScroll);
			}
		TextViewFullRedraw(Edit->View);
		TextEditRedrawFrame(Edit);
	}


/* redraw the outline frame of the text edit box */
void							TextEditRedrawFrame(TextEditRec* Edit)
	{
		Patterns				PatternToUse;

		CheckPtrExistence(Edit);
		if (TextEditIsShowSelectionEnabled(Edit))
			{
				PatternToUse = eBlack;
			}
		 else
			{
				PatternToUse = eMediumGrey;
			}
		SetClipRect(Edit->Window,Edit->X,Edit->Y,
			Edit->TextAreaWidth,Edit->TextAreaHeight);
		DrawBoxFrame(Edit->Window,PatternToUse,Edit->X,Edit->Y,
			Edit->TextAreaWidth,Edit->TextAreaHeight);
		DrawBoxFrame(Edit->Window,eWhite,Edit->X + 1,Edit->Y + 1,
			Edit->TextAreaWidth - 2,Edit->TextAreaHeight - 2);
		DrawBoxFrame(Edit->Window,eWhite,Edit->X + 2,Edit->Y + 2,
			Edit->TextAreaWidth - 4,Edit->TextAreaHeight - 4);
	}


/* update cursor.  This should be called during idle events.  It keeps track of */
/* when the cursor was last blinked and blinks the cursor again if necessary. */
void							TextEditUpdateCursor(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		TextViewUpdateCursor(Edit->View);
	}


/* get the specified line of text from the exit */
char*							GetTextEditLine(TextEditRec* Edit, long LineIndex)
	{
		CheckPtrExistence(Edit);
		return GetTextViewLine(Edit->View,LineIndex);
	}


/* get a line of text, but first convert all tabs in the line into the */
/* proper number of spaces. */
char*							GetTextEditSpaceFromTabLine(TextEditRec* Edit, long LineIndex)
	{
		CheckPtrExistence(Edit);
		return GetTextViewSpaceFromTabLine(Edit->View,LineIndex);
	}


/* put a new line in the text box.  This overwrites data already on that line */
MyBoolean					SetTextEditLine(TextEditRec* Edit, long LineIndex, char* LineToCopy)
	{
		CheckPtrExistence(Edit);
		TextEditPurgeUndoRecord(Edit); /* this routine modifies data, so remove undo info */
		return SetTextViewLine(Edit->View,LineIndex,LineToCopy);
	}


/* use the LineFeed string to create a single block of text containing all */
/* of the lines packed into it */
char*							TextEditGetRawData(TextEditRec* Edit, char* LineFeed)
	{
		CheckPtrExistence(Edit);
		return TextViewGetRawData(Edit->View,LineFeed);
	}


/* put new data into the text edit.  The RawData is a block with all text lines */
/* packed into it separated by the LineFeed string. */
MyBoolean					TextEditNewRawData(TextEditRec* Edit, char* RawData, char* LineFeed)
	{
		MyBoolean				ReturnValue;

		CheckPtrExistence(Edit);
		TextEditPurgeUndoRecord(Edit); /* this routine modifies data, so remove undo info */
		ReturnValue = TextViewNewRawData(Edit->View,LineFeed,RawData);
		TextEditRecalcVerticalScroll(Edit);
		TextEditRecalcHorizontalScroll(Edit);
		return ReturnValue;
	}


/* get a text block containing the selected data */
char*							TextEditGetSelection(TextEditRec* Edit)
	{
		TextStorageRec*			Storage;
		char*								Raw;

		CheckPtrExistence(Edit);
		Storage = TextViewGetSelection(Edit->View);
		if (Storage == NIL)
			{
				return NIL;
			}
		/* selection uses system's line feed */
		Raw = TextStorageMakeRawBuffer(Storage,SYSTEMLINEFEED);
		DisposeTextStorage(Storage);
		if (Raw == NIL)
			{
				return NIL;
			}
		return Raw;
	}


/* replace the current selection (if any) with the specified raw data block. */
/* if this fails, some of the data may have been inserted */
MyBoolean					TextEditInsertRawData(TextEditRec* Edit, char* RawData, char* LineFeed)
	{
		TextStorageRec*	Block;
		MyBoolean				ReturnValue;

		CheckPtrExistence(Edit);
		CheckPtrExistence(RawData);
		TextEditPurgeUndoRecord(Edit); /* this routine modifies data, so remove undo info */
		Block = TextStorageFromRawBuffer(RawData,LineFeed);
		if (Block == NIL)
			{
				return False;
			}
		ReturnValue = TextViewInsertBlock(Edit->View,Block);
		DisposeTextStorage(Block);
		TextEditRecalcVerticalScroll(Edit);
		TextEditRecalcHorizontalScroll(Edit);
		return ReturnValue;
	}


/* find the union of two selection ranges */
void							UnionSelection(SelRec One, SelRec Two, SelRec Three,
										SelRec* Start, SelRec* End)
	{
		SortSelection(&One,&Three);
		SortSelection(&One,&Two);
		SortSelection(&Two,&Three);
		*Start = One;
		*End = Three;
	}


/* find the difference (union - intersection) of two selection ranges.  This is */
/* used to avoid redrawing the entire selection range all the time */
void							DiffSelection(SelRec OneStart, SelRec OneEnd, SelRec TwoStart,
										SelRec TwoEnd, SelRec* OutStart, SelRec* OutEnd)
	{
		SortSelection(&OneStart,&OneEnd);
		SortSelection(&TwoStart,&TwoEnd);
		if (GreaterThan(&OneStart,&TwoStart))
			{
				*OutStart = TwoStart;
				*OutEnd = OneStart;
				if (GreaterThan(&OneEnd,&TwoEnd))
					{
						*OutEnd = OneEnd;
						return;
					}
				if (GreaterThan(&TwoEnd,&OneEnd))
					{
						*OutEnd = TwoEnd;
						return;
					}
				return;
			}
		if (GreaterThan(&TwoStart,&OneStart))
			{
				*OutStart = OneStart;
				*OutEnd = TwoStart;
				if (GreaterThan(&OneEnd,&TwoEnd))
					{
						*OutEnd = OneEnd;
						return;
					}
				if (GreaterThan(&TwoEnd,&OneEnd))
					{
						*OutEnd = TwoEnd;
						return;
					}
				return;
			}
		if (GreaterThan(&OneEnd,&TwoEnd))
			{
				*OutStart = TwoEnd;
				*OutEnd = OneEnd;
				return;
			}
		if (GreaterThan(&TwoEnd,&OneEnd))
			{
				*OutStart = OneEnd;
				*OutEnd = TwoEnd;
				return;
			}
		*OutStart = OneStart;
		*OutEnd = OneStart;
	}


/* if the first selection point is after the second then reverse their order */
void							SortSelection(SelRec* One, SelRec* Two)
	{
		SelRec			Temp;

		if (GreaterThan(One,Two))
			{
				Temp = *One;
				*One = *Two;
				*Two = Temp;
			}
	}


/* returns True if the first selection point is after the second one */
MyBoolean					GreaterThan(SelRec* One, SelRec* Two)
	{
		if (One->Line == Two->Line)
			{
				if (One->Column > Two->Column)
					{
						return True;
					}
			}
		 else
			{
				if (One->Line > Two->Line)
					{
						return True;
					}
			}
		return False;
	}


/* extend the selection using the current mouse-click state (single, double, triple) */
void							ExtendSelection(TextEditRec* Edit, SelRec* Start, SelRec* End)
	{
		char*				Buffer;
		long				LineLength;

		CheckPtrExistence(Edit);
		switch (Edit->ClickPhase)
			{
				case eNoClick:
					break;
				case eSingleClick:
					break;
				case eDoubleClick:
					if ((Start != NIL) && (Start->Line >= 0)
						&& (Start->Line < GetTextEditNumLines(Edit)))
						{
							Buffer = GetTextEditLine(Edit,Start->Line);
							if (Buffer != NIL)
								{
									LineLength = PtrSize(Buffer);
									if (AlphaNum(Buffer[Start->Column]))
										{
											while ((Start->Column > 0)
												&& AlphaNum(Buffer[Start->Column - 1]))
												{
													Start->Column -= 1;
												}
										}
									ReleasePtr(Buffer);
								}
						}
					if ((End != NIL) && (End->Line >= 0)
						&& (End->Line < GetTextEditNumLines(Edit)))
						{
							Buffer = GetTextEditLine(Edit,End->Line);
							if (Buffer != NIL)
								{
									LineLength = PtrSize(Buffer);
									while ((End->Column < LineLength)
										&& AlphaNum(Buffer[End->Column]))
										{
											End->Column += 1;
										}
									ReleasePtr(Buffer);
								}
						}
					break;
				case eTripleClick:
					if (Start != NIL)
						{
							Start->Column = 0;
						}
					if ((End != NIL) && (End->Column != 0))
						{
							End->Column = 0;
							End->Line += 1;
						}
					break;
			}
	}


/* find out if the character is an alphanumeric character.  this is used by */
/* ExtendSelection for figuring out where double-click extends should stop. */
MyBoolean					AlphaNum(char It)
	{
		return ((It >= 'a') && (It <= 'z')) || ((It >= 'A') && (It <= 'Z'))
			|| ((It >= '0') && (It <= '9')) || (It == '_');
	}


/* append a line of text to the end of the text edit.  This can be used if the */
/* text edit box is being used as an interaction (terminal) window */
/* if NIL is passed in for Data, a blank line will be appended */
MyBoolean					TextEditAppendLineInteraction(TextEditRec* Edit, char* Data)
	{
		CheckPtrExistence(Edit);
		TextEditPurgeUndoRecord(Edit); /* this routine modifies data, so remove undo info */
		SetTextEditInsertionPoint(Edit,GetTextViewNumLines(Edit->View) - 1,
			GetTextViewLineLength(Edit->View,GetTextViewNumLines(Edit->View) - 1));
		if (!TextViewBreakLine(Edit->View,GetTextEditSelectStartLine(Edit),
			GetTextEditSelectStartChar(Edit)))
			{
				return False;
			}
		SetTextEditInsertionPoint(Edit,GetTextEditSelectStartLine(Edit) + 1,0);
		if (Data == NIL)
			{
				return True;
			}
		return SetTextViewLine(Edit->View,GetTextEditSelectStartLine(Edit),Data);
	}


/* dump the data contained in the text edit to the current position in the */
/* specified file.  returns True if all the data was written successfully */
MyBoolean					TestEditWriteDataToFile(TextEditRec* Edit,
										struct FileType* FileRefNum, char* EOLN)
	{
		CheckPtrExistence(Edit);
		return TextViewWriteDataToFile(Edit->View,FileRefNum,EOLN);
	}


/* cut the selected data to the clipboard.  if this fails, some of the data */
/* may have been deleted */
MyBoolean					TextEditDoMenuCut(TextEditRec* Edit)
	{
		TextStorageRec*			Removed;
		TextStorageRec*			UndoInfo;
		char*								RawBlock;
		MyBoolean						Flag;

		CheckPtrExistence(Edit);
		Removed = TextViewGetSelection(Edit->View);
		if (Removed == NIL)
			{
			 FailurePoint1:
				return False;
			}
		UndoInfo = TextViewGetSelection(Edit->View);
		if (UndoInfo == NIL)
			{
			 FailurePoint2:
				DisposeTextStorage(Removed);
				goto FailurePoint1;
			}
		RawBlock = TextStorageMakeRawBuffer(Removed,SYSTEMLINEFEED);
		if (RawBlock == NIL)
			{
			 FailurePoint3:
				DisposeTextStorage(UndoInfo);
				goto FailurePoint2;
			}
		Flag = SetScrapToThis(RawBlock);
		ReleasePtr(RawBlock);
		if (!Flag)
			{
				goto FailurePoint3;
			}
		DisposeTextStorage(Removed);

		if (!TextViewDeleteSelection(Edit->View))
			{
				goto FailurePoint3;
			}
		TextEditBlockRemovedUndoSave(Edit,UndoInfo,GetTextEditSelectStartLine(Edit),
			GetTextEditSelectStartChar(Edit));
		TextEditRecalcVerticalScroll(Edit);
		TextEditRecalcHorizontalScroll(Edit);
		TextEditShowSelection(Edit);
		return True;
	}


/* copy the selected data to the clipboard */
MyBoolean					TextEditDoMenuCopy(TextEditRec* Edit)
	{
		TextStorageRec*	Removed;
		char*						RawBlock;
		MyBoolean				Flag;

		CheckPtrExistence(Edit);
		Removed = TextViewGetSelection(Edit->View);
		if (Removed == NIL)
			{
				return False;
			}
		RawBlock = TextStorageMakeRawBuffer(Removed,SYSTEMLINEFEED);
		DisposeTextStorage(Removed);
		if (RawBlock == NIL)
			{
				return False;
			}
		Flag = SetScrapToThis(RawBlock);
		ReleasePtr(RawBlock);
		if (!Flag)
			{
				return False;
			}
		return True;
	}


/* paste the clipboard in, replacing the current selection if there is one */
/* if this fails, some of the data may have been inserted */
MyBoolean					TextEditDoMenuPaste(TextEditRec* Edit)
	{
		char*						RawBlock;
		TextStorageRec*	ToBeInserted;

		CheckPtrExistence(Edit);
		RawBlock = GetCopyOfScrap();
		if (RawBlock == NIL)
			{
			 FailurePoint1:
				return False;
			}
		ToBeInserted = TextStorageFromRawBuffer(RawBlock,SYSTEMLINEFEED);
		ReleasePtr(RawBlock);
		if (ToBeInserted == NIL)
			{
				goto FailurePoint1;
			}
		if (!TextEditDoMenuClear(Edit)) /* erase old selection & save it in Undo record */
			{
			 FailurePoint2:
				DisposeTextStorage(ToBeInserted);
				goto FailurePoint1;
			}
		Edit->Undo.ReplacingValidFlag = True;
		Edit->Undo.ReplacingStartLine = GetTextEditSelectStartLine(Edit);
		Edit->Undo.ReplacingStartChar = GetTextEditSelectStartChar(Edit);
		if (!TextViewInsertBlock(Edit->View,ToBeInserted))
			{
				goto FailurePoint2;
			}
		Edit->Undo.ReplacingEndLine = GetTextEditSelectEndLine(Edit);
		Edit->Undo.ReplacingEndCharPlusOne = GetTextEditSelectEndCharPlusOne(Edit);
		DisposeTextStorage(ToBeInserted);
		TextEditRecalcVerticalScroll(Edit);
		TextEditRecalcHorizontalScroll(Edit);
		TextEditShowSelection(Edit);
		return True;
	}


/* select the entire data area of the text edit */
void							TextEditDoMenuSelectAll(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		SetTextViewSelection(Edit->View,0,0,GetTextViewNumLines(Edit->View) - 1,
			GetTextViewLineLength(Edit->View,GetTextViewNumLines(Edit->View) - 1));
	}


/* delete the selected area.  This is the same as pressing the delete key */
/* when there is a valid selection.  if this fails, some of the data may */
/* have been deleted */
MyBoolean					TextEditDoMenuClear(TextEditRec* Edit)
	{
		TextStorageRec*			Deleted;

		/* this routine is called from several other routines which need to delete */
		/* a selection before inserting.  Therefore, we can do the delete-save part of */
		/* the 'undo' info construction in this routine. */
		CheckPtrExistence(Edit);
		Deleted = TextViewGetSelection(Edit->View);
		if (Deleted == NIL)
			{
			 FailurePoint1:
				return False;
			}
		if (!TextViewDeleteSelection(Edit->View))
			{
				DisposeTextStorage(Deleted);
				goto FailurePoint1;
			}
		TextEditBlockRemovedUndoSave(Edit,Deleted,GetTextEditSelectStartLine(Edit),
			GetTextEditSelectStartChar(Edit));
		TextEditRecalcVerticalScroll(Edit);
		TextEditRecalcHorizontalScroll(Edit);
		TextEditShowSelection(Edit);
		return True;
	}


/* shift the selection toward the left margin by deleting one tab (or spaces) */
/* from the beginning of the line.  It will not remove non-whitespace characters */
/* if this fails, some of the lines may have been shifted */
MyBoolean					TextEditShiftSelectionLeftOneTab(TextEditRec* Edit)
	{
		long					Scan;
		long					Limit;
		long					SpacesPerTab;

		CheckPtrExistence(Edit);
		Limit = GetTextViewSelectEndLine(Edit->View);
		if (TextEditIsThereValidSelection(Edit)
			&& (GetTextViewSelectEndCharPlusOne(Edit->View) == 0))
			{
				Limit -= 1;
			}
		SpacesPerTab = GetTextViewSpacesPerTab(Edit->View);
		for (Scan = GetTextViewSelectStartLine(Edit->View); Scan <= Limit; Scan += 1)
			{
				char*					LineCopy;
				char*					Line;
				long					CharScan;
				long					ColumnCount;

				Line = GetTextViewLine(Edit->View,Scan);
				if (Line == NIL)
					{
						return False;
					}
				CharScan = 0;
				ColumnCount = 0;
				while ((CharScan < PtrSize(Line)) && (ColumnCount < SpacesPerTab))
					{
						if (Line[CharScan] == ' ')
							{
								CharScan += 1;
								ColumnCount += 1;
							}
						else if (Line[CharScan] == 9)
							{
								CharScan += 1;
								ColumnCount = SpacesPerTab; /* cause loop termination */
							}
						else
							{
								ColumnCount = SpacesPerTab; /* cause loop termination */
							}
					}
				LineCopy = RemoveBlockFromBlockCopy(Line,0,CharScan);
				ReleasePtr(Line);
				if (LineCopy == NIL)
					{
						return False;
					}
				SetTextEditLine(Edit,Scan,LineCopy);
				ReleasePtr(LineCopy);
			}
		if (TextEditIsThereValidSelection(Edit)
			&& (GetTextViewSelectEndCharPlusOne(Edit->View) == 0))
			{
				SetTextViewSelection(Edit->View,GetTextViewSelectStartLine(Edit->View),
					0,GetTextViewSelectEndLine(Edit->View),0);
			}
		 else
			{
				SetTextViewSelection(Edit->View,GetTextViewSelectStartLine(Edit->View),
					0,GetTextViewSelectEndLine(Edit->View) + 1,0);
			}
		TextEditRecalcHorizontalScroll(Edit);
		return True;
	}


/* shift selection toward the right margin by inserting a tab at the */
/* beginning of each line.  if this fails, some of the lines may have been shifted. */
MyBoolean					TextEditShiftSelectionRightOneTab(TextEditRec* Edit)
	{
		long					Scan;
		long					Limit;
		char					MyTab[1] = {9};

		CheckPtrExistence(Edit);
		Limit = GetTextViewSelectEndLine(Edit->View);
		if (TextEditIsThereValidSelection(Edit)
			&& (GetTextViewSelectEndCharPlusOne(Edit->View) == 0))
			{
				Limit -= 1;
			}
		for (Scan = GetTextViewSelectStartLine(Edit->View); Scan <= Limit; Scan += 1)
			{
				char*					Line;

				Line = GetTextViewLine(Edit->View,Scan);
				if (Line == NIL)
					{
						return False;
					}
				if (PtrSize(Line) != 0)
					{
						char*					LineCopy;

						/* we only shift if the line has stuff on it. */
						LineCopy = InsertBlockIntoBlockCopy(Line,MyTab,0,1);
						if (LineCopy == NIL)
							{
								ReleasePtr(Line);
								return False;
							}
						SetTextEditLine(Edit,Scan,LineCopy);
						ReleasePtr(LineCopy);
					}
				ReleasePtr(Line);
			}
		if (TextEditIsThereValidSelection(Edit)
			&& (GetTextViewSelectEndCharPlusOne(Edit->View) == 0))
			{
				SetTextViewSelection(Edit->View,GetTextViewSelectStartLine(Edit->View),
					0,GetTextViewSelectEndLine(Edit->View),0);
			}
		 else
			{
				SetTextViewSelection(Edit->View,GetTextViewSelectStartLine(Edit->View),
					0,GetTextViewSelectEndLine(Edit->View) + 1,0);
			}
		TextEditRecalcHorizontalScroll(Edit);
		return True;
	}


/* convert all tab characters in the text box to the appropriate number of spaces */
/* if this fails, some of the lines may have been converted. */
MyBoolean					TextEditConvertTabsToSpaces(TextEditRec* Edit)
	{
		long						Scan;
		long						Limit;
		long						SelectionStartColumn;
		long						SelectionEndColumn;

		CheckPtrExistence(Edit);

		/* before we do it, we should remember the start and end columns of the insertion */
		/* point so that we can do this so that the selection screen does not change. */
		SelectionStartColumn = TextViewCalculateColumnFromCharIndex(Edit->View,
			GetTextViewSelectStartLine(Edit->View),GetTextViewSelectStartChar(Edit->View));
		SelectionEndColumn = TextViewCalculateColumnFromCharIndex(Edit->View,
			GetTextViewSelectEndLine(Edit->View),GetTextViewSelectEndCharPlusOne(Edit->View));

		/* do the conversion */
		Limit = GetTextViewNumLines(Edit->View);
		for (Scan = 0; Scan < Limit; Scan += 1)
			{
				char*						Line;
				MyBoolean				Flag;

				Line = GetTextViewSpaceFromTabLine(Edit->View,Scan);
				if (Line == NIL)
					{
						return False;
					}
				Flag = SetTextEditLine(Edit,Scan,Line);
				ReleasePtr(Line);
				if (!Flag)
					{
						return False;
					}
			}
		TextEditRecalcHorizontalScroll(Edit);

		/* restore apparent selection */
		SetTextViewSelection(Edit->View,GetTextViewSelectStartLine(Edit->View),
			SelectionStartColumn,GetTextViewSelectEndLine(Edit->View),SelectionEndColumn);

		/* exit successfully */
		return True;
	}


/* show the current selection in the edit window */
void							TextEditShowSelection(TextEditRec* Edit)
	{
		long			CheckValue;

		CheckPtrExistence(Edit);
		/* figure out how much space to leave at bottom and top edges */
		CheckValue = 4;
		while ((CheckValue > 0)
			&& (TextViewNumVisibleLines(Edit->View) < CheckValue * 2 + 4))
			{
				CheckValue -= 1;
			}
		/* vertical adjustment */
		if (((GetTextEditSelectStartLine(Edit) >= GetTextEditTopLine(Edit) + CheckValue)
				&& (GetTextEditSelectStartLine(Edit) < GetTextEditTopLine(Edit)
				+ (TextViewNumVisibleLines(Edit->View) - CheckValue - 1)))
			|| ((GetTextEditSelectStartLine(Edit) < CheckValue)
				&& (GetTextEditSelectStartLine(Edit) >= GetTextEditTopLine(Edit))))
			{
				/* beginning of selection is in the box, so try to center the end */
				if (GetTextEditSelectEndLine(Edit) < GetTextEditTopLine(Edit) + CheckValue)
					{
						/* selection is to far up */
						SetTextEditTopLine(Edit,GetTextEditSelectEndLine(Edit) - CheckValue);
					}
				if (GetTextEditSelectEndLine(Edit) >= GetTextEditTopLine(Edit)
					+ (TextViewNumVisibleLines(Edit->View) - CheckValue - 1))
					{
						/* selection is too far down */
						SetTextEditTopLine(Edit,GetTextEditSelectEndLine(Edit)
							- (TextViewNumVisibleLines(Edit->View) - CheckValue - 1) + 1);
					}
			}
		 else
			{
				/* center the beginning in the box */
				if (GetTextEditSelectStartLine(Edit) < GetTextEditTopLine(Edit) + CheckValue)
					{
						/* selection is to far up */
						SetTextEditTopLine(Edit,GetTextEditSelectStartLine(Edit) - CheckValue);
					}
				if (GetTextEditSelectStartLine(Edit) >= GetTextEditTopLine(Edit)
					+ (TextViewNumVisibleLines(Edit->View) - CheckValue - 1))
					{
						/* selection is too far down */
						SetTextEditTopLine(Edit,GetTextEditSelectStartLine(Edit)
							- (TextViewNumVisibleLines(Edit->View) - CheckValue - 1) + 1);
					}
			}
		/* figure out how much space on left and right edges to leave */
		CheckValue = 32;
		while ((CheckValue > 0)
			&& (GetTextViewWidth(Edit->View) < CheckValue * 4 + 16))
			{
				CheckValue -= 1;
			}
		/* horizontal adjustment */
		if (!TextEditIsThereValidSelection(Edit))
			{
				/* only adjust left-to-right if it's an insertion point */
				if (TextViewScreenXFromCharIndex(Edit->View,
					GetTextEditSelectStartLine(Edit),
					GetTextEditSelectStartChar(Edit))
					< GetTextEditPixelIndent(Edit) + CheckValue)
					{
						SetTextEditPixelIndent(Edit,
							TextViewScreenXFromCharIndex(Edit->View,
							GetTextEditSelectStartLine(Edit),
							GetTextEditSelectStartChar(Edit)) - (2 * CheckValue));
					}
				if (TextViewScreenXFromCharIndex(Edit->View,
					GetTextEditSelectStartLine(Edit),
					GetTextEditSelectStartChar(Edit))
					> GetTextEditPixelIndent(Edit)
					+ GetTextViewWidth(Edit->View) - CheckValue)
					{
						SetTextEditPixelIndent(Edit,
							TextViewScreenXFromCharIndex(Edit->View,
							GetTextEditSelectStartLine(Edit),
							GetTextEditSelectStartChar(Edit))
							- GetTextViewWidth(Edit->View) + (2 * CheckValue));
					}
			}
	}


/* handle a keypress for inserting or deleting into the text box */
void							TextEditDoKeyPressed(TextEditRec* Edit, char TheKey,
										ModifierFlags Modifiers)
	{
		MyBoolean			Extension;

		CheckPtrExistence(Edit);
		Extension = ((Modifiers & eShiftKey) != 0);
		switch ((unsigned char)TheKey)
			{
				case (unsigned char)13:  /* carriage return */
					if (TextViewIsThereValidSelection(Edit->View))
						{
							/* delete any existing section & restore insertion point */
							TextEditDoMenuClear(Edit);
						}
					TextViewBreakLine(Edit->View,GetTextEditSelectStartLine(Edit),
						GetTextEditSelectStartChar(Edit));
					SetTextEditInsertionPoint(Edit,
						GetTextEditSelectStartLine(Edit) + 1,0);
					TextEditKeyPressedUndoSave(Edit); /* record the new line in the undo info */
					if (Edit->AutoIndent)
						{
							char*			PreviousLine;
							long				CharScan;
							long				Limit;

							PreviousLine = GetTextEditLine(Edit,GetTextEditSelectStartLine(Edit) - 1);
							if (PreviousLine != NIL)
								{
									Limit = PtrSize(PreviousLine);
									CharScan = 0;
									while ((CharScan < Limit) && ((PreviousLine[CharScan] == 9)
										|| (PreviousLine[CharScan] == 32))) /* space or tab */
										{
											/* yo, this is recursive and could cause problems.  It would */
											/* be a BAD idea to call this if the char was a carriage return */
											TextEditDoKeyPressed(Edit,PreviousLine[CharScan],0);
											CharScan += 1;
										}
									ReleasePtr(PreviousLine);
								}
						}
					TextEditRecalcVerticalScroll(Edit);
					TextEditRecalcHorizontalScroll(Edit);
					break;
				case (unsigned char)8:  /* backspace key */
				case (unsigned char)127:  /* delete key */
					if (TextViewIsThereValidSelection(Edit->View))
						{
							/* delete any existing section & restore insertion point */
							TextEditDoMenuClear(Edit);
						}
					 else
						{
							if (GetTextEditSelectStartChar(Edit) == 0)
								{
									long				SelStartLine;

									/* delete carriage return */
									SelStartLine = GetTextEditSelectStartLine(Edit);
									if (SelStartLine > 0)
										{
											long				FirstLineLength;

											FirstLineLength = GetTextViewLineLength(Edit->View,
												SelStartLine - 1);
											TextViewFoldLines(Edit->View,SelStartLine - 1);
											SetTextEditInsertionPoint(Edit,SelStartLine - 1,FirstLineLength);
											TextEditRememberUndoDeletedCR(Edit);
											TextEditRecalcVerticalScroll(Edit);
											TextEditRecalcHorizontalScroll(Edit);
										}
									/* else, can't delete past start */
								}
							 else
								{
									char*					LineTemp;
									char*					LineCopy;

									LineTemp = GetTextEditLine(Edit,GetTextEditSelectStartLine(Edit));
									if (LineTemp != NIL)
										{
											char					WhatWeDeleted;

											WhatWeDeleted = LineTemp[GetTextEditSelectStartChar(Edit) - 1];
											LineCopy = RemoveBlockFromBlockCopy(LineTemp,
												GetTextEditSelectStartChar(Edit) - 1,1);
											if (LineCopy != NIL)
												{
													SetTextEditInsertionPoint(Edit,
														GetTextEditSelectStartLine(Edit),
														GetTextEditSelectStartChar(Edit) - 1);
													/* this is one of the few places where we can use */
													/* SetTextViewLine instead of SetTextEditLine.  We have */
													/* to since SetTextEditLine deletes the Undo information. */
													SetTextViewLine(Edit->View,
														GetTextEditSelectStartLine(Edit),LineCopy);
													TextEditRememberUndoDeletedChar(Edit,WhatWeDeleted);
													ReleasePtr(LineCopy);
												}
											ReleasePtr(LineTemp);
										}
									TextEditRecalcHorizontalScroll(Edit);
								}
						}
					break;
				case eLeftArrow:
					if ((Modifiers & eCommandKey) != 0)
						{
							if (!Extension)
								{
									SetTextEditInsertionPoint(Edit,
										GetTextEditSelectStartLine(Edit),0);
								}
							 else
								{
									SetTextEditSelection(Edit,GetTextEditSelectStartLine(Edit),0,
										GetTextEditSelectEndLine(Edit),
										GetTextEditSelectEndCharPlusOne(Edit));
								}
						}
					else if ((Modifiers & eOptionKey) != 0)
						{
							long				Line;
							long				Index;
							char*				LineTemp;

							Line = GetTextEditSelectStartLine(Edit);
							Index = GetTextEditSelectStartChar(Edit);
							LineTemp = GetTextEditLine(Edit,Line);
							if (LineTemp != NIL)
								{
									if (Index > 0)
										{
											while ((Index > 0) && !AlphaNum(LineTemp[Index - 1]))
												{
													/* skipping white space between cursor & previous word */
													Index -= 1;
												}
											while ((Index > 0) && AlphaNum(LineTemp[Index - 1]))
												{
													/* skipping over the word itself */
													Index -= 1;
												}
										}
									 else
										{
											if (Line > 0)
												{
													Line -= 1;
													Index = GetTextViewLineLength(Edit->View,Line);
												}
										}
									ReleasePtr(LineTemp);
								}
							if (!Extension)
								{
									SetTextEditInsertionPoint(Edit,Line,Index);
								}
							 else
								{
									SetTextEditSelection(Edit,Line,Index,
										GetTextEditSelectEndLine(Edit),
										GetTextEditSelectEndCharPlusOne(Edit));
								}
						}
					else
						{
							if (TextEditIsThereValidSelection(Edit) && !Extension)
								{
									SetTextEditInsertionPoint(Edit,
										GetTextEditSelectStartLine(Edit),GetTextEditSelectStartChar(Edit));
								}
							 else
								{
									if (GetTextEditSelectStartChar(Edit) == 0)
										{
											if (GetTextEditSelectStartLine(Edit) > 0)
												{
													if (!Extension)
														{
															SetTextEditInsertionPoint(Edit,
																GetTextEditSelectStartLine(Edit) - 1,
																GetTextViewLineLength(Edit->View,
																GetTextEditSelectStartLine(Edit) - 1));
														}
													 else
														{
															SetTextEditSelection(Edit,
																GetTextEditSelectStartLine(Edit) - 1,
																GetTextViewLineLength(Edit->View,
																	GetTextEditSelectStartLine(Edit) - 1),
																GetTextEditSelectEndLine(Edit),
																GetTextEditSelectEndCharPlusOne(Edit));
														}
												}
										}
									 else
										{
											if (!Extension)
												{
													SetTextEditInsertionPoint(Edit,
														GetTextEditSelectStartLine(Edit),
														GetTextEditSelectStartChar(Edit) - 1);
												}
											 else
												{
													SetTextEditSelection(Edit,
														GetTextEditSelectStartLine(Edit),
														GetTextEditSelectStartChar(Edit) - 1,
														GetTextEditSelectEndLine(Edit),
														GetTextEditSelectEndCharPlusOne(Edit));
												}
										}
								}
						}
					break;
				case eRightArrow:
					if ((Modifiers & eCommandKey) != 0)
						{
							if (!Extension)
								{
									SetTextEditInsertionPoint(Edit,GetTextEditSelectEndLine(Edit),
										GetTextViewLineLength(Edit->View,GetTextEditSelectEndLine(Edit)));
								}
							 else
								{
									SetTextEditSelection(Edit,GetTextEditSelectStartLine(Edit),
										GetTextEditSelectStartChar(Edit),GetTextEditSelectEndLine(Edit),
										GetTextViewLineLength(Edit->View,GetTextEditSelectEndLine(Edit)));
								}
						}
					else if ((Modifiers & eOptionKey) != 0)
						{
							long				Line;
							long				Index;
							long				Length;
							char*				LineTemp;

							Line = GetTextEditSelectEndLine(Edit);
							Index = GetTextEditSelectEndCharPlusOne(Edit);
							Length = GetTextViewLineLength(Edit->View,Line);
							LineTemp = GetTextEditLine(Edit,Line);
							if (LineTemp != NIL)
								{
									if (Index < Length)
										{
											while ((Index < Length) && !AlphaNum(LineTemp[Index]))
												{
													/* skipping white space between cursor & next word */
													Index += 1;
												}
											while ((Index < Length) && AlphaNum(LineTemp[Index]))
												{
													/* skipping over the word itself */
													Index += 1;
												}
										}
									 else
										{
											if (Line < GetTextViewNumLines(Edit->View) - 1)
												{
													Line += 1;
													Index = 0;
												}
										}
									ReleasePtr(LineTemp);
								}
							if (!Extension)
								{
									SetTextEditInsertionPoint(Edit,Line,Index);
								}
							 else
								{
									SetTextEditSelection(Edit,GetTextEditSelectStartLine(Edit),
										GetTextEditSelectStartChar(Edit),Line,Index);
								}
						}
					else
						{
							if (TextEditIsThereValidSelection(Edit) && !Extension)
								{
									SetTextEditInsertionPoint(Edit,GetTextEditSelectEndLine(Edit),
										GetTextEditSelectEndCharPlusOne(Edit));
								}
							 else
								{
									if (GetTextEditSelectEndCharPlusOne(Edit) == GetTextViewLineLength(
										Edit->View,GetTextEditSelectEndLine(Edit)))
										{
											if (GetTextEditSelectEndLine(Edit) <
												GetTextViewNumLines(Edit->View) - 1)
												{
													if (!Extension)
														{
															SetTextEditInsertionPoint(Edit,
																GetTextEditSelectEndLine(Edit) + 1,0);
														}
													 else
														{
															SetTextEditSelection(Edit,
																GetTextEditSelectStartLine(Edit),
																GetTextEditSelectStartChar(Edit),
																GetTextEditSelectEndLine(Edit) + 1,0);
														}
												}
										}
									 else
										{
											if (!Extension)
												{
													SetTextEditInsertionPoint(Edit,GetTextEditSelectEndLine(Edit),
														GetTextEditSelectEndCharPlusOne(Edit) + 1);
												}
											 else
												{
													SetTextEditSelection(Edit,GetTextEditSelectStartLine(Edit),
														GetTextEditSelectStartChar(Edit),
														GetTextEditSelectEndLine(Edit),
														GetTextEditSelectEndCharPlusOne(Edit) + 1);
												}
										}
								}
						}
					break;
				case eUpArrow:
					if ((Modifiers & eCommandKey) != 0)
						{
							if (!Extension)
								{
									SetTextEditInsertionPoint(Edit,0,0);
								}
							 else
								{
									SetTextEditSelection(Edit,0,0,GetTextEditSelectEndLine(Edit),
										GetTextEditSelectEndCharPlusOne(Edit));
								}
						}
					else if ((Modifiers & eOptionKey) != 0)
						{
							if (!Extension)
								{
									long						NewPosition;
									long						NewPoint;

									NewPosition = GetTextEditSelectStartLine(Edit)
										- TextViewNumVisibleLines(Edit->View) + 5;
									if (NewPosition < 0)
										{
											NewPosition = 0;
										}
									NewPoint = TextViewCharIndexFromScreenX(Edit->View,
										NewPosition, /* previous line */
										TextViewScreenXFromCharIndex(Edit->View,
										GetTextEditSelectStartLine(Edit), /* this line */
										GetTextEditSelectStartChar(Edit)));
									SetTextEditInsertionPoint(Edit,NewPosition,NewPoint);
								}
						}
					else
						{
							long						NewLineIndex;
							long						NewPoint;

							NewLineIndex = GetTextEditSelectStartLine(Edit) - 1;
							if (NewLineIndex < 0)
								{
									NewLineIndex = 0;
								}
							/* snap it to the closest point on the next line */
							NewPoint = TextViewCharIndexFromScreenX(Edit->View,
								NewLineIndex, /* previous line */
								TextViewScreenXFromCharIndex(Edit->View,
								GetTextEditSelectStartLine(Edit), /* this line */
								GetTextEditSelectStartChar(Edit)));
							if (!Extension)
								{
									SetTextEditInsertionPoint(Edit,NewLineIndex,NewPoint);
								}
							 else
								{
									SetTextEditSelection(Edit,NewLineIndex,NewPoint,
										GetTextEditSelectEndLine(Edit),
										GetTextEditSelectEndCharPlusOne(Edit));
								}
						}
					break;
				case eDownArrow:
					if ((Modifiers & eCommandKey) != 0)
						{
							if (!Extension)
								{
									SetTextEditInsertionPoint(Edit,GetTextViewNumLines(
										Edit->View) - 1,GetTextViewLineLength(Edit->View,
										GetTextViewNumLines(Edit->View) - 1));
								}
							 else
								{
									SetTextEditSelection(Edit,GetTextEditSelectStartLine(Edit),
										GetTextEditSelectStartChar(Edit),GetTextViewNumLines(
										Edit->View) - 1,GetTextViewLineLength(Edit->View,
										GetTextViewNumLines(Edit->View) - 1));
								}
						}
					else if ((Modifiers & eOptionKey) != 0)
						{
							if (!Extension)
								{
									long						NewPosition;
									long						NewPoint;

									NewPosition = GetTextEditSelectEndLine(Edit)
										+ TextViewNumVisibleLines(Edit->View) - 5;
									if (NewPosition > GetTextViewNumLines(Edit->View) - 1)
										{
											NewPosition = GetTextViewNumLines(Edit->View) - 1;
										}
									NewPoint = TextViewCharIndexFromScreenX(Edit->View,
										NewPosition, /* next line */
										TextViewScreenXFromCharIndex(Edit->View,
										GetTextEditSelectEndLine(Edit), /* this line */
										GetTextEditSelectEndCharPlusOne(Edit)));
									SetTextEditInsertionPoint(Edit,NewPosition,NewPoint);
								}
						}
					else
						{
							long						NewLineIndex;
							long						NewPoint;

							NewLineIndex = GetTextEditSelectEndLine(Edit) + 1;
							if (NewLineIndex > GetTextViewNumLines(Edit->View) - 1)
								{
									NewLineIndex = GetTextViewNumLines(Edit->View) - 1;
								}
							/* snap it to the closest point on the next line */
							NewPoint = TextViewCharIndexFromScreenX(Edit->View,
								NewLineIndex, /* next line */
								TextViewScreenXFromCharIndex(Edit->View,
								GetTextEditSelectEndLine(Edit), /* this line */
								GetTextEditSelectEndCharPlusOne(Edit)));
							if (!Extension)
								{
									SetTextEditInsertionPoint(Edit,NewLineIndex,NewPoint);
								}
							 else
								{
									SetTextEditSelection(Edit,GetTextEditSelectStartLine(Edit),
										GetTextEditSelectStartChar(Edit),NewLineIndex,NewPoint);
								}
						}
					break;
				default:  /* any other character */
					if ((Modifiers & eCommandKey) == 0)
						{
							char*					LineTemp;
							char					Buffer[1];
							char*					LineCopy;

							CheckPtrExistence(Edit);
							if (TextViewIsThereValidSelection(Edit->View))
								{
									/* delete any existing section & restore insertion point */
									TextEditDoMenuClear(Edit);
								}
							LineTemp = GetTextEditLine(Edit,GetTextEditSelectStartLine(Edit));
							if (LineTemp != NIL)
								{
									Buffer[0] = TheKey;
									LineCopy = InsertBlockIntoBlockCopy(LineTemp,Buffer,
										GetTextEditSelectStartChar(Edit),1);
									if (LineCopy != NIL)
										{
											/* this is one of the few places where we can use */
											/* SetTextViewLine instead of SetTextEditLine.  We have */
											/* to since SetTextEditLine deletes the Undo information. */
											SetTextViewLine(Edit->View,
												GetTextEditSelectStartLine(Edit),LineCopy);
											ReleasePtr(LineCopy);
										}
									SetTextEditInsertionPoint(Edit,GetTextEditSelectStartLine(Edit),
										GetTextEditSelectStartChar(Edit) + 1);
									ReleasePtr(LineTemp);
									TextEditKeyPressedUndoSave(Edit);
								}
						}
					break;
			}
		TextEditShowSelection(Edit); /* make sure insertion point is on screen */
	}


/* vertical scroll callback routine */
static void			TEVerticalScrollHook(long Parameter, ScrollType How,
									TextEditRec* TempScrollEdit)
	{
		CheckPtrExistence(TempScrollEdit);
		switch (How)
			{
				case eScrollToPosition:
					SetTextEditTopLine(TempScrollEdit,Parameter);
					break;
				case eScrollPageMinus:
					SetTextEditTopLine(TempScrollEdit,GetTextEditTopLine(TempScrollEdit)
						- (TextViewNumVisibleLines(TempScrollEdit->View) - 3));
					break;
				case eScrollPagePlus:
					SetTextEditTopLine(TempScrollEdit,GetTextEditTopLine(TempScrollEdit)
						+ (TextViewNumVisibleLines(TempScrollEdit->View) - 3));
					break;
				case eScrollLineMinus:
					SetTextEditTopLine(TempScrollEdit,GetTextEditTopLine(TempScrollEdit) - 1);
					break;
				case eScrollLinePlus:
					SetTextEditTopLine(TempScrollEdit,GetTextEditTopLine(TempScrollEdit) + 1);
					break;
				default:
					EXECUTE(PRERR(AllowResume,"TEVerticalScrollHook:  Unknown scroll opcode"));
			}
	}


/* horizontal scroll callback routine */
static void			TEHorizontalScrollHook(long Parameter, ScrollType How,
									TextEditRec* TempScrollEdit)
	{
		CheckPtrExistence(TempScrollEdit);
		switch (How)
			{
				case eScrollToPosition:
					SetTextEditPixelIndent(TempScrollEdit,Parameter);
					break;
				case eScrollPageMinus:
					SetTextEditPixelIndent(TempScrollEdit,GetTextEditPixelIndent(TempScrollEdit)
						- (GetTextViewWidth(TempScrollEdit->View) - 32));
					break;
				case eScrollPagePlus:
					SetTextEditPixelIndent(TempScrollEdit,GetTextEditPixelIndent(TempScrollEdit)
						+ (GetTextViewWidth(TempScrollEdit->View) - 32));
					break;
				case eScrollLineMinus:
					SetTextEditPixelIndent(TempScrollEdit,
						GetTextEditPixelIndent(TempScrollEdit) - 32);
					break;
				case eScrollLinePlus:
					SetTextEditPixelIndent(TempScrollEdit,
						GetTextEditPixelIndent(TempScrollEdit) + 32);
					break;
				default:
					EXECUTE(PRERR(AllowResume,"TEHorizontalScrollHook:  Unknown scroll opcode"));
			}
	}


/* handle a mouse-down in the text box */
void							TextEditDoMouseDown(TextEditRec* Edit, OrdType OrigX, OrdType OrigY,
										ModifierFlags Modifiers)
	{
		SelRec				BaseSelectionStart; /* where the mouse first hits, and if shift */
		SelRec				BaseSelectionEnd; /* is down, then it's the previous range */
		SelRec				CurrentMousePoint;
		SelRec				TempFirst;
		SelRec				TempLast;
		SelRec				OldSelectionStart;
		SelRec				OldSelectionEnd;
		OrdType				WhereX;
		OrdType				WhereY;
		OrdType				MouseLocationX;
		OrdType				MouseLocationY;

		CheckPtrExistence(Edit);
		WhereX = OrigX;
		WhereY = OrigY;
		if (Edit->VerticalScroll != NIL)
			{
				if (ScrollHitTest(Edit->VerticalScroll,WhereX,WhereY))
					{
						ScrollHitProc(Edit->VerticalScroll,Modifiers,WhereX,
							WhereY,Edit,(void (*)(long,ScrollType,void*))&TEVerticalScrollHook);
						return;
					}
			}
		if (Edit->HorizontalScroll != NIL)
			{
				if (ScrollHitTest(Edit->HorizontalScroll,WhereX,WhereY))
					{
						ScrollHitProc(Edit->HorizontalScroll,Modifiers,WhereX,
							WhereY,Edit,(void (*)(long,ScrollType,void*))&TEHorizontalScrollHook);
						return;
					}
			}
		WhereX -= GetTextViewXLoc(Edit->View);
		WhereY -= GetTextViewYLoc(Edit->View);

		if ((TimerDifference(ReadTimer(),Edit->LastClickTime) < GetDoubleClickInterval())
			&& ((WhereX - Edit->LastClickX <= 3) && (WhereX - Edit->LastClickX >= -3))
			&& ((WhereY - Edit->LastClickY <= 3) && (WhereY - Edit->LastClickY >= -3)))
			{
				switch (Edit->ClickPhase)
					{
						case eNoClick:
							Edit->ClickPhase = eSingleClick;
							break;
						case eSingleClick:
							Edit->ClickPhase = eDoubleClick;
							break;
						case eDoubleClick:
							Edit->ClickPhase = eTripleClick;
							break;
						case eTripleClick:
							/* no change */
							break;
					}
			}
		 else
			{
				Edit->ClickPhase = eSingleClick;
			}
		Edit->LastClickTime = ReadTimer();
		Edit->LastClickX = WhereX;
		Edit->LastClickY = WhereY;

		CurrentMousePoint.Line = WhereY / GetTextViewLineHeight(Edit->View)
			+ GetTextViewTopLine(Edit->View);
		CurrentMousePoint.Column = TextViewCharIndexFromScreenX(Edit->View,
			CurrentMousePoint.Line,WhereX + GetTextViewPixelIndent(Edit->View));
		if ((Modifiers & eShiftKey) != 0)
			{
				BaseSelectionStart.Line = GetTextEditSelectStartLine(Edit);
				BaseSelectionStart.Column = GetTextEditSelectStartChar(Edit);
				BaseSelectionEnd.Line = GetTextEditSelectEndLine(Edit);
				BaseSelectionEnd.Column = GetTextEditSelectEndCharPlusOne(Edit);
			}
		 else
			{
				if (TextEditIsThereValidSelection(Edit))
					{
						SelRec				Start,End;

						Start.Line = GetTextEditSelectStartLine(Edit);
						Start.Column = GetTextEditSelectStartChar(Edit);
						End.Line = GetTextEditSelectEndLine(Edit);
						End.Column = GetTextEditSelectEndCharPlusOne(Edit);
						SetTextEditInsertionPoint(Edit,Start.Line,Start.Column);
						TextViewRedrawRange(Edit->View,Start.Line,End.Line);
					}
				BaseSelectionStart = CurrentMousePoint;
				BaseSelectionEnd = CurrentMousePoint;
				ExtendSelection(Edit,&BaseSelectionStart,&BaseSelectionEnd);
				SetTextEditSelection(Edit,BaseSelectionStart.Line,BaseSelectionStart.Column,
					BaseSelectionEnd.Line,BaseSelectionEnd.Column);
			}

		while (eMouseUp != GetAnEvent(&MouseLocationX,&MouseLocationY,NIL,NIL,NIL,NIL))
			{
				WhereX = MouseLocationX - GetTextViewXLoc(Edit->View);
				WhereY = MouseLocationY - GetTextViewYLoc(Edit->View);
				if (WhereX < 0)
					{
						SetTextEditPixelIndent(Edit,GetTextEditPixelIndent(Edit) - 24);
					}
				if (WhereX > GetTextViewWidth(Edit->View))
					{
						SetTextEditPixelIndent(Edit,GetTextEditPixelIndent(Edit) + 24);
					}
				if (WhereY < 0)
					{
						SetTextEditTopLine(Edit,GetTextEditTopLine(Edit) - 1);
					}
				if (WhereY > GetTextViewHeight(Edit->View))
					{
						SetTextEditTopLine(Edit,GetTextEditTopLine(Edit) + 1);
					}

				if (WhereX < 0)
					{
						WhereX = 0;
					}
				if (WhereX > GetTextViewWidth(Edit->View) - 1)
					{
						WhereX = GetTextViewWidth(Edit->View) - 1;
					}
				if (WhereY < 0)
					{
						WhereY = 0;
					}
				if (WhereY > GetTextViewHeight(Edit->View) - 1)
					{
						WhereY = GetTextViewHeight(Edit->View) - 1;
					}
				CurrentMousePoint.Line = WhereY / GetTextViewLineHeight(Edit->View)
					+ GetTextViewTopLine(Edit->View);
				CurrentMousePoint.Column = TextViewCharIndexFromScreenX(Edit->View,
					CurrentMousePoint.Line,WhereX + GetTextViewPixelIndent(Edit->View));
				/* calculate what the extent of the current mouse selection should be */
				if (GreaterThan(&BaseSelectionStart,&CurrentMousePoint)
					|| ((BaseSelectionStart.Line == CurrentMousePoint.Line)
					&& (BaseSelectionStart.Column == CurrentMousePoint.Column)))
					{
						ExtendSelection(Edit,&CurrentMousePoint,NIL);
					}
				 else
					{
						ExtendSelection(Edit,NIL,&CurrentMousePoint);
					}
				/* calculating the total selection */
				UnionSelection(BaseSelectionStart,BaseSelectionEnd,CurrentMousePoint,
					&TempFirst,&TempLast);
				/* redrawing what has changed */
				OldSelectionStart.Line = GetTextEditSelectStartLine(Edit);
				OldSelectionStart.Column = GetTextEditSelectStartChar(Edit);
				OldSelectionEnd.Line = GetTextEditSelectEndLine(Edit);
				OldSelectionEnd.Column = GetTextEditSelectEndCharPlusOne(Edit);
				SetTextEditSelection(Edit,TempFirst.Line,TempFirst.Column,
					TempLast.Line,TempLast.Column);
			}
	}


/* this would be called from TextEditDoMenuClear, which would then call */
/* TextEditBlockRemoved to install the stuff that had been removed */
static void				TextEditPurgeUndoRecord(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		if (Edit->Undo.CanUndoFlag)
			{
				/* valid record needs purging */
				if (Edit->Undo.DeletedValidFlag)
					{
						/* delete the text thing */
						DisposeTextStorage(Edit->Undo.DeletedStuff);
					}
				Edit->Undo.CanUndoFlag = False;
			}
	}


/* something tricky to watch out for:  If you delete stuff, then insert stuff */
/* above it, the position that it was deleted from will be invalid.  You then */
/* have to exactly remove the stuff you inserted above BEFORE deleting when */
/* you actually perform the 'undo' operation */
static void				TextEditBlockRemovedUndoSave(TextEditRec* Edit,
										TextStorageRec* Stuff, long WhereLine, long WhereChar)
	{
		TextEditPurgeUndoRecord(Edit);
		Edit->Undo.CanUndoFlag = True;
		Edit->Undo.DeletedValidFlag = True;
		Edit->Undo.DeletedStuff = Stuff;
		Edit->Undo.DeletedLine = WhereLine;
		Edit->Undo.DeletedChar = WhereChar;
		Edit->Undo.ReplacingValidFlag = False; /* haven't started replacing yet */
	}


/* this routine is called when a key is pressed to update the information */
/* for the undo record.  If the insertion point is in a place indicating that */
/* the user did NOT move to another location, then it will be added to the record. */
/* otherwise the undo record will be purged and a new one started. */
static void				TextEditKeyPressedUndoSave(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		ERROR(TextEditIsThereValidSelection(Edit),PRERR(AllowResume,
			"TextEditKeyPressedUndoSave:  Why is there a valid selection?"));
		if (!Edit->Undo.CanUndoFlag)
			{
				Edit->Undo.CanUndoFlag = True;
				Edit->Undo.DeletedValidFlag = False;
				Edit->Undo.ReplacingValidFlag = False;
			}
		if (Edit->Undo.ReplacingValidFlag)
			{
				/* was the character added in the logical next position? */
				if ((Edit->Undo.ReplacingEndLine == GetTextEditSelectEndLine(Edit))
					&& (Edit->Undo.ReplacingEndCharPlusOne + 1
					== GetTextEditSelectEndCharPlusOne(Edit)))
					{
						/* yup, right after the last char */
						Edit->Undo.ReplacingEndCharPlusOne += 1; /* increment & all's cool */
					}
				 else
					{
						/* there's still hope.  Did they make a new line? */
						if ((Edit->Undo.ReplacingEndLine + 1 == GetTextEditSelectEndLine(Edit))
							&& (GetTextEditSelectEndCharPlusOne(Edit) == 0)
							&& (Edit->Undo.ReplacingEndCharPlusOne ==
							GetTextViewLineLength(Edit->View,Edit->Undo.ReplacingEndLine)))
							{
								/* stated in English:  If we're on the line right after the last */
								/* line in the added range, and we're at the beginning, and */
								/* the last char added before this was at the end of the previous */
								/* line, then the user probably hit return. */
								Edit->Undo.ReplacingEndCharPlusOne = 0;
								Edit->Undo.ReplacingEndLine += 1;
							}
						 else
							{
								/* otherwise we need to purge and reset. */
								TextEditPurgeUndoRecord(Edit);
								Edit->Undo.CanUndoFlag = True;
								Edit->Undo.DeletedValidFlag = False;
							 MakeNewRangePoint: /* we might just drop in some day */
								Edit->Undo.ReplacingValidFlag = True; /* only this happens */
								if (GetTextEditSelectStartChar(Edit) == 0)
									{
										ERROR(GetTextEditSelectStartLine(Edit) == 0,PRERR(AllowResume,
											"TextEditKeyPressedUndoSave:  Char "
											"inserted, but insertion point "
											"is still in the home position (0,0)."));
										Edit->Undo.ReplacingStartLine
											= GetTextEditSelectStartLine(Edit) - 1;
										Edit->Undo.ReplacingStartChar
											= GetTextViewLineLength(Edit->View,
											Edit->Undo.ReplacingStartLine);
									}
								 else
									{
										Edit->Undo.ReplacingStartChar
											= GetTextEditSelectStartChar(Edit) - 1;
										Edit->Undo.ReplacingStartLine = GetTextEditSelectStartLine(Edit);
									}
								Edit->Undo.ReplacingEndLine = GetTextEditSelectEndLine(Edit);
								Edit->Undo.ReplacingEndCharPlusOne
									= GetTextEditSelectEndCharPlusOne(Edit);
							}
					}
			}
		 else
			{
				/* else, there isn't even a range defined.  So define one. */
				goto MakeNewRangePoint;
			}
	}


/* undo the last operation that changed the contained data.  not all operations */
/* can be undone.  Who knows what the state of things will be if this fails. */
MyBoolean					TextEditDoMenuUndo(TextEditRec* Edit)
	{
		TextStorageRec*	OldAddedStuff;
		MyBoolean				OldStuffValid;
		long						OldAddedLine;
		long						OldAddedChar;
		MyBoolean				StuffWasInserted;

		CheckPtrExistence(Edit);
		if (Edit->Undo.CanUndoFlag)
			{
				OldStuffValid = Edit->Undo.ReplacingValidFlag;
				if (OldStuffValid)
					{
						SetTextEditSelection(Edit,Edit->Undo.ReplacingStartLine,
							Edit->Undo.ReplacingStartChar,Edit->Undo.ReplacingEndLine,
							Edit->Undo.ReplacingEndCharPlusOne);
						OldAddedStuff = TextViewGetSelection(Edit->View);
						if (OldAddedStuff == NIL)
							{
								return False; /* abort */
							}
						if (!TextViewDeleteSelection(Edit->View))
							{
								TextEditPurgeUndoRecord(Edit); /* avoid inconsistencies */
								return False;
							}
						/* save the place where we'd put this stuff back */
						OldAddedLine = GetTextEditSelectStartLine(Edit);
						OldAddedChar = GetTextEditSelectStartChar(Edit);
					}
				/* now the coordinates for the old deleted stuff are valid again */
				StuffWasInserted = Edit->Undo.DeletedValidFlag;
				if (StuffWasInserted)
					{
						SetTextEditInsertionPoint(Edit,Edit->Undo.DeletedLine,
							Edit->Undo.DeletedChar);
						if (!TextViewInsertBlock(Edit->View,Edit->Undo.DeletedStuff))
							{
								TextEditPurgeUndoRecord(Edit); /* avoid inconsistencies */
								return False;
							}
						/* this might have failed & nothing (or part) was inserted.  This */
						/* will end up selecting what was inserted, but the rest is lost. */
						SetTextEditSelection(Edit,Edit->Undo.DeletedLine,
							Edit->Undo.DeletedChar,GetTextEditSelectEndLine(Edit),
							GetTextEditSelectEndCharPlusOne(Edit));
						DisposeTextStorage(Edit->Undo.DeletedStuff);
					}
				/* now we have to pull a switcheroo */
				/* first, the deleted stuff becomes what we removed above */
				Edit->Undo.DeletedValidFlag = OldStuffValid;
				if (OldStuffValid)
					{
						Edit->Undo.DeletedLine = OldAddedLine;
						Edit->Undo.DeletedChar = OldAddedChar;
						Edit->Undo.DeletedStuff = OldAddedStuff;
					}
				/* now, update the inserted stuff */
				Edit->Undo.ReplacingValidFlag = StuffWasInserted;
				if (StuffWasInserted)
					{
						Edit->Undo.ReplacingStartLine = GetTextEditSelectStartLine(Edit);
						Edit->Undo.ReplacingStartChar = GetTextEditSelectStartChar(Edit);
						Edit->Undo.ReplacingEndLine = GetTextEditSelectEndLine(Edit);
						Edit->Undo.ReplacingEndCharPlusOne
							= GetTextEditSelectEndCharPlusOne(Edit);
					}
			}
		return True;
	}


/* this routine is called when the user has pressed delete and deleted */
/* a line break.  If this deletion is a logical extention of the existing */
/* deleted range, AND there is no insertion, then adjust the deleted range. */
/* if there is an insertion AND the deletion is of the last character of */
/* the insertion, then adjust the insertion.  If it is neither, purge the */
/* undo information and start a new deletion record. */
static void				TextEditRememberUndoDeletedCR(TextEditRec* Edit)
	{
		CheckPtrExistence(Edit);
		ERROR(TextEditIsThereValidSelection(Edit),PRERR(AllowResume,
			"TextEditRememberUndoDeletedCR:  Why is there a valid selection?"));
		if (!Edit->Undo.CanUndoFlag)
			{
				Edit->Undo.CanUndoFlag = True;
				Edit->Undo.DeletedValidFlag = False;
				Edit->Undo.ReplacingValidFlag = False;
			}
		if (Edit->Undo.ReplacingValidFlag)
			{
				/* valid replacement, but is this deletion part of it? */
				/* if it is, then the current insertion point will be on the line above */
				/* the last insertion line; the last insertion char will be 0. */
				if ((Edit->Undo.ReplacingEndLine - 1 == GetTextEditSelectStartLine(Edit))
					&& (Edit->Undo.ReplacingEndCharPlusOne == 0))
					{
						/* yup, just roll back the insertion range */
						/* but we have to make sure we aren't deleting past the beginning */
						/* of the insertion range. */
						Edit->Undo.ReplacingEndLine -= 1;
						Edit->Undo.ReplacingEndCharPlusOne = GetTextEditSelectStartChar(Edit);
						if ((Edit->Undo.ReplacingEndLine < Edit->Undo.ReplacingStartLine)
							|| ((Edit->Undo.ReplacingEndLine == Edit->Undo.ReplacingStartLine)
							&& (Edit->Undo.ReplacingEndCharPlusOne
							< Edit->Undo.ReplacingStartChar)))
							{
								/* we just deleted a character not in the range.  Start a new */
								/* deletion */
								goto DeletingSomewhereElsePoint;
							}
					}
				 else
					{
						TextStorageRec*				NewDeleter;

						/* otherwise, there was an insertion, but this isn't part of it, which */
						/* means the user moved the cursor before deleting.  Thus we start a */
						/* new deletion */
					 DeletingSomewhereElsePoint:
						TextEditPurgeUndoRecord(Edit);
						Edit->Undo.CanUndoFlag = True;
						Edit->Undo.DeletedValidFlag = True;
						Edit->Undo.ReplacingValidFlag = False;
						NewDeleter = NewTextStorage();
						if (NewDeleter == NIL)
							{
								/* woah, major error -- user just won't be able to undo! */
								TextEditPurgeUndoRecord(Edit);
								return;
							}
						Edit->Undo.DeletedStuff = NewDeleter;
						/* deleter should contain a new line */
						TextStorageInsertLine(NewDeleter,0);
						Edit->Undo.DeletedLine = GetTextEditSelectStartLine(Edit);
						Edit->Undo.DeletedChar = GetTextEditSelectStartChar(Edit);
					}
			}
		 else
			{
				/* there is no insertion, but is there a deletion? */
				if (Edit->Undo.DeletedValidFlag)
					{
						/* there's a deletion, but was this part of it? */
						/* to be part of it, the cursor has to be on the line before */
						/* the current deletion line and the deletion char has to be 0 */
						if ((Edit->Undo.DeletedLine - 1 == GetTextEditSelectStartLine(Edit))
							&& (Edit->Undo.DeletedChar == 0))
							{
								/* yup.  Just insert a blank line at the beginning of the */
								/* deletion thing and adjust the point.  If a memory out error */
								/* occurs, the data will be mangled -- too bad for the user */
								TextStorageInsertLine(Edit->Undo.DeletedStuff,0);
								Edit->Undo.DeletedLine -= 1;
								Edit->Undo.DeletedChar = GetTextEditSelectStartChar(Edit);
							}
						 else
							{
								/* nope, we are deleting somewhere else */
								goto DeletingSomewhereElsePoint;
							}
					}
				 else
					{
						/* no insertion and no deletion?  Well, let's just start one */
						/* on our own. */
						goto DeletingSomewhereElsePoint;
					}
			}
	}


/* this routine is called when the user has pressed delete and deleted */
/* a normal character.  If this deletion is a logical extention of the existing */
/* deleted range, AND there is no insertion, then adjust the deleted range. */
/* if there is an insertion AND the deletion is of the last character of */
/* the insertion, then adjust the insertion.  If it is neither, purge the */
/* undo information and start a new deletion record. */
static void				TextEditRememberUndoDeletedChar(TextEditRec* Edit, char What)
	{
		CheckPtrExistence(Edit);
		ERROR(TextEditIsThereValidSelection(Edit),PRERR(AllowResume,
			"TextEditRememberUndoDeletedChar:  Why is there a valid selection?"));
		if (!Edit->Undo.CanUndoFlag)
			{
				Edit->Undo.CanUndoFlag = True;
				Edit->Undo.DeletedValidFlag = False;
				Edit->Undo.ReplacingValidFlag = False;
			}
		if (Edit->Undo.ReplacingValidFlag)
			{
				/* valid replacement, but is this deletion part of it? */
				/* if it is, then the current insertion point will be on the same line */
				/* as the last insertion line, and the insertion char will be the last */
				/* insertion char - 1 */
				if ((Edit->Undo.ReplacingEndLine == GetTextEditSelectStartLine(Edit))
					&& (Edit->Undo.ReplacingEndCharPlusOne - 1
					== GetTextEditSelectStartChar(Edit)))
					{
						/* yup, just roll back the insertion range */
						Edit->Undo.ReplacingEndCharPlusOne -= 1;
						if ((Edit->Undo.ReplacingEndLine < Edit->Undo.ReplacingStartLine)
							|| ((Edit->Undo.ReplacingEndLine == Edit->Undo.ReplacingStartLine)
							&& (Edit->Undo.ReplacingEndCharPlusOne
							< Edit->Undo.ReplacingStartChar)))
							{
								/* we just deleted a character not in the range.  Start a new */
								/* deletion */
								goto DeletingSomewhereElsePoint;
							}
					}
				 else
					{
						TextStorageRec*		NewDeleter;
						char*							Line;
						char*							LineCopy;
						char							Buffer[1];

						/* otherwise, there was an insertion, but this isn't part of it, which */
						/* means the user moved the cursor before deleting.  Thus we start a */
						/* new deletion */
					 DeletingSomewhereElsePoint:
						TextEditPurgeUndoRecord(Edit);
						Edit->Undo.CanUndoFlag = True;
						Edit->Undo.DeletedValidFlag = True;
						Edit->Undo.ReplacingValidFlag = False;
						NewDeleter = NewTextStorage();
						if (NewDeleter == NIL)
							{
								/* woah, major error -- user just won't be able to undo! */
								TextEditPurgeUndoRecord(Edit);
								return;
							}
						Edit->Undo.DeletedStuff = NewDeleter;
						/* deleter should contain the character */
						Line = TextStorageGetLineCopy(Edit->Undo.DeletedStuff,0);
						if (Line != NIL)
							{
								Buffer[0] = What;
								LineCopy = InsertBlockIntoBlockCopy(Line,Buffer,0,1);
								if (LineCopy != NIL)
									{
										TextStorageChangeLine(Edit->Undo.DeletedStuff,0,LineCopy);
										ReleasePtr(LineCopy);
									}
								ReleasePtr(Line);
							}
						Edit->Undo.DeletedLine = GetTextEditSelectStartLine(Edit);
						Edit->Undo.DeletedChar = GetTextEditSelectStartChar(Edit);
					}
			}
		 else
			{
				/* there is no insertion, but is there a deletion? */
				if (Edit->Undo.DeletedValidFlag)
					{
						/* there's a deletion, but was this part of it? */
						/* to be part of it, the cursor has to be on the same line as */
						/* the current deletion line and the deletion char has to be */
						/* the cursor char + 1 */
						if ((Edit->Undo.DeletedLine == GetTextEditSelectStartLine(Edit))
							&& (Edit->Undo.DeletedChar - 1 == GetTextEditSelectStartChar(Edit)))
							{
								char*					Line;
								char*					LineCopy;
								char					Buffer[1];

								/* yup.  Just insert the character at the beginning of the */
								/* deletion thing and adjust the point. */
								Line = TextStorageGetLineCopy(Edit->Undo.DeletedStuff,0);
								if (Line != NIL)
									{
										Buffer[0] = What;
										LineCopy = InsertBlockIntoBlockCopy(Line,Buffer,0,1);
										if (LineCopy != NIL)
											{
												TextStorageChangeLine(Edit->Undo.DeletedStuff,0,LineCopy);
												ReleasePtr(LineCopy);
											}
										ReleasePtr(Line);
									}
								Edit->Undo.DeletedChar -= 1;
							}
						 else
							{
								/* nope, we are deleting somewhere else */
								goto DeletingSomewhereElsePoint;
							}
					}
				 else
					{
						/* no insertion and no deletion?  Well, let's just start one */
						/* on our own. */
						goto DeletingSomewhereElsePoint;
					}
			}
	}


#define OpenParen '('
#define CloseParen ')'
#define OpenBrace '{'
#define CloseBrace '}'
#define OpenBracket '['
#define CloseBracket ']'
#define MaxStackSize (1024)


/* extend the current selection to show balanced parentheses, or beep if */
/* the parentheses are not balanced */
void							TextEditBalanceParens(TextEditRec* Edit)
	{
		char					Form;
		char					Stack[MaxStackSize];
		long					StackIndex;
		long					BackLine;
		long					BackChar;
		long					ForwardLine;
		long					ForwardChar;
		char*					Line;

		CheckPtrExistence(Edit);
		BackLine = GetTextEditSelectStartLine(Edit);
		BackChar = GetTextEditSelectStartChar(Edit);
		ForwardLine = GetTextEditSelectEndLine(Edit);
		ForwardChar = GetTextEditSelectEndCharPlusOne(Edit);
		if ((BackLine == ForwardLine) && (BackChar == ForwardChar))
			{
				/* just an insertion point.  In this case, if it's like this: */
				/* (...)|  or like this:  |(...), then the group immediately */
				/* next to the insertion point should be selected */
				Line = GetTextEditLine(Edit,BackLine);
				if (Line == NIL)
					{
						return;
					}
				if (BackChar > 0)
					{
						if ((Line[BackChar - 1] == CloseParen)
							|| (Line[BackChar - 1] == CloseBrace)
							|| (Line[BackChar - 1] == CloseBracket))
							{
								/* move insertion point left */
								BackChar -= 1;
								ForwardChar -= 1;
								goto InitialSetupSkipOutPoint;
							}
					}
				if (BackChar < PtrSize(Line)/*no -1*/)
					{
						/* notice we don't use BackChar + 1 here, because the insertion point */
						/* is BETWEEN two characters (the x-1 and the x character) */
						if ((Line[BackChar] == OpenParen)
							|| (Line[BackChar] == OpenBrace)
							|| (Line[BackChar] == OpenBracket))
							{
								/* move insertion point right */
								BackChar += 1;
								ForwardChar += 1;
								goto InitialSetupSkipOutPoint;
								/* BackChar and ForwardChar could be equal to PtrSize(Line) */
								/* after this. */
							}
					}
				/* jump here when the insertion point has been adjusted */
			 InitialSetupSkipOutPoint:
				ReleasePtr(Line);
			}
		StackIndex = 0;
		while (BackLine >= 0)
			{
				Line = GetTextEditLine(Edit,BackLine);
				if (Line == NIL)
					{
						return;
					}
				while (BackChar > 0)
					{
						char					C;

						BackChar -= 1;
						PRNGCHK(Line,&(Line[BackChar]),sizeof(char));
						C = Line[BackChar];
						if ((C == CloseParen) || (C == CloseBrace) || (C == CloseBracket))
							{
								/* we ran into the trailing end of a grouping, so we increment */
								/* the count and look for the beginning end. */
								Stack[StackIndex] = C;
								StackIndex += 1;
								if (StackIndex >= MaxStackSize)
									{
										/* expression is too complex to be analyzed */
										ErrorBeep();
										ReleasePtr(Line);
										return;
									}
							}
						else if ((C == OpenParen) || (C == OpenBrace) || (C == OpenBracket))
							{
								/* here we found a beginning end of some sort.  If it's the */
								/* beginning of a group we aren't in, then check to see that */
								/* it matches */
								if (StackIndex == 0)
									{
										/* there are no other blocks we had to go through so this */
										/* begin must enclose us */
										Form = C;
										ReleasePtr(Line);
										goto ForwardScanEntryPoint;
									}
								StackIndex -= 1;
								if (((C == OpenParen) && (Stack[StackIndex] == CloseParen))
									|| ((C == OpenBrace) && (Stack[StackIndex] == CloseBrace))
									|| ((C == OpenBracket) && (Stack[StackIndex] == CloseBracket)))
									{
										/* good */
									}
								 else
									{
										/* bad */
										ReleasePtr(Line);
										ErrorBeep();
										return;
									}
							}
					}
				BackLine -= 1;
				ReleasePtr(Line);
				if (BackLine >= 0)
					{
						Line = GetTextEditLine(Edit,BackLine);
						BackChar = PtrSize(Line);
						ReleasePtr(Line);
					}
			}
		ErrorBeep();
		return;
	 ForwardScanEntryPoint:
		StackIndex = 0;
		while (ForwardLine < GetTextEditNumLines(Edit))
			{
				Line = GetTextEditLine(Edit,ForwardLine);
				if (Line == NIL)
					{
						return;
					}
				while (ForwardChar < PtrSize(Line))
					{
						char					C;

						PRNGCHK(Line,&(Line[ForwardChar]),sizeof(char));
						C = Line[ForwardChar];
						ForwardChar += 1;
						if ((C == OpenParen) || (C == OpenBrace) || (C == OpenBracket))
							{
								/* we ran into the leading end of a grouping, so we increment */
								/* the count and look for the end end. */
								Stack[StackIndex] = C;
								StackIndex += 1;
								if (StackIndex >= MaxStackSize)
									{
										/* expression is too complex to be analyzed */
										ErrorBeep();
										ReleasePtr(Line);
										return;
									}
							}
						else if ((C == CloseParen) || (C == CloseBrace) || (C == CloseBracket))
							{
								/* here we found an end of some sort.  If it's the */
								/* end of a group we aren't in, then check to see that */
								/* it matches */
								if (StackIndex == 0)
									{
										/* there are no other blocks we had to go through so this */
										/* end must enclose us */
										ReleasePtr(Line);
										if (((Form == OpenParen) && (C == CloseParen))
											|| ((Form == OpenBrace) && (C == CloseBrace))
											|| ((Form == OpenBracket) && (C == CloseBracket)))
											{
												SetTextEditSelection(Edit,BackLine,BackChar,
													ForwardLine,ForwardChar);
												TextEditShowSelection(Edit);
												return;
											}
										 else
											{
												ErrorBeep();
												return;
											}
									}
								StackIndex -= 1;
								if (((C == CloseParen) && (Stack[StackIndex] == OpenParen))
									|| ((C == CloseBrace) && (Stack[StackIndex] == OpenBrace))
									|| ((C == CloseBracket) && (Stack[StackIndex] == OpenBracket)))
									{
										/* good */
									}
								 else
									{
										/* bad */
										ReleasePtr(Line);
										ErrorBeep();
										return;
									}
							}
					}
				ForwardLine += 1;
				ReleasePtr(Line);
				ForwardChar = 0;
			}
		ErrorBeep();
		return;
	}


/* find the specified search string starting at the current selection. */
MyBoolean					TextEditFindAgain(TextEditRec* Edit, char* SearchString)
	{
		long						LineScan;
		long						KeyLength;
		long						LineLimit;
		long						ColumnScan;
		long						ElapsedLineCount;

		CheckPtrExistence(Edit);
		CheckPtrExistence(SearchString);
		KeyLength = PtrSize(SearchString);
		LineLimit = GetTextEditNumLines(Edit);
		LineScan = GetTextEditSelectStartLine(Edit);
		ElapsedLineCount = 0;
		if (TextEditIsThereValidSelection(Edit))
			{
				/* if there is a selection, assume it's from a previous search.  we need */
				/* to have + 1 so we don't find what we found again. */
				ColumnScan = GetTextEditSelectStartChar(Edit) + 1;
			}
		 else
			{
				/* if no selection, start search at current position.  this lets us find */
				/* patterns at the very beginning of the file. */
				ColumnScan = GetTextEditSelectStartChar(Edit);
			}
		while (LineScan < LineLimit)
			{
				char*						TestLine;
				long						ColumnLimit;

				TestLine = GetTextEditLine(Edit,LineScan);
				if (TestLine == NIL)
					{
						return False;
					}
				ColumnLimit = PtrSize(TestLine) - KeyLength;
				while (ColumnScan <= ColumnLimit)
					{
						if (MemEquNoCase(&(SearchString[0]),&(TestLine[ColumnScan]),KeyLength))
							{
								/* found it! */
								SetTextEditSelection(Edit,LineScan,ColumnScan,
									LineScan,ColumnScan + KeyLength);
								TextEditShowSelection(Edit);
								ReleasePtr(TestLine);
								return True;
							}
						ColumnScan += 1;
					}
				ReleasePtr(TestLine);
				LineScan += 1;
				ElapsedLineCount += 1;
				ColumnScan = 0;
				if (RelinquishCPUJudiciouslyCheckCancel())
					{
						/* cancelling */
						return False;
					}
			}
		ErrorBeep(); /* selection not found */
		return False;
	}


/* see if the specified location is in the text edit box */
MyBoolean					TextEditHitTest(TextEditRec* Edit, OrdType X, OrdType Y)
	{
		CheckPtrExistence(Edit);
		return (X >= Edit->X) && (Y >= Edit->Y) && (X < Edit->X + Edit->TotalWidth)
			&& (Y < Edit->Y + Edit->TotalHeight);
	}


/* see if the specified location is in the text edit area of the box (not the */
/* scrollbars).  This is used for deciding whether the mouse should be an Ibeam */
/* or an arrow. */
MyBoolean					TextEditIBeamTest(TextEditRec* Edit, OrdType X, OrdType Y)
	{
		CheckPtrExistence(Edit);
		return (X >= Edit->X) && (Y >= Edit->Y)
			&& (X < Edit->X + GetTextViewWidth(Edit->View))
			&& (Y < Edit->Y + GetTextViewHeight(Edit->View));
	}
