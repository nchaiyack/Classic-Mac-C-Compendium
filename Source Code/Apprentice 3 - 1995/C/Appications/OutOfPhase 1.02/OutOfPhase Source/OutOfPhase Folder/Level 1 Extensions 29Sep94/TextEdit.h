/* TextEdit.h */

#ifndef Included_TextEdit_h
#define Included_TextEdit_h

/* TextEdit module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* Scroll */
/* TextView */
/* EventLoop */
/* Scrap */
/* TextStorage */
/* DataMunging */
/* EventLoop */

#include "Screen.h"
#include "EventLoop.h"

struct TextEditRec;
typedef struct TextEditRec TextEditRec;

/* to avoid dragging Files in */
struct FileType;

/* what kind of scrolling should be supported by our text editing object */
typedef enum {eTENoScrollBars = 0, eTEVScrollBar = 1, eTEHScrollBar = 2} TEScrollType;

/* for selecting ranges of text */
typedef struct
	{
		long				Line;
		long				Column;
	} SelRec;

/* create a new, empty text edit */
TextEditRec*			NewTextEdit(WinType* Window, TEScrollType ScrollStuff,
										FontType FontID, FontSizeType FontSize, OrdType X, OrdType Y,
										OrdType Width, OrdType Height);

/* dispose text edit and all text it contains */
void							DisposeTextEdit(TextEditRec* Edit);


/* get location of text edit */
OrdType						GetTextEditXLoc(TextEditRec* Edit);
OrdType						GetTextEditYLoc(TextEditRec* Edit);
OrdType						GetTextEditWidth(TextEditRec* Edit);
OrdType						GetTextEditHeight(TextEditRec* Edit);

/* get font stuff for text edit */
FontType					GetTextEditFont(TextEditRec* Edit);
FontSizeType			GetTextEditPointSize(TextEditRec* Edit);

/* get the number of spaces per tab character */
long							GetTextEditSpacesPerTab(TextEditRec* Edit);

/* get the index of the top line in the window */
long							GetTextEditTopLine(TextEditRec* Edit);

/* get the pixel index of the leftmost text of the text box */
OrdType						GetTextEditPixelIndent(TextEditRec* Edit);

/* returns True if the selection is non-empty, or false if it's an insertion point */
MyBoolean					TextEditIsThereValidSelection(TextEditRec* Edit);

/* get the line number of the start of the selection */
long							GetTextEditSelectStartLine(TextEditRec* Edit);

/* get the line number of the end of the selection */
long							GetTextEditSelectEndLine(TextEditRec* Edit);

/* get the character index of the start of the selection */
long							GetTextEditSelectStartChar(TextEditRec* Edit);

/* get the character index of the character immediately after the end of the */
/* selection.  (if this == start char and startline == endline, then there is no */
/* space between them and therefore there is no selection */
long							GetTextEditSelectEndCharPlusOne(TextEditRec* Edit);

/* find out if selection & scrollbar display is enabled */
MyBoolean					TextEditIsShowSelectionEnabled(TextEditRec* Edit);

/* find out if the data has been modified since the last call to TextEditHasBeenSaved */
MyBoolean					TextEditDoesItNeedToBeSaved(TextEditRec* Edit);

/* get the total number of lines contained in the edit */
long							GetTextEditNumLines(TextEditRec* Edit);

/* find out if auto-indent upon newline is enabled */
MyBoolean					TextEditIsAutoIndentEnabled(TextEditRec* Edit);

/* find out if it is possible to undo the last operation (for enabling menu item) */
MyBoolean					TextEditCanWeUndo(TextEditRec* Edit);


/* change the screen location of the text edit box */
void							SetTextEditPosition(TextEditRec* Edit, OrdType X, OrdType Y,
										OrdType Width, OrdType Height);

/* change the font being used to display the text */
void							SetTextEditFontStuff(TextEditRec* Edit, FontType Font,
										FontSizeType Size);

