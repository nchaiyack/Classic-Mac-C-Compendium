/* TextView.h */

#ifndef Included_TextView_h
#define Included_TextView_h

/* TextView module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* TextStorage */
/* EventLoop */
/* Memory */
/* Screen */

#include "Screen.h"

struct TextViewRec;
typedef struct TextViewRec TextViewRec;

/* so we can avoid dragging include files into the header */
struct FileType;
struct TextStorageRec;

/* create a new text view object */
TextViewRec*			NewTextView(WinType* Window, OrdType X, OrdType Y,
										OrdType Width, OrdType Height, long SpacesPerTab,
										FontType Font, FontSizeType PointSize);

/* dispose the text view object and all data it contains */
void							DisposeTextView(TextViewRec* View);


/* find out where the text view object is located on the screen */
OrdType						GetTextViewXLoc(TextViewRec* View);
OrdType						GetTextViewYLoc(TextViewRec* View);
OrdType						GetTextViewWidth(TextViewRec* View);
OrdType						GetTextViewHeight(TextViewRec* View);

/* find out what font the text is being displayed in */
FontType					GetTextViewFont(TextViewRec* View);
FontSizeType			GetTextViewPointSize(TextViewRec* View);

/* find out how many spaces are displayed for each tab */
long							GetTextViewSpacesPerTab(TextViewRec* View);

/* find out what the index of the top line on the display is */
long							GetTextViewTopLine(TextViewRec* View);

/* find out what the pixel position of the leftmost character displayed is */
OrdType						GetTextViewPixelIndent(TextViewRec* View);

/* returns True if the selection contains more than 0 characters */
MyBoolean					TextViewIsThereValidSelection(TextViewRec* View);

/* get the index of the starting line of the selection */
long							GetTextViewSelectStartLine(TextViewRec* View);

/* get the index of the last line of the selection */
long							GetTextViewSelectEndLine(TextViewRec* View);

/* get the index of the starting character of the selection */
long							GetTextViewSelectStartChar(TextViewRec* View);

/* get the index of the character after the end of the selection.  if the starting */
/* line == ending line, and starting char == last char + 1, then there is no */
/* selection, but instead an insertion point */
long							GetTextViewSelectEndCharPlusOne(TextViewRec* View);

/* find out if display of inverted selection text is enabled */
MyBoolean					TextViewIsShowSelectionEnabled(TextViewRec* View);

/* find out if any changes have been made to the text view object since */
/* the last call to TextViewHasBeenSaved */
MyBoolean					TextViewDoesItNeedToBeSaved(TextViewRec* View);

/* see if the specified position is within the text view box */
MyBoolean					TextViewHitTest(TextViewRec* View, OrdType X, OrdType Y);

/* find out the total number of lines in the text view box */
long							GetTextViewNumLines(TextViewRec* View);

/* find out how many characters the specified line contains */
long							GetTextViewLineLength(TextViewRec* View, long LineIndex);

/* get the height of a line of text (font height) */
OrdType						GetTextViewLineHeight(TextViewRec* View);


/* change where in the window the text view box is located */
void							SetTextViewPosition(TextViewRec* View, OrdType X, OrdType Y,
										OrdType Width, OrdType Height);

/* change the font being used to display the text */
void							SetTextViewFontStuff(TextViewRec* View, FontType Font,
										FontSizeType Size);

/* change the number of spaces displayed for each tab */
void							SetTextViewTabSize(TextViewRec* View, long SpacesPerTab);

/* set the index of the top line being displayed in the text box */
void							SetTextViewTopLine(TextViewRec* View, long NewTopLine);

/* set the index of the leftmost character in the text edit box */
void							SetTextViewPixelIndent(TextViewRec* View, OrdType NewPixelIndent);

/* set the selection to the specified area */
void							SetTextViewSelection(TextViewRec* View, long StartLine,
										long StartChar, long EndLine, long EndCharPlusOne);

/* set the selection to an insertion point at the specified position */
void							SetTextViewInsertionPoint(TextViewRec* View, long Line, long Char);

/* enable the display of inverted selected text */
void							EnableTextViewSelection(TextViewRec* View);

/* disable the display of inverted selected text */
void							DisableTextViewSelection(TextViewRec* View);

/* indicate that any changes to the text have been recognized */
void							TextViewHasBeenSaved(TextViewRec* View);

/* break the specified line at the specified character index.  this is used */
/* for when they hit carriage return in the middle of a line */
MyBoolean					TextViewBreakLine(TextViewRec* View, long LineIndex, long CharIndex);

/* concatenate the line after the specified line onto the end of the */
/* specified line.  Used for hitting delete at the beginning of a line */
MyBoolean					TextViewFoldLines(TextViewRec* View, long LineIndex);


/* do a complete redraw of the text view box */
void							TextViewFullRedraw(TextViewRec* View);

/* redraw the specified line */
void							TextViewRedrawLine(TextViewRec* View, long LineIndex);

/* redraw a series of lines */
void							TextViewRedrawRange(TextViewRec* View, long StartLine, long EndLine);

/* update the cursor.  this should be called every time there is an idle */
/* event.  periodically, it will blink the insertion point state (if there */
/* is no valid selection) */
void							TextViewUpdateCursor(TextViewRec* View);

/* find out the pixel index of the left edge of the specified character */
OrdType						TextViewScreenXFromCharIndex(TextViewRec* View, long LineIndex,
										long CharIndex);

/* convert a pixel position into the nearest character */
long							TextViewCharIndexFromScreenX(TextViewRec* View, long LineIndex,
										OrdType ScreenX);

/* find out how many lines are visible on the screen */
long							TextViewNumVisibleLines(TextViewRec* View);

/* get the width of the horizontal scrollable area */
OrdType						TextViewGetVirtualWindowWidth(TextViewRec* View);


/* get a copy of the specified line */
char*							GetTextViewLine(TextViewRec* View, long LineIndex);

/* get a copy of the specified line, but first convert all tabs to the */
/* appropriate number of spaces */
char*							GetTextViewSpaceFromTabLine(TextViewRec* View, long LineIndex);

/* given a character index, calculate where the corresponding position is */
/* when tabs have been converted into spaces */
long							TextViewCalculateColumnFromCharIndex(TextViewRec* View,
										long LineIndex, long CharIndex);

/* put a new line of data in.  data on the old line is replaced */
MyBoolean					SetTextViewLine(TextViewRec* View, long LineIndex, char* LineToCopy);

/* get a copy of the selected area */
struct TextStorageRec*	TextViewGetSelection(TextViewRec* View);

/* delete the selection.  if this fails, the selection may have been partially */
/* deleted */
MyBoolean					TextViewDeleteSelection(TextViewRec* View);

/* insert a new block of data at the insertion point, deleting any existing */
/* selection first.  if this fails, the block may have been partially inserted */
MyBoolean					TextViewInsertBlock(TextViewRec* View, struct TextStorageRec* Data);

/* convert the text view box into a packed buffer of lines. */
char*							TextViewGetRawData(TextViewRec* View, char* EOLN);

/* delete the old data in the text view object and replace it with the */
/* specified block of data */
MyBoolean					TextViewNewRawData(TextViewRec* View, char* EOLN, char* RawData);

/* write the data to a file.  returns True if successful */
MyBoolean					TextViewWriteDataToFile(TextViewRec* View,
										struct FileType* FileRefNum, char* EOLN);

#endif
