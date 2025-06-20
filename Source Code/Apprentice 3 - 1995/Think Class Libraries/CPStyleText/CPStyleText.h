/**********************************************
 "CPStyleText.h"

 by John A. Love, III [Ph.D. student]

 using Symantec's "THINK C / C++", v 6.�.1
 based on Christopher R. Wysocki's "CPEditText"
 **********************************************/



#pragma	once						/* Include this file only once */


#include <CArray.h>					/* For line record data		   */

#include <CScrollBar.h>				/* Superclasses ...			   */
#include "CPEditText.h"
#include "CPEditScrollPane.h"
#include "CTextStyleTask.h"
/*
	Miscellaneous constants included
	by CPEditText.h:
	
#include "CPEditTextX.h"
*/

#include <CDocument.h>				/* For special utilities ...   */
#include <CWindow.h>


/*
	More than the TCL default
	because of word-wrap:
*/
#define MIN_WINDOW_WIDTH	200

/*
	More word-wrap stuff ...
*/
#define kWillBeRefreshed	TRUE
#define kWontBeRefreshed	FALSE


/*
	Substitutes for Chris' HorizInset and VertInset macros to
	eliminate his built-in offsets.  Offsets should be controlled
	ONLY via calls to ChangeSize, Offset, Place and similar methods.
*/
#define HorizLeftInset()	(frame.left - position.h * hScale)
#define VertTopInset()		(frame.top  - position.v * vScale)


/*
	Now, the beginning of the 'mother' of all additions ... style info ...
	Before this addition is finished, it'll change many, many times.
	For the moment, this is just a shell for us to chew on ...
	
	For example, we need to decide how we're going to save the style info.
	Are we going to create a CResFile in our supervising CDocument and
	attach it to our CDocument via _AddResource when saving it ...
	or are we going to do repeated calls to CDataFile's WriteSome just as
	John Lengyel does in his CDots code (MacTutor - August 1991)?
*/

#define MAX_LINES				65535L
#define MAX_STYLES_PER_LINE		255L

#define atBeginning				-1L
#define atEnd					-65535L

typedef struct	StyleRecord
{
	short		sStart;				/* Beginning & end of Style Run ...	*/
	short		sEnd;
	short		sHeight;
	short		sFont;				/* ... aka TextStyle ...			*/
	short		sSize;
	Style		sFace;
	short		sMode;
	RGBColor	sColor;
	short		sMaxCharWidth;
	
}	StyleRecord, *StyleRecordPtr, **StyleRecordHdl;

typedef struct	LineRecord
{
	/*
		There is one LineRecord per line and within each
		line lStyles is an ordered array of StyleRecords.
		Just as with TextEdit's TEStyleTable, this array
		may NOT contain any duplicate elements.  Each
		StyleRecord in lStyles corresponds one-to-one
		with the styles in each line.
	*/
	
	CArray		*lStyles;
	long		lSpacingCmd;
	short		lHeight;
	Fixed		lJustAmount;
	short		lTabSpaces;
	short		lTabWidth;
	
}	LineRecord, *LineRecordPtr, **LineRecordHdl;



class CPStyleText : public CPEditText
{
public:

				/*
					An ordered array of LineRecords each of which
					corresponds one-to-one with the text in each
					<CR>-delimited line.  This is to say that
					word-wrapping to margins or to line lengths
					does NOT affect this array:
				*/
				
				CArray		*itsLineInfo;
					
					
									/** Construction/Destruction **/
			void		IPStyleText (CView *anEnclosure, CBureaucrat *aSupervisor,
									 short aWidth, short aHeight,
									 short aHEncl, short aVEncl,
									 SizingOption aHSizing, SizingOption aVSizing,
									 short aLineWidth);
	virtual void		IViewTemp (CView *anEnclosure, CBureaucrat *aSupervisor,
								   Ptr viewData);
	
	virtual void		Dispose (void);
									
									/** Display **/
	virtual void		Draw (Rect *area);
	virtual void		SetSelection (long selStart, long selEnd, Boolean fRedraw);

