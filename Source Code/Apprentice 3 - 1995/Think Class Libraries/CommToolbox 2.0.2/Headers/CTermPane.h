/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같�

	CTermPane.h
	
	Commtoolbox terminal emulation class.
	
	SUPERCLASS = CPanorama.
	
	Copyright � 1992-93 Romain Vignes. All rights reserved.
	Modified 1994 Ithran Einhorn.
	
같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같� */

#ifndef _H_CTermPane
#define _H_CTermPane

#include <Terminals.h>				/* Apple includes */

#ifdef __USETHREADS__
	#include "CThread.h"			/* Thread Manager */
#endif

#include <CAbstractText.h>			/* Interface for its superclass */

#include "CConnection.h"
#include "CFileTransfer.h"
#include "CScrollBuffer.h"

/* Constants & Macros */

#define EMPTY_STR			0L

/* Terminal specific commands */

#define cmdTermChoose		2200	/* Terminal tool setup */
#define cmdTermClear		2201	/* Screen clearing */
#define cmdTermReset		2202	/* Terminal reset */
#define cmdTabCopy			2203	/* Table copy */

// parameters for private copy method
enum {
	kStdCopy = 0,
	kTabCopy = 1
};

/* Broadcast indications */

enum	{
	termSizeChangedInd = 2201		/* Emulation resizing */
};

typedef enum { kBreakLeft, kBreakRight } BreakDirection;

typedef TerminalSendProcPtr				TMSendProcPtr;
typedef TerminalBreakProcPtr			TMBreakProcPtr;
typedef TerminalSearchCallBackProcPtr	TMSearchProcPtr;
typedef TerminalClikLoopProcPtr			TMClickProcPtr;
typedef TerminalEnvironsProcPtr			TMEnvProcPtr;
typedef TerminalChooseIdleProcPtr		TMChIdleProcPtr;

// Is the Apple definition wrong?
//typedef TerminalCacheProcPtr			TMCacheProcPtr;
typedef pascal long (*TMCacheProcPtr) (long refCon, TermDataBlock *theTermData);

/* Class definition */

class CTermPane : public CAbstractText {

protected:
	TCL_DECLARE_CLASS

	/* Class Variables */
		
	static	CPtrArray<CTermPane>	*cTermList;
	
	/* Instance Variables */

	TermHandle		itsTerm;
	
	Ptr				ReadBuffer;
	long			readAmount;
	CMFlags			readFlags;
	CMErr			asyncErr;
	
	Boolean			asyncRead;
	Boolean			readCompleted;
	long			TimeOutLen;
	CConnection		*itsConnection;
	CFileTransfer	*itsFileTransfer;
	
	#ifdef __USETHREADS__
	CThread			*itsIdleThread;
	#endif
	
	long			origLine, lastLine;	// helper variables for mouse tracking
	long			origCharOffset;
	long			lastCharOffset;
	Boolean			reallyActive;		/* Is the pane really active?		*/
	
	long			numCachedLines;		/* scrollback data objects			*/
	CScrollBuffer	*itsScrollBuffer;
	
	int				topMargin;			/* border on top of terminal		*/
	
	/* private methods */
	void					changedConfig(Boolean doBroadcast = true);
	long					EnTab(Handle selectionHdl);
	virtual void			SetTermOffset(void);	
	void					TestTerm(void);
	virtual void			DeltaTermSize(int *deltaH, int *deltaV);
	virtual void			CalcTermRect(Rect *frameRect);
	virtual void			CalcFullRect(Rect *frameRect);
	virtual void			CalcTermViewRect(Rect *frameRect);
	virtual void			GetTermRect(Rect *frameRect);
	virtual void			EraseTermBorder(void);
	virtual void			AdjustTermPosition(TermPtr oldRecord, TermEnvironRec *theOldEnvirons);
	virtual	void			RefreshLine(int line);
	virtual					CalcLineRect(int line, Rect *lineRect);
	virtual	short			PixelsToLines(short	pixCount);
	virtual void			GetTermExtent(long *theHExtent,long *theVExtent);
	virtual void			AdjustBounds(void);
		
public:

	long			itsSelStart;		/* Start of selection range			*/
	long			itsSelEnd;			/* End of selection range			*/
	long			itsSelAnchor;		/* Anchor of selection range		*/
	
	/* Class Methods */
	
	static OSErr			cCheckToolName(Str63 toolName);
	static void				cInitManager(void);
	static short			cGetTMVersion(void);
	static void				cTermIdle(void);
	static pascal void 		*cOneTermIdle (void *threadParam);
	static Boolean			cTestToolMenu(short theMenu, short theItem);
	static Boolean			cTestToolEvent(EventRecord *macEvent,WindowPtr theWindow);
	
	/* Instance Methods */
	
	CTermPane(CView *anEnclosure, CBureaucrat *aSupervisor,
			  Str63 toolName,Str63 commName,Str63 xferName,TMFlags flags,
			  TMSendProcPtr sendProc,TMCacheProcPtr cacheProc,
			  TMBreakProcPtr breakProc,TMClickProcPtr clickLoop,
			  TMEnvProcPtr environsProc,long refCon,long userData,
			  Boolean isAsync,long TimeOut = 60L);
	CTermPane();
	~CTermPane();
	