/* set the number of spaces displayed for a tab */
void							SetTextEditTabSize(TextEditRec* Edit, long SpacesPerTab);

/* change the top line being displayed in the exit box */
void							SetTextEditTopLine(TextEditRec* Edit, long NewTopLine);

/* change the pixel index of the left edge of the text box */
void							SetTextEditPixelIndent(TextEditRec* Edit, OrdType NewPixelIndent);

/* set the selection to a specified range */
void							SetTextEditSelection(TextEditRec* Edit, long StartLine,
										long StartChar, long EndLine, long EndCharPlusOne);

/* set the selection to an insertion point at the specified position */
void							SetTextEditInsertionPoint(TextEditRec* Edit, long Line, long Char);

/* enable display of selection and scrollbars */
void							EnableTextEditSelection(TextEditRec* Edit);

/* disable display of selection and scrollbars */
void							DisableTextEditSelection(TextEditRec* Edit);

/* indicate that any data in the text edit has been saved.  After this call, */
/* TextEditDoesItNeedToBeSaved will return False.  It will start returning true */
/* if any subsequent changes are made. */
void							TextEditHasBeenSaved(TextEditRec* Edit);

/* enable or disable auto-indent on carriage return */
void							SetTextEditAutoIndent(TextEditRec* Edit, MyBoolean AutoIndentFlag);


/* recalculate the position index of the vertical scrollbar */
void							TextEditRecalcVerticalScroll(TextEditRec* Edit);

/* recalculate the position index of the horizontal scrollbar */
void							TextEditRecalcHorizontalScroll(TextEditRec* Edit);

/* redraw the entire text edit box */
void							TextEditFullRedraw(TextEditRec* Edit);

/* redraw the outline frame of the text edit box */
void							TextEditRedrawFrame(TextEditRec* Edit);

/* update cursor.  This should be called during idle events.  It keeps track of */
/* when the cursor was last blinked and blinks the cursor again if necessary. */
void							TextEditUpdateCursor(TextEditRec* Edit);


/* get the specified line of text from the exit */
char*							GetTextEditLine(TextEditRec* Edit, long LineIndex);

/* get a line of text, but first convert all tabs in the line into the */
/* proper number of spaces. */
char*							GetTextEditSpaceFromTabLine(TextEditRec* Edit, long LineIndex);

/* put a new line in the text box.  This overwrites data already on that line */
MyBoolean					SetTextEditLine(TextEditRec* Edit, long LineIndex, char* LineToCopy);

/* use the LineFeed string to create a single block of text containing all */
/* of the lines packed into it */
char*							TextEditGetRawData(TextEditRec* Edit, char* LineFeed);

/* put new data into the text edit.  The RawData is a block with all text lines */
/* packed into it separated by the LineFeed string. */
MyBoolean					TextEditNewRawData(TextEditRec* Edit, char* RawData, char* LineFeed);

/* get a text block containing the selected data */
char*							TextEditGetSelection(TextEditRec* Edit);

/* replace the current selection (if any) with the specified raw data block. */
/* if this fails, some of the data may have been inserted */
MyBoolean					TextEditInsertRawData(TextEditRec* Edit, char* RawData, char* LineFeed);


/* find the union of two selection ranges */
void							UnionSelection(SelRec One, SelRec Two, SelRec Three,
										SelRec* Start, SelRec* End);

/* find the difference (union - intersection) of two selection ranges.  This is */
/* used to avoid redrawing the entire selection range all the time */
void							DiffSelection(SelRec OneStart, SelRec OneEnd, SelRec TwoStart,
										SelRec TwoEnd, SelRec* OutStart, SelRec* OutEnd);

/* if the first selection point is after the second then reverse their order */
void							SortSelection(SelRec* One, SelRec* Two);

/* returns True if the first selection point is after the second one */
MyBoolean					GreaterThan(SelRec* One, SelRec* Two);

