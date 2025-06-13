/********************************************************\
 CWASTEText.h

	by Dan Crevier
	version 1.6
 
\********************************************************/

#pragma once

#include "CAbstractText.h"
#include "WASTE.h"
class CPrinter;
class CWASTEText;

//#define WASTE_OBJECT_ARCHIVE

long  const cmdBlack=11420;
long  const cmdRed=11421;
long  const cmdGreen=11422;
long  const cmdBlue=11423;
long  const cmdCyan=11424;
long  const cmdMagenta=11425;
long  const cmdYellow=11426;

#ifdef WASTE11
// define WASTE_TABS if you are using the optional tab code in CWASTE1.1r7 or later
#define WASTE_TABS
// define WASTE_AUTO_TABS if you want all CWASTEText objects to automatically install
// the hooks for WASTE tabs -- requires CWASTE1.1r7 or later
#define WASTE_AUTO_TABS
#endif

class CWASTEText : public CAbstractText
{
	public:
		
		TCL_DECLARE_CLASS
		
		// data members
		WEHandle 	macWE;			// handle to WE data
		long		spacingCmd;		// Line spacing command number
		long		alignCmd;		// alignment cmd number
		long		printPageWidth;
		CPrinter	*itsPrinter;
			
		// construction/destruction
		CWASTEText();
		CWASTEText(CView *anEnclosure, CBureaucrat *aSupervisor,
			short aWidth = 0, short aHeight = 0, short aHEncl = 0,
			short aVEncl = 0, SizingOption aHSizing = sizELASTIC,
			SizingOption aVSizing = sizELASTIC, short aLineWidth = -1,
			Boolean aScrollHoriz = FALSE, TextStyle *tStyle = NULL);
		
		virtual ~CWASTEText();

		void IWASTEText(CView *anEnclosure, CBureaucrat *aSupervisor,
							short aWidth, short aHeight,
							short aHEncl, short aVEncl,
							SizingOption aHSizing, SizingOption aVSizing,
							short aLineWidth, TextStyle *tStyle = NULL);
		virtual void IViewTemp(CView *anEnclosure, CBureaucrat *aSupervisor,
							Ptr viewData);
		void IWASTETextX();

#ifdef WASTE11
		virtual void DoKeyDown(char theChar, Byte keyCode, EventRecord *macEvent);
		virtual WEActionKind GetUndoInfo(Boolean *redoFlag);
		virtual void DoUndo(void);
#else
		CTextEditTask		*MakeEditTask( long editCmd);
		CTextStyleTask		*MakeStyleTask( long styleCmd);
#endif

#ifdef WASTE_TABS
		virtual void		InstallTabHandlers(void);
#endif

		virtual void 		AdjustCursor(Point where, RgnHandle mouseRgn);

#ifdef WASTE11
		// drag and drop
		void				InstallDragHandlers(void);
		void 				RemoveDragHandlers(void);
		virtual Boolean		DoDrag(EventRecord *theEvent, Point where);
#endif

		// Menu and commands
		virtual void		UpdateMenus(void);
		virtual void 		DoCommand(long theCommand);  //mf

		// Mouse and Keystrokes
		virtual void		DoClick(Point hitPt, short modifierKeys, long when);
		virtual void		PerformEditCommand(long theCommand);

		// Validation
		virtual void		CheckInsertion(long numChars, long styleSize, Boolean useSelection);

		// Display
		virtual void		Draw(Rect *area);
		virtual void		Activate();
		virtual void		Deactivate();
		virtual void		SetSelection(long selStart, long selEnd, Boolean fRedraw);
		Boolean				SetOutlineHighliting(Boolean hilite);
		virtual void		SetTextMargin(short margin);
		
		// Text Specification
		virtual void 		Clear(void);
		virtual void 		SetTextPtr(Ptr textPtr, long numChars);
		virtual void 		StopInlineSession(void);
		virtual Handle 		GetTextHandle(void);
		virtual Handle 		CopyTextRange(long start, long end);
		virtual void 		CopyRangeWithStyle(long start, long end, Handle hText,
								StScrpHandle hStyles);
#ifdef WASTE11
		virtual void 		CopyRangeWithStyleSoup(long start, long end, Handle hText,
								StScrpHandle hStyles, WESoupHandle hSoup);
#endif
		virtual void 		InsertTextPtr(Ptr text, long length, Boolean fRedraw);
		virtual void 		InsertWithStyle(Ptr text, long length, StScrpHandle hStyles,
								Boolean fRedraw);
#ifdef WASTE11
		virtual void 		InsertWithStyleSoup(Ptr text, long length, StScrpHandle hStyles,
								WESoupHandle hSoup, Boolean fRedraw);
#endif
		virtual void 		TypeChar(char theChar, short theModifers);
		virtual void 		Specify(Boolean fEditable, Boolean fSelectable,
								Boolean fStylable);