									/** Calibration **/
	virtual void		UpdateMenus (void);
	virtual void		ResizeFrame (Rect *delta);
	virtual void		SetWholeLines (Boolean aWholeLines);
	
									/** Accessing **/
	virtual void		SetBounds (LongRect *aBounds);
	
	virtual long		GetHeight (long startLine, long endLine);
	
	virtual long		GetCharOffset (LongPt *aPt);
	virtual void		GetCharPoint (long offset, LongPt *aPt);

	virtual void		SetMargins (long leftMargin, long rightMargin);
	virtual void		GetMargins (long *leftMargin, long *rightMargin);
	
									/** Mouse and Keystroke **/
	virtual void		DoClick (Point hitPt, short modifierKeys, long when);
	virtual void		DoKeyDown (char theChar, Byte keyCode, EventRecord *macEvent);
	virtual void		TypeChar (char theChar, short theModifiers);
	virtual void		DoArrowKey (char theChar, short theModifiers);
									
									/** Text Specification **/
	virtual void		SetTextPtr (Ptr textPtr, long numChars);
	virtual void		ReplaceSelection (Ptr replacePtr, long replaceLen);

									/** Word Wrap **/
	virtual void		SetWordWrap (Boolean doWrap, long lineLengthInChars);
	virtual Boolean		GetWordWrap (void);
	virtual void		DoWordWrap (void);

									/** Styling **/
	virtual void		SetSpacingCmd (long aSpacingCmd);
	virtual void		SetAlignCmd (long anAlignCmd);
	virtual void		SetAlignment (short anAlignment);
	virtual long		GetAlignCmd (void);
	
	virtual	StScrpHandle	GetStyleScrap (void);
	virtual void		SetStyleScrap (long rangeStart, long rangeEnd,
									   StScrpHandle styleScrap, Boolean redraw);
	
	virtual void		AddLine (long afterLine);
	virtual void		DeleteLine (long lineNum);
	virtual void		AddStyle (long lineNum, long afterStyleIndex,
								  StyleRecord *styleInfo);
	virtual void		DeleteStyle (long lineNum, long styleIndex);
	virtual void		GetStyleInfo (long lineNum, long styleIndex,
									  StyleRecord *styleInfo);
	virtual void		SetStyleInfo (long lineNum, long styleIndex,
									  StyleRecord *styleInfo);
	virtual Boolean		FindStyle (long startPos, long endPos,
								   long *startLine, long *endLine,
								   long *startIndex, long *endIndex);

	virtual void		SetFontNumber (short aFontNumber);
	virtual void		SetFontSize (short aSize);
	virtual void		SetFontStyle (short aStyle);
	virtual void		SetTextMode (short aMode);
	
									/** Commands **/
	virtual void		DoCommand (long theCommand);
	virtual void		PerformEditCommand (long theCommand);
	
									/** One of Chris' Hook Methods **/
	virtual void		CaretHook (const Rect *caretRect);

									/** Scrolling **/
	virtual void		Scroll (long hDelta, long vDelta, Boolean redraw);
	virtual void		ScrollToSelection (void);
	virtual void		ScrollToOffset (long charOffset);

									/** Printing **/
	virtual void		PrintPage (short pageNum, short pageWidth,
								   short pageHeight, CPrinter *aPrinter);
	virtual void		DonePrinting (void);

								/** end Public Methods **/
	
protected:

				long			itsAlignCmd;			/* Alignment NOW supported */
				long			itsLeftMargin;			/* Margins in pixels ...   */
				long			itsRightMargin;
				LongRect		itsViewRect;			/* For word-wrap ...	   */
				LongRect		itsDestRect;
				Boolean			fWordWrap;
				Boolean			fOldEndWrapLineCaret;
				Boolean			fNewEndWrapLineCaret;
				Boolean			fEndWrapLine;
				
									/**			Class Variables			**/
									
									/**		B&W + color cursors ...		**/
									/**									**/
									/** An idea generated from my use	**/
									/** of Symantec's "THINK Reference"	**/
									/** application.					**/
								
	static CursHandle		cAutoScrollCursor;
	static CCrsrHandle		cAutoScrollCRSR;
	