/* extend the selection using the current mouse-click state (single, double, triple) */
void							ExtendSelection(TextEditRec* Edit, SelRec* Start, SelRec* End);

/* find out if the character is an alphanumeric character.  this is used by */
/* ExtendSelection for figuring out where double-click extends should stop. */
MyBoolean					AlphaNum(char It);

/* append a line of text to the end of the text edit.  This can be used if the */
/* text edit box is being used as an interaction (terminal) window */
/* if NIL is passed in for Data, a blank line will be appended */
MyBoolean					TextEditAppendLineInteraction(TextEditRec* Edit, char* Data);

/* dump the data contained in the text edit to the current position in the */
/* specified file.  returns True if all the data was written successfully */
MyBoolean					TestEditWriteDataToFile(TextEditRec* Edit,
										struct FileType* FileRefNum, char* EOLN);


/* cut the selected data to the clipboard.  if this fails, some of the data */
/* may have been deleted */
MyBoolean					TextEditDoMenuCut(TextEditRec* Edit);

/* copy the selected data to the clipboard. */
MyBoolean					TextEditDoMenuCopy(TextEditRec* Edit);

/* paste the clipboard in, replacing the current selection if there is one */
/* if this fails, some of the data may have been inserted */
MyBoolean					TextEditDoMenuPaste(TextEditRec* Edit);

/* select the entire data area of the text edit */
void							TextEditDoMenuSelectAll(TextEditRec* Edit);

/* delete the selected area.  This is the same as pressing the delete key */
/* when there is a valid selection.  if this fails, some of the data may */
/* have been deleted */
MyBoolean					TextEditDoMenuClear(TextEditRec* Edit);

/* shift the selection toward the left margin by deleting one tab (or spaces) */
/* from the beginning of the line.  It will not remove non-whitespace characters */
/* if this fails, some of the lines may have been shifted */
MyBoolean					TextEditShiftSelectionLeftOneTab(TextEditRec* Edit);

/* shift selection toward the right margin by inserting a tab at the */
/* beginning of each line.  if this fails, some of the lines may have been shifted. */
MyBoolean					TextEditShiftSelectionRightOneTab(TextEditRec* Edit);

/* convert all tab characters in the text box to the appropriate number of spaces */
/* if this fails, some of the lines may have been converted. */
MyBoolean					TextEditConvertTabsToSpaces(TextEditRec* Edit);

/* show the current selection in the edit window */
void							TextEditShowSelection(TextEditRec* Edit);

/* show the starting edge of the selection. */
void							TextEditShowSelectionStartEdge(TextEditRec* Edit);

/* show the ending edge of the selection. */
void							TextEditShowSelectionEndEdge(TextEditRec* Edit);

/* handle a keypress for inserting or deleting into the text box */
void							TextEditDoKeyPressed(TextEditRec* Edit, char TheKey,
										ModifierFlags Modifiers);

/* handle a mouse-down in the text box */
void							TextEditDoMouseDown(TextEditRec* Edit, OrdType OrigX, OrdType OrigY,
										ModifierFlags Modifiers);

/* undo the last operation that changed the contained data.  not all operations */
/* can be undone.  Who knows what the state of things will be if this fails. */
MyBoolean					TextEditDoMenuUndo(TextEditRec* Edit);

/* extend the current selection to show balanced parentheses, or beep if */
/* the parentheses are not balanced */
void							TextEditBalanceParens(TextEditRec* Edit);

/* find the specified search string starting at the current selection. */
MyBoolean					TextEditFindAgain(TextEditRec* Edit, char* SearchString);

/* see if the specified location is in the text edit box */
MyBoolean					TextEditHitTest(TextEditRec* Edit, OrdType X, OrdType Y);

/* see if the specified location is in the text edit area of the box (not the */
/* scrollbars).  This is used for deciding whether the mouse should be an Ibeam */
/* or an arrow. */
MyBoolean					TextEditIBeamTest(TextEditRec* Edit, OrdType X, OrdType Y);

#endif