		// Calibrating
		virtual void 		CalcWERects();
		virtual void 		ResizeFrame(Rect *delta);
		virtual void 		AdjustBounds();
		virtual void 		InhibitRecal(Boolean inhibit);
	
		virtual long 		FindLine(long charPos);
		virtual long 		GetLength();
		
		// Text Characteristics
		virtual void		SetFontColor(RGBColor *aFontColor);		//mf
		virtual void		SetFontColorAll(RGBColor *aFontColor);		//mf
		virtual void 		SetFontNumber(short aFontNumber);
		virtual void 		SetFontNumberAll(short aFontNumber);
		virtual void 		SetFontNameAll(Str255 aFontName);
		virtual void 		SetFontStyle(short aStyle);
		virtual void 		SetFontStyleAll(short aStyle);
		virtual void 		SetFontSize(short aSize);
		virtual void 		SetFontSizeAll(short aSize);
		virtual void 		SetTextMode(short aMode);
		virtual void 		SetAlignment(short anAlignment);
		virtual void 		SetAlignCmd(long alignCmd);
		virtual void 		SetSpacingCmd(long aSpacingCmd);
		virtual void		SetTheStyleScrap(long rangeStart, long rangeEnd,
								StScrpHandle styleScrap, Boolean redraw);
		virtual void 		SetStyle(short mode, TextStyle *newStyle, Boolean redraw);

		virtual long 		GetHeight(long startLine, long endLine);
		virtual long 		GetCharOffset(LongPt *aPt);
		virtual void 		GetCharPoint(long offset, LongPt *aPt);
		virtual void 		GetTextStyle(short *whichAttributes, TextStyle *aStyle);
		virtual StScrpHandle GetTextStyles(void);
		virtual void 		GetCharStyle(long charOffset, TextStyle *theStyle);
		virtual long 		GetSpacingCmd();
		virtual long 		GetAlignCmd();
		virtual StScrpHandle GetTheStyleScrap();
#ifdef WASTE11
		virtual WESoupHandle GetTheSoup(void);
#endif

		virtual long 		GetNumLines(void);
		virtual void		GetSelection(long *selStart, long *selEnd);
		virtual void 		HideSelection(Boolean hide, Boolean redraw);
		virtual void 		GetSteps(short *hStep, short *vStep);

		// Printing
		virtual void 		Paginate(CPrinter *aPrinter, short pageWidth, short pageHeight);
		virtual void 		AboutToPrint(short *firstPage, short *lastPage);
		virtual void 		PrintPage(short pageNum, short pageWidth, short pageHeight,
								CPrinter *aPRinter);
		virtual void 		DonePrinting();

#ifdef WASTE11
		// Pictures
		virtual void 		InsertPicture(PicHandle pHandle, Boolean fRedraw);

		// Sounds
		virtual void 		InsertSound(Handle sHandle, Boolean fRedraw);

#ifdef WASTE_OBJECT_ARCHIVE
		// Files
		virtual void 		InsertFSSpec(FSSpec *theFSSpec, Boolean fRedraw);
#endif // WASTE_OBJECT_ARCHIVE
#endif // WASTE11

		// Cursor
		virtual void 		Dawdle(long *maxSleep);

		// Object I/O
		virtual void 		PutTo(CStream& stream);
		virtual void 		GetFrom(CStream& stream);

		static CWASTEText *curWASTEText;
		static pascal void WEPostUpdate(WEHandle hWE, long fixLength, long inputAreaStart,
							long inputAreaEnd, long pinRangeStart, long pinRangeEnd);
		static pascal void WEPreUpdate(WEHandle hWE);
		static pascal Boolean WEClickLoop(WEHandle hWE);
	
	protected:

		// temporarily changing selection to all
		virtual void 		TempSelectAll(void);
		virtual void 		RestoreSelection(void);

		Boolean				dragHandlersInstalled;
		short				textMargin;
};