									/**			For styling info		**/
									
	static Boolean			cHasTrueType;


									/** Construction/Destruction **/
			void		IPStyleTextX (void);

									/** Commands **/
	virtual CTextStyleTask	*MakeStyleTask (long styleCmd);
	
									/** Display **/
			long		CountRangeSPACEs (long start, long end);
	virtual void		DrawLineRange (long startLine, long endLine,
									   long startLineOffset, Boolean erase);
	virtual ShortHandle	MeasureTextWidths (long startPos, long endPos,
										   short maxWidth);
	virtual void		HiliteTextRange (long startChar, long endChar);
	virtual void		CalcLineHeight (void);
	virtual void		RefreshTextAfter (long afterPos, Boolean refreshOnlyLine);

									/** Word Wrap **/
	virtual void		CalcLineStarts (void);
	virtual void		RecalcLineStarts (void);
	virtual void		WrapLineStarts (register long startLine,
										register long endLine,
										Boolean willBeRefreshed);
	virtual void		AdjustLineStarts (long startChar, long numCharsDelta,
										  long numLinesDelta);
	virtual void		CalcPERects (void);
	virtual void		AdjustBounds (void);

									/**				  Caret Drawing				 **/
									/**											 **/
									/** Modified Chris' DrawCaret method for	 **/
									/** drawing the caret at the end of a		 **/
									/** "sub-line" of a word-wrapped line.		 **/
									/** Chris' DrawCaret method is declared		 **/
									/** NON-virtual within his "CPEditText.h"	 **/
									/** interface file.  So the most simple! way **/
									/** I can override it for both THINK C and	 **/
									/** Symantec C++ is to declare Chris'		 **/
									/** DrawCaret a virtual method within that	 **/
									/** interface file.							 **/
	virtual void		DrawCaret (void);

								/** end Protected Methods **/
	
};	/* CPStyleText */


class CPStyleScrollPane : CPEditScrollPane
{
public:

									/** Construction/Destruction **/
			void		IPStyleScrollPane (CView *anEnclosure,
										   CBureaucrat *aSupervisor,
										   short aWidth, short aHeight,
										   short aHEncl, short aVEncl,
										   SizingOption aHSizing,
										   SizingOption aVSizing,
										   Boolean hasHoriz, Boolean hasVert,
										   Boolean hasSizeBox);

			void		IPStyleScrollPaneX (Boolean hasHoriz, Boolean hasVert,
											Boolean hasSizeBox);

};	/* CPStyleScrollPane */


class CPStyleScrollBar : CScrollBar
{
public:

									/** Contruction/Destruction **/
			void		IPStyleScrollBar (CView *anEnclosure,
										  CBureaucrat *aSupervisor,
										  Orientation anOrientation, short aLength,
										  short aHEncl, short aVEncl);
	
									/** Mouse and Keystroke **/
	virtual void		DoClick (Point hitPt, short modifierKeys, long when);

};	/* CPStyleScrollBar */


class CStylePStyleTask : public CTextStyleTask
{
	/*
		Construct a new sub-class for CAbstractText's stylable CTask
		because of the sensitivity of word-wrap to various commands,
		such as a new font size and a new font style.  This new
		sibling looks VERY similar to the TCL's CStyleTEStyleTask
		which is created by the TextEdit class = CStyleText.
	*/
				
public:

			void		IStylePStyleTask (CPStyleText *aTextPane,
										  long aStyleCmd, short firstTaskIndex);
	virtual void		Dispose (void);
			
	virtual void		Do (void);
	virtual void		Undo (void);
	
protected:

				StScrpHandle	oldStyles;
				long			selStart, selEnd;
			
	virtual void		SaveStyle (void);
	virtual void		RestoreStyle (void);
	
};	/* CStylePStyleTask */


/****   G E N E R A L   U T I L I T I E S   ****/

			void	DirtyWindow (CWindow *theWindow);
			void	DirtyDocument (CDocument *theDocument);
			void	UndirtyWindow (CWindow *theWindow);
			void	UndirtyDocument (CDocument *theDocument);




/*	{ end file "CPStyleText.h" }  */