	virtual void			Draw(Rect *area);
	virtual void			DrawAllOffscreen(Rect *area);
	virtual void			RefreshOffscreen(void);
	virtual void			DoCommand(long theCommand);
	virtual void			UpdateMenus(void);
	virtual void			TermChoose(void);
	virtual void			DoIdle(void);
	virtual void			AdjustCursor(Point where,RgnHandle mouseRgn);	
	virtual void			Activate(void);
	virtual void			Deactivate(void);
	virtual void			DoClick(Point hitPt,short modifierKeys,long when);
	virtual void			ResizeFrame(Rect *delta);
	virtual void			DoKeyDown(char theChar,Byte keyCode,EventRecord	*macEvent);
	virtual void			DoAutoKey(char theChar,Byte keyCode,EventRecord	*macEvent);
	virtual short			SetConfig(char *theConfig);
	virtual void			GetToolName(Str63 toolName);
	virtual Ptr				GetConfig(void);
	virtual Boolean			DoMenu(short theMenu,short theItem);
	virtual Boolean			DoEvent(EventRecord *theEvent,WindowPtr theWindow);
	virtual TermHandle		GetTermHandle(void);
	virtual long			DoStream(Ptr buffer,long buffSize,CMFlags flags);
	virtual void			TermClear(void);
	virtual void			Reset(void);
	virtual void			DoTabCopy(void);
	virtual void			GetExtent(long *theHExtent,long *theVExtent);
	virtual OSErr			GetEnvirons(TermEnvironRec *theEnvirons);
	virtual void			GetLine(short lineNo,TermDataBlock *theTermData);
	virtual void			Scroll(long hDelta,long vDelta,Boolean redraw);
	virtual void			ScrollTerminal(int hDelta, int vDelta);
	virtual void			DoSelectAll(void);
	virtual void			BindConnection(CConnection *newConnection);
	virtual	CConnection		*GetConnection(void);
	virtual	CFileTransfer	*GetFileTransfer(void);
	virtual	Point			GetCursor(TMCursorTypes curType = cursorText);
	
	// member function overrides
	
	void			Prepare(void);
	
	void			SetWholeLines(Boolean aWholeLines);
	void			SetTextPtr(Ptr textPtr, long numChars);
	Handle			GetTextHandle(void);
	void			SetFontNumber(short aFontNumber);
	//void			SetFontName(Str255 aFontName);
	void			SetFontStyle(short aStyle);
	void			SetFontSize(short aSize);
	void			SetTextMode(short aMode);
	void			SetAlignCmd(long anAlignment);
	long			GetAlignCmd( void);
	void			SetSpacingCmd(long aSpacingCmd);
	long			GetHeight( long startLine, long endLine);
	long			GetCharOffset( LongPt *aPt);
	void			GetCharPoint( long offset, LongPt *aPt);
	void			GetTextStyle( short *whichAttributes, TextStyle *aStyle);
	void			GetCharStyle( long charOffset, TextStyle *theStyle);
	long			FindLine(long charPos);
	long			GetLength( void);
	void			TypeChar(char theChar, short theModifiers);
	void			SetSelection( long selStart, long selEnd, Boolean fRedraw);
	long			GetSpacingCmd( void);
	void			GetSelection( long *selStart, long *selEnd);
	long			GetNumLines(void);
	Handle			CopyTextRange( long start, long end);
	void			PerformEditCommand( long theCommand);
	void			InsertTextPtr( Ptr text, long length, Boolean fRedraw);
	void			Dawdle(long *maxSleep);
	void			HideSelection(Boolean hide, Boolean redraw);
	void			GetFrameSpan(short *theHSpan, short *theVSpan);
	
	// utility routines
	void					GetCharBefore( long *aPosition, tCharBuf charBuf);
	void					GetCharAfter( long *aPosition, tCharBuf charBuf);
	virtual	void			GetCharAt( long *aPosition, tCharBuf charBuf);
	virtual	long			LinesScrolledUp(void);
	virtual	long			LinesScrolledDown(void);
	virtual	long			TerminalTextLength(long startLine, long endLine);
	virtual	long			GetLineStart(long line);
	virtual	long			GetLineEnd(long line);
	virtual	long			GetLineLength(long line);
	virtual void			GetLineInfo(long line, long *start, long *len);
	
	// static call-back routines
	static pascal CMErr		DefEnvironsProc (long refCon, ConnEnvironRecPtr theEnvPtr);
	static pascal long		DefSendProc (Ptr buffPtr, long howMany, long refCon, CMFlags flags);
	static pascal void		DefBreakProc (long duration, long refCon);
	static pascal void		DefReadCompletedProc (ConnHandle myConn);
	static pascal long		DefCacheProc (long refCon, TermDataBlock *theTermData);
	static pascal Boolean	DefClikLoop (long refCon);
	
	// cache support routines
	virtual void			DrawLineRange(long startLine, long endLine, long startLineOffset,
										  Boolean erase);
	virtual void			HiliteTextRange(long startChar, long endChar);
	virtual void			HiliteRect(const Rect *hiliteRect);
	virtual	long			CalcWordBreak(long charPos, BreakDirection direction);
	virtual	Boolean			ProcessCacheClick(Point hitPt);
	virtual void			DoCopy(short copyType = kStdCopy);
};

#endif

/* 같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같같 */
