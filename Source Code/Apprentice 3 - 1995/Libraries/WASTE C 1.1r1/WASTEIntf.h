/*
// { WASTE PROJECT: }
// { Internal interface: constants, types and inline code }

// { Copyright © 1993-1995 Marco Piovanelli }
// { All Rights Reserved }
*/

#ifndef __LONGCOORDINATES__
#ifndef _LongCoords_
#include "LongCoords.h"
#endif
#endif

/*	useful macros for testing, setting and clearing bits */

#include <TextEdit.h>
#include <Scrap.h>
#include <TextServices.h>
#include <Script.h>
#include <ToolUtils.h>
#include <TextUtils.h>
#include <LowMem.h>
#include <AERegistry.h>
#include <Drag.h>

#define BTST( FLAGS, BIT )	(((FLAGS) & (1L << (BIT))) ? 1 : 0)
#define BSET( FLAGS, BIT )  ((FLAGS) |= (1L << (BIT)))
#define BCLR( FLAGS, BIT )  ((FLAGS) &= (~(1L << (BIT))))

#define ABS(A) ((A) > 0 ? (A) : -(A))

#define BSL(A, B)	(((long)A) << (B))
#define BSR(A, B)	(((long)A) >> (B))

// if WEREDRAW_SPEED is defined, Mark Alldritt's optimizations of _WERedraw are used
//#define WEREDRAW_SPEED		1

// if WEPINSCROLL is defined, Chris Thomas's modifications are used to make WEScroll()
// behave like TEPinScroll()
//#define WEPINSCROLL			1

// Don't let WASTE synchronize keyboard and fonts
//#define WASTENOSYNCH		1

// if WEFORCE_2BYTE is defined, weFDoubleByte is always defined
// this is to work around a bug in JLK and CLK 1.1.1 which incorrectly
// says there are no double byte scripts in PPC native code
//#define WEFORCE_2BYTE	1

// if WASTE_DEBUG is defined various asserts are done.  If they
// fail, an error will be reported with DebugStr
// #define WASTE_DEBUG

// if WASTE_NO_RO_CARET is defined, the caret will not be shown if it is read-only
//#define WASTE_NO_RO_CARET	1

// if WASTE_IC is defined, cmd-clicking URL's is supported through Internet Config
// #define WASTE_IC

#include "LongCoords.h"
#include <limits.h>

/* result codes */
enum {
	weCantUndoErr				=	-10015,	/* undo buffer is clear (= errAECantUndo) */
	weEmptySelectionErr			=	-10013,	/* selection range is empty (= errAENoUserSelection) */
	weUnknownObjectTypeErr		=	-9478,	/* specified object type is not registered */
	weObjectNotFoundErr			=	-9477,	/* no object found at specified offset */
	weReadOnlyErr				=	-9476	/* instance is read-only */
};

// { values for WEInstallObjectHandler handlerSelector parameter }
enum {
		weNewHandler = 'new ',
		weDisposeHandler = 'free',
		weDrawHandler = 'draw',
		weClickHandler = 'clik',
		weCursorHandler = 'curs'
};

/*	action kinds */

enum {
	weAKNone			=	0,		/* null action */
	weAKUnspecified		=	1,		/* action of unspecified nature */
	weAKTyping			=	2,		/* some text has been typed in */
	weAKCut				=	3,		/* the selection range has been cut */
	weAKPaste			=	4,		/* something has been pasted */
	weAKClear			=	5,		/* the selection range has been deleted */
	weAKDrag			=	6,		/* drag and drop operation */
	weAKSetStyle		=	7		/* some style has been applied to a text range */
};

/* action flags */

enum {
	weAFIsRedo			=	0x0001,	// action saves edit state prior to a WEUndo call
	weAFDontSaveText	=	0x0002, // don't save text
	weAFDontSaveStyles	=	0x0004,	// don't save styles
	weAFDontSaveSoup	=	0x0008
};

typedef short WEActionKind;
typedef short WEActionFlags;

/*	alignment styles */

enum {
	weFlushLeft 		=	-2,		/* flush left */
	weFlushRight		=	-1,		/* flush right */
	weFlushDefault		=	 0,		/* flush according to system direction */
	weCenter			=	 1,		/* centered */
	weJustify			=	 2		/* fully justified */
};

// { values for the edge parameter }

enum {
	kLeadingEdge = -1,			// { point is on the leading edge of a glyph }
	kTrailingEdge = 0,			// { point is on the trailing edge of a glyph }
	kObjectEdge = 2				// { point is inside an embedded object }
};

// { control character codes }

enum {
	kObjectMarker = 1,
	kBackspace = 8,
	kTab = 9,
	kEOL = 13,
	kSpace = 32,
	kArrowLeft = 28,
	kArrowRight = 29,
	kArrowUp = 30,
	kArrowDown = 31,
	kForwardDelete =127
};

// { bit equates for QuickDraw styles }

enum {
	tsBold = 0,
	tsItalic = 1,
	tsUnderline = 2,
	tsOutline = 3,
	tsShadow = 4,
	tsCondense = 5,
	tsExtend = 6
};

// { bit equates for the tsFlags field of the WETextStyle record }
enum {
	tsTSMHilite = 4,						// { set if style run is part of active input area }
	tsTSMSelected = 5,						// { set for selected raw/converted text }
	tsTSMConverted = 6,						// { set for converted text, clear for raw text }
	tsRightToLeft = 7						// { reserved for future use }
};

// { bit equates for the mode parameter in WESetStyle and WEContinuousStyle }
enum {
	kModeFont = 0,
	kModeFace = 1,
	kModeSize = 2,
	kModeColor = 3,
	kModeAddSize = 4,
	kModeToggleFace = 5,
	kModeReplaceFace = 6,
	kModeObject = 14,
	kModeFlags = 15
};

// { values for the mode parameter in WESetStyle and WEContinuousStyle }
enum {
	weDoFont			=	0x0001,
	weDoFace			=	0x0002,
	weDoSize			=	0x0004,
	weDoColor			=	0x0008,
	weDoAll				=	weDoFont + weDoFace + weDoSize + weDoColor,
	weDoAddSize			=	0x0010,
	weDoToggleFace		=	0x0020,
	weDoReplaceFace		=	0x0040,
	weDoObject			=	0x4000,
	weDoFlags			=	0x8000
};

// { values for WEFeatureFlag action parameter }
enum {
	weBitClear = 0,								// { disables the specified feature }
	weBitSet = 1,								// { enables the specified feature }
	weBitTest = -1,								// { returns the current setting of the specified feature }
	weBitToggle = -2							// { toggles the specified feature }
};

// { bit equates for the flags field in the WE record }
// { bits 0..15 can be used to turn on and off specific features with WEFeatureFlag }
// { bits 16..31 are used internally and should not be modified }

// { private flags }
enum {
	weFHasColorQD = 31,						// { Color QuickDraw is available }
	weFHasTextServices = 30,				// { Text Services Manager is available }
	weFNonRoman = 29,						// { at least one non-Roman script is enabled }
	weFDoubleByte = 28,						// { a double-byte script is installed }
	weFHasDragManager = 27,					// { Drag Manager is available }
	weFCaretVisible = 24,					// { the caret is currently visible }
	weFMouseTracking = 23,					// { set internally during mouse tracking }
	weFAnchorIsEnd = 22,					// { anchor offset is selEnd }
	weFUseNullStyle = 21,					// { a null style is associated with the empty selection }
	weFActive = 20,							// { we're active }
	weFHilited = 19,						// { TRUE if text pane is highlighted (for Drag & Drop) }
	weFCanAcceptDrag = 18,					// { The drag in the text pane can be accepted }
	weFDragCaretVisible = 17,				// { Drag caret is currently visible }
	weFDestRectChanged = 16					// { Set if scroll callback needs to be called }
};

// { public flags }
enum {
	weFDrawOffscreen = 11,					// { draw text offscreen for smoother visual results }
	weFUseTempMem = 10,						// { use temporary memory for main data structures }
	weFInhibitRecal = 9,					// { if set, recals and redraws are inhibited }
	weFDragAndDrop = 8,						// { Support Drag and Drop }
	weFIntCutAndPaste = 7,					// { Intelligent Cut & Paste }
	weFUndoSupport = 6,						// { Support Undo/Redo }
	weFReadOnly = 5,						// { Disallow editing }
	weFOutlineHilite = 2,					// { frame selection range when text pane is inactive }
	weFAutoScroll = 0						// { automatically scroll text when cursor is outside pane }
};

// { masks for setting the feature bits }
enum {
	weDoAutoScroll = 1 << weFAutoScroll,
	weDoOutlineHilite = 1 << weFOutlineHilite,
	weDoReadOnly = 1 << weFReadOnly,
	weDoUndoSupport = 1 << weFUndoSupport,
	weDoIntCutAndPaste = 1 << weFIntCutAndPaste,
	weDoDragAndDrop = 1 << weFDragAndDrop,
	weDoInhibitRecal =  1 << weFInhibitRecal,
	weDoUseTempMem = 1 << weFUseTempMem,
	weDoDrawOffscreen = 1 << weFDrawOffscreen 
};

// { scrap types }
enum {
	kTypeText = 'TEXT',
	kTypeStyles = 'styl',
	kTypePicture = 'PICT',
	kTypeSoup = 'SOUP'
};

// { selectors for WEGetInfo/WESetInfo }
enum {
	weCharByteHook = 'cbyt',
	weCharTypeHook = 'ctyp',
	weCharToPixelHook = 'c2p ',
	weClickLoop = 'clik',
	weCurrentDrag = 'drag',
	weDrawTextHook = 'draw',
	weLineArray = 'line',
	weLineBreakHook = 'lbrk',
	wePixelToCharHook = 'p2c ',
	wePort = 'port',
	weRefCon = 'refc',
	weRunArray = 'runa',
	weScrollProc = 'scrl',
	weStyleTable = 'styl',
	weText = 'text',
	weTranslateDragHook = 'xdrg',
	weTSMDocumentID = 'tsmd',
	weTSMPostUpdate = 'post',
	weTSMPreUpdate = 'pre '
};

// { possible values returned by WEIntelligentPaste }
enum {
	weDontAddSpaces = 0,
	weAddSpaceOnLeftSide = -1,
	weAddSpaceOnRightSide = 1
};

// { values for WEAllocate allocFlags parameter }
enum {
	kAllocClear = 0x0001,						// { clear handle after allocation }
	kAllocTemp = 0x0002						// { use temporary memory if available }
};

// { other miscellaneous constants }
enum {
	kCaretWidth = 1,					// { width of the caret, in pixels }
	kMinFontSize = 1,					// { minimum valid font size }
	kMaxFontSize = SHRT_MAX,			// { maximum valid font size }
	kInvalidOffset = -1, 				// { used to flag an invalid or nonexistent offset }
	kNullDrag = 0,						// { null or nonexistent drag reference }
	kNullObject = 0						// { null object reference }
};

// from WEMouse.c
enum {
	noDragErr = 128,
	kTextMargin = 3,		// { width of border area surrounding the text (in pixels) }
	kAutoScrollDelay = 10	// { delay before auto-scroll starts (in ticks) }
};


// static long kOneToOneScaling = 0x00010001;		// { 1:1 scaling ratio }
static Point kOneToOneScaling = {1, 1}; // { 1:1 scaling ratio }

// { WETextStyle is just a familiar TextStyle record as defined by TextEdit, where }
// { tsFace is redefined as SignedByte (rather than Style) so that the filler byte }
// { becomes "visible" from Pascal and can be used to store internal flags }

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif

typedef struct WETextStyle {
	short tsFont;				/* font family number */
	char tsFace;				/* set of QuickDraw styles */
	char tsFlags;				/* flags (FOR INTERNAL USE ONLY) */
	short tsSize;				/* font size, in integer points */
	RGBColor tsColor;			/* absolute red-green-blue color */
	long tsObject;			// { embedded object reference }
} WETextStyle, *WETextStylePtr, **WETextStyleHandle;

typedef struct WERunAttributes {
	short runHeight;			/* style run height (ascent + descent + leading) */
	short runAscent;			/* font ascent */
	WETextStyle runStyle;		/* text style */
} WERunAttributes, *WERunAttributesPtr, **WERunAttributesHandle;

typedef struct WERunInfo {
	long runStart;				/* byte offset to first character of this style run */
	long runEnd;				/* byte offset to first character of next style run */
	WERunAttributes runAttrs;	/* run attributes */
} WERunInfo, *WERunInfoPtr, **WERunInfoHandle;

typedef struct WEStyleScrapElement {
	long scrpStartChar;
	WERunAttributes scrpAttrs;
} WEStyleScrapElement, *WEStyleScrapElementPtr, **WEStyleScrapElementHandle;

// { another "view" for the familiar TextEdit 'styl' Clipboard type }
/*
typedef struct WEStyleScrap {
	short scrpNStyles;
	WEStyleScrapElement scrpStyleTab[1];
} WEStyleScrap, *WEStyleScrapPtr, **WEStyleScrapHandle;
*/
// { the purpose of the GrafPort1 declaration is to redefine the txFace field as SignedByte, }
// { rather than Style ( = set of StyleItem), to avoid using THINK Pascal runtime routines }
// { for set manipulation }

typedef struct GrafPort1
{
	short						device;
	BitMap						portBits;
	Rect						portRect;
	RgnHandle					visRgn;
	RgnHandle					clipRgn;
	Pattern						bkPat;
	Pattern						fillPat;
	Point						pnLoc;
	Point						pnSize;
	short						pnMode;
	Pattern						pnPat;
	short						pnVis;
	short						txFont;
	char						txFace;
	char						filler;
	short						txMode;
	short						txSize;
	Fixed						spExtra;
	long						fgColor;
	long						bkColor;
	short						colrBit;
	short						patStretch;
	Handle						picSave;
	Handle						rgnSave;
	Handle						polySave;
	QDProcsPtr					grafProcs;
} GrafPort1, *GrafPtr1;

typedef struct QDEnvironment {
	GrafPtr envPort;
	PenState envPen;
	WETextStyle envStyle;
	short envMode;
} QDEnvironment;

typedef struct RunArrayElement {
	long runStart;						// { offset to first character in style run }
	long styleIndex;					// { index into style table (high word currently unused) }
} RunArrayElement, *RunArrayElementPtr, **RunArrayElementHandle;

typedef struct RunArrayPair {
	RunArrayElement first;
	RunArrayElement second;
} RunArrayPair, *RunArrayPeek;

typedef struct TERunAttributes {
	short runHeight;
	short runAscent;
	TextStyle runTEStyle;
} TERunAttributes, *TERunAttributesPtr;

typedef struct TEStyleScrapElement {
	long scrpStartChar;
	TERunAttributes scrpTEAttrs;
} TEStyleScrapElement, *TEStyleScrapElementPtr;

typedef struct TEStyleScrap {
	short scrpNStyles;
	TEStyleScrapElement scrpStyleTab[1];
} TEStyleScrap, *TEStyleScrapPtr, **TEStyleScrapHandle;

typedef struct TEStyleScrapPair {
	TEStyleScrapElement first;
	TEStyleScrapElement second;
} TEStyleScrapPair, *TEStyleScrapPeek;


// const kRunArrayMaxIndex = (maxLong / sizeOf(RunArrayElement)) - 1;

typedef RunArrayElement *RunArrayPtr;
typedef RunArrayPtr *RunArrayHandle;

typedef struct StyleTableElement {
	long refCount;					// { reference count }
	WERunAttributes info;			// { style information }
} StyleTableElement, *StyleTableElementPtr, **StyleTableElementHandle;

//		kStyleTableMaxIndex = (maxint div SizeOf(StyleTableElement)) - 1;

typedef StyleTableElement *StyleTablePtr;
typedef StyleTablePtr *StyleTableHandle;

typedef struct LineRec {
	long lineStart;				/* byte offset to first character in line */
	long lineOrigin;			/* pixel offset from destRect.top */
	short lineAscent;			/* maximum font ascent for this line */
	short lineSlop;				/* extra pixels needed to fill up the line */
	Fixed lineJustAmount;		/* normalized slop value, used for justification */
} LineRec, *LinePtr, **LineHandle;


typedef struct LinePair {
	LineRec first;
	LineRec second;
} LinePair, *LinePeek;

//		kLineArrayMaxIndex = (maxLongInt div SizeOf(LineRec)) - 1;

typedef LineRec *LineArrayPtr;
typedef LineArrayPtr *LineArrayHandle;

typedef struct	WERec {
	GrafPtr port;				/* graphics port text is drawn into */
	Handle hText;				/* handle to the text */
	LineArrayHandle hLines;		/* handle to the line array */
	StyleTableHandle hStyles;	/* handle to the style table */
	RunArrayHandle hRuns;		/* handle to the style run array */
	long textLength;			/* length of text */
	long nLines;				/* number of lines */
	long nStyles;				/* number of unique styles in the style table */
	long nRuns;					/* number of style runs */
	LongRect viewRect;			/* view rectangle, all drawing is clipped to this */
	LongRect destRect;			/* destination rectangle */
	long selStart;				/* start of selection range */
	long selEnd;				/* end of selection range */
	long flags;					/* 32 bits of miscellaneous flags */
	long caretTime;				/* time of most recent caret drawing, in ticks */
	long clickTime;				/* time of most recent click, in ticks */
	long clickLoc;				/* byte offset of most recent click */
	long anchorStart;			/* start offset of anchor word/line */
	long anchorEnd;				/* end offset of anchor word/line */
	UniversalProcPtr clickLoop;	/* click loop callback */
	char unused1;				/* unused */
	char clickEdge;				/* edge of character hit by most recent click */
	char unused2;				/* unused */
	char firstByte;				/* first byte of a double-byte character */
	GrafPtr offscreenPort;		/* offscreen graphics world */
	RgnHandle viewRgn;			/* handle to the view region */
	UniversalProcPtr scrollProc;		/* scroll callback */
	short clickCount;			/* multiple click count */	
	char alignment;				/* alignment style */
	long refCon;				/* reference value for client use */
	TSMDocumentID tsmReference;	/* reference value for the text services manager */
	long tsmAreaStart;			/* start of active input area (for TSM) */
	long tsmAreaEnd;			/* end of active input area */
	UniversalProcPtr tsmPreUpdate;		/* TSM pre-update callback */
	UniversalProcPtr tsmPostUpdate;	/* TSM post-update callback */
	DragReference currentDrag;	/* refereence of drag being tracked by _WEDrag */
	long dragCaretOffset;		/* offset to caret displayed during a drag */
	UniversalProcPtr translateDragHook;	/* drag translation hook */
	Handle hActionStack;		/* action stack for undo */
	long modCount;				/* modification count */
	UniversalProcPtr drawTextHook;		// { hook for drawing text }
	UniversalProcPtr pixelToCharHook;	// { hook for hit-testing }
	UniversalProcPtr charToPixelHook;	// { hook for locating glyph position }
	UniversalProcPtr lineBreakHook;		// { hook for finding line breaks }
	UniversalProcPtr wordBreakHook;		// { hook for finding word breaks }
	UniversalProcPtr charByteHook;		// { hook for finding character byte type }
	UniversalProcPtr charTypeHook;		// { hook for finding character type }
	Handle hObjectHandlerTable;	// { handle to object handler table for this instance }
	WERunAttributes nullStyle;	/* style for null selection */
} WERec, *WEPtr, **WEHandle;


typedef struct WEAction {
	WEHandle hOwner;			// handle to associated WE instance
	struct WEAction **hNext;	// used to keep a linked list of actions
	Handle hText;				// handle to saved text
	Handle hStyles;				// handle to saved styles
	Handle hSoup;				// handle to saved "soup"
	long delRangeStart;			// start of range to delete
	long delRangeLength;		// length of range to delete
	long insRangeLength;		// lenfth of range to insert
	long hiliteStart;			// start of range to hilite
	long hiliteEnd;				// end of range to hilite
	WEActionKind actionKind;	// identifies event that caused this action to be pushed
	WEActionFlags actionFlags;	// miscellaneous flags
} WEAction, *WEActionPtr, **WEActionHandle;

typedef struct WEObjectDesc {
	OSType objectType;			// { 4-letter tag identifying object type }
	Handle objectDataHandle;	// { handle to object data }
	Point objectSize;			// { object height and width, in pixels }
	Handle objectTable;			// { handle to object handler table }
	short objectIndex;			// { index into object handler table (USED INTERNALLY) }
	WEHandle objectOwner;		// { handle to owner WE instance }
	long objectRefCon;			// { free for use by object handlers }
} WEObjectDesc, *WEObjectDescPtr, **WEObjectDescHandle, **WEObjectReference;

// { A WESoup record is a static description of an object embedded in the text. }
// { The 'SOUP' data type is just a collection of WESoup records, each followed }
// { by the corresponding object data. }
// { This data type complements the standard TEXT/styl pair. }

typedef struct WESoup {
	long soupOffset;			// { insertion offset for this object }
	OSType soupType;			// { 4-letter tag identifying object type }
	long soupReserved1;			// { reserved for future use; set to zero }
	Size soupDataSize;			// { size of object data following this record }
	Point soupSize;				// { object height and width, in pixels }
	long soupReserved2;			// { reserved for future use; set to zero }
} WESoup, *WESoupPtr, **WESoupHandle;

/*	callback prototypes */

typedef pascal Boolean (*WEClickLoopProcPtr)(WEHandle hWE);
typedef pascal void (*WEScrollProcPtr)(WEHandle hWE);
typedef pascal void (*WETSMPreUpdateProcPtr)(WEHandle hWE);
typedef pascal void (*WETSMPostUpdateProcPtr)(WEHandle hWE,
		long fixLength, long inputAreaStart, long inputAreaEnd,
		long pinRangeStart, long pinRangeEnd);
typedef pascal OSErr (*WETranslateDragProcPtr)(DragReference theDrag,
		ItemReference theItem, FlavorType requestedType, Handle putDataHere);
typedef pascal void (*WEDrawTextProcPtr)(Ptr pText, long textLength, Fixed slop,
		JustStyleCode styleRunPosition, WEHandle hWE);
typedef pascal long (*WEPixelToCharProcPtr)(Ptr pText, long textLength, Fixed slop,
		Fixed *pixelWidth, char *edge, JustStyleCode styleRunPosition, Fixed hPos, WEHandle hWE);
typedef pascal short (*WECharToPixelProcPtr)(Ptr pText, long textLength, Fixed slop,
		long offset, short direction, JustStyleCode styleRunPosition, long hPos, WEHandle hWE);
typedef pascal StyledLineBreakCode (*WELineBreakProcPtr)(Ptr pText, long textLength,
		long textStart, long textEnd, Fixed *textWidth, long *textOffset, WEHandle hWE);
typedef pascal void (*WEWordBreakProcPtr)(Ptr pText, short textLength, short offset,
		char edge, OffsetTable breakOffsets, ScriptCode script, WEHandle hWE);
typedef pascal short (*WECharByteProcPtr)(Ptr pText, short textOffset, ScriptCode script,
		WEHandle hWE);
typedef pascal short (*WECharTypeProcPtr)(Ptr pText, short textOffset, ScriptCode script, WEHandle hWE);

typedef pascal OSErr (*WENewObjectProcPtr)(Point *defaultObjectSize,
		WEObjectReference objectRef);
typedef pascal OSErr (*WEDisposeObjectProcPtr)(WEObjectReference objectRef);
typedef pascal OSErr (*WEDrawObjectProcPtr)(const Rect *destRect,
		WEObjectReference objectRef);
typedef pascal Boolean (*WEClickObjectProcPtr)(Point hitPt, short modifiers, long clickTime,
		WEObjectReference objectRef);


/*	UPP proc info */

enum {
	uppWEClickLoopProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
};
enum {
	uppWEScrollProcInfo = kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
};
enum {
	uppWETSMPreUpdateProcInfo = kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
};
enum {
	uppWETSMPostUpdateProcInfo = kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(long /*fixLength*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(long /*inputAreaStart*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(long /*inputAreaEnd*/)))
		| STACK_ROUTINE_PARAMETER(5,SIZE_CODE(sizeof(long /*pinRangeStart*/)))
		| STACK_ROUTINE_PARAMETER(6,SIZE_CODE(sizeof(long /*pinRangeEnd*/)))
};
enum {
	uppWETranslateDragProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(DragReference /*theDrag*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(ItemReference /*theItem*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(FlavorType /*requestedType*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(Handle /*putDataHere*/)))
};
enum {
	uppWEDrawTextProcInfo = kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Ptr /*pText*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(long /*textLength*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(Fixed /*slop*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(JustStyleCode /*styleRunPosition*/)))
		| STACK_ROUTINE_PARAMETER(5,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
};
enum {
	uppWEPixelToCharProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(long)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Ptr /*pText*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(long /*textLength*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(Fixed /*slop*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(Fixed * /*pixelWidth*/)))
		| STACK_ROUTINE_PARAMETER(5,SIZE_CODE(sizeof(char * /*edge*/)))
		| STACK_ROUTINE_PARAMETER(6,SIZE_CODE(sizeof(JustStyleCode /*styleRunPosition*/)))
		| STACK_ROUTINE_PARAMETER(7,SIZE_CODE(sizeof(Fixed /*hPos*/)))
		| STACK_ROUTINE_PARAMETER(8,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
};
enum {
	uppWECharToPixelProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(short)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Ptr /*pText*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(long /*textLength*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(Fixed /*slop*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(long /*offset*/)))
		| STACK_ROUTINE_PARAMETER(5,SIZE_CODE(sizeof(short /*direction*/)))
		| STACK_ROUTINE_PARAMETER(6,SIZE_CODE(sizeof(JustStyleCode /*styleRunPosition*/)))
		| STACK_ROUTINE_PARAMETER(7,SIZE_CODE(sizeof(long /*hPos*/)))
		| STACK_ROUTINE_PARAMETER(8,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
};
enum {
	uppWELineBreakProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(StyledLineBreakCode )))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Ptr /*pText*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(long /*textLength*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(long /*textStart*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(long /*textEnd*/)))
		| STACK_ROUTINE_PARAMETER(5,SIZE_CODE(sizeof(Fixed * /*textWidth*/)))
		| STACK_ROUTINE_PARAMETER(6,SIZE_CODE(sizeof(long * /*textOffset*/)))
		| STACK_ROUTINE_PARAMETER(7,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
};
enum {
	uppWEWordBreakProcInfo = kPascalStackBased
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Ptr /*pText*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(short /*textLength*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(short /*offset*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(char /*edge*/)))
		| STACK_ROUTINE_PARAMETER(5,SIZE_CODE(sizeof(OffsetTable * /*breakOffsets*/)))
		| STACK_ROUTINE_PARAMETER(6,SIZE_CODE(sizeof(ScriptCode * /*script*/)))
		| STACK_ROUTINE_PARAMETER(7,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
};
enum {
	uppWECharByteProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(short )))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Ptr /*pText*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(short /*textOffset*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(ScriptCode * /*script*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
};
enum {
	uppWECharTypeProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(short )))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Ptr /*pText*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(short /*textOffset*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(ScriptCode * /*script*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(WEHandle /*hWE*/)))
};


enum {
	uppWENewObjectProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Point * /*defaultObjectSize*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(WEObjectReference /*objectRef*/)))
};
enum {
	uppWEDisposeObjectProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(WEObjectReference /*objectRef*/)))
};
enum {
	uppWEDrawObjectProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(const Rect * /*destRect*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(WEObjectReference /*objectRef*/)))
};
enum {
	uppWEClickObjectProcInfo = kPascalStackBased
		| RESULT_SIZE(SIZE_CODE(sizeof(Boolean)))
		| STACK_ROUTINE_PARAMETER(1,SIZE_CODE(sizeof(Point /*hitPt*/)))
		| STACK_ROUTINE_PARAMETER(2,SIZE_CODE(sizeof(short /*modifiers*/)))
		| STACK_ROUTINE_PARAMETER(3,SIZE_CODE(sizeof(long /*clickTime*/)))
		| STACK_ROUTINE_PARAMETER(4,SIZE_CODE(sizeof(WEObjectReference /*objectRef*/)))
};


/*	UPPs, NewÅProc macros & CallÅProc macros */

/*
	NOTE:
    For compatibility with the Pascal version, CallÅProc macros take the form:

		CallFooProc(..., userRoutine)

	instead of:

		CallFooProc(userRoutine, ...)

*/

#if GENERATINGCFM

typedef UniversalProcPtr WEClickLoopUPP;
typedef UniversalProcPtr WEScrollUPP;
typedef UniversalProcPtr WETSMPreUpdateUPP;
typedef UniversalProcPtr WETSMPostUpdateUPP;
typedef UniversalProcPtr WETranslateDragUPP;
typedef UniversalProcPtr WEDrawTextUPP;
typedef UniversalProcPtr WEPixelToCharUPP;
typedef UniversalProcPtr WECharToPixelUPP;
typedef UniversalProcPtr WELineBreakUPP;
typedef UniversalProcPtr WEWordBreakUPP;
typedef UniversalProcPtr WECharByteUPP;
typedef UniversalProcPtr WECharTypeUPP;
typedef UniversalProcPtr WENewObjectUPP;
typedef UniversalProcPtr WEDisposeObjectUPP;
typedef UniversalProcPtr WEDrawObjectUPP;
typedef UniversalProcPtr WEClickObjectUPP;

#define NewWEClickLoopProc(userRoutine) \
	(WEClickLoopUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWEClickLoopProcInfo, GetCurrentArchitecture())
#define NewWEScrollProc(userRoutine) \
	(WEScrollUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWEScrollProcInfo, GetCurrentArchitecture())
#define NewWETSMPreUpdateProc(userRoutine) \
	(WETSMPreUpdateUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWETSMPreUpdateProcInfo, GetCurrentArchitecture())
#define NewWETSMPostUpdateProc(userRoutine) \
	(WETSMPostUpdateUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWETSMPostUpdateProcInfo, GetCurrentArchitecture())
#define NewWETranslateDragProc(userRoutine) \
	(WETranslateDragUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWETranslateDragProcInfo, GetCurrentArchitecture())
#define NewWEDrawTextProc(userRoutine) \
	(WEDrawTextUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWEDrawTextProcInfo, GetCurrentArchitecture())
#define NewWEPixelToCharProc(userRoutine) \
	(WEPixelToCharUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWEPixelToCharProcInfo, GetCurrentArchitecture())
#define NewWECharToPixelProc(userRoutine) \
	(WECharToPixelUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWECharToPixelProcInfo, GetCurrentArchitecture())
#define NewWELineBreakProc(userRoutine) \
	(WELineBreakUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWELineBreakProcInfo, GetCurrentArchitecture())
#define NewWEWordBreakProc(userRoutine) \
	(WEWordBreakUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWEWordBreakProcInfo, GetCurrentArchitecture())
#define NewWECharByteProc(userRoutine) \
	(WECharByteUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWECharByteProcInfo, GetCurrentArchitecture())
#define NewWECharTypeProc(userRoutine) \
	(WECharTypeUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWECharTypeProcInfo, GetCurrentArchitecture())
#define NewWENewObjectProc(userRoutine) \
	(WENewObjectUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWENewObjectProcInfo, GetCurrentArchitecture())
#define NewWEDisposeObjectProc(userRoutine) \
	(WEDisposeObjectUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWEDisposeObjectProcInfo, GetCurrentArchitecture())
#define NewWEDrawObjectProc(userRoutine) \
	(WEDrawObjectUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWEDrawObjectProcInfo, GetCurrentArchitecture())
#define NewWEClickObjectProc(userRoutine) \
	(WEClickObjectUPP) NewRoutineDescriptor((ProcPtr) (userRoutine), uppWEClickObjectProcInfo, GetCurrentArchitecture())

#define CallWEClickLoopProc(hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWEClickLoopProcInfo, (hWE))
#define CallWEScrollProc(hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWEScrollProcInfo, (hWE))
#define CallWETSMPreUpdateProc(hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWETSMPreUpdateProcInfo, (hWE))
#define CallWETSMPostUpdateProc(hWE, fixLength, inputAreaStart, inputAreaEnd, pinRangeStart, pinRangeEnd, userRoutine) \
	CallUniversalProc((userRoutine), uppWETSMPostUpdateProcInfo, (hWE), (fixLength), (inputAreaStart), (inputAreaEnd), (pinRangeStart), (pinRangeEnd))
#define CallWETranslateDragProc(theDrag, theItem, requestedType, putDataHere, userRoutine) \
	CallUniversalProc((userRoutine), uppWETranslateDragProcInfo, (theDrag), (theItem), (requestedType), (putDataHere))
#define CallWEDrawTextProc(pText, textLength, slop, styleRunPosition, hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWEDrawTextProcInfo, (pText), (textLength), (slop), (styleRunPosition), (hWE))
#define CallWEPixelToCharProc(pText, textLength, slop, pixelWidth, edge, styleRunPosition, hPos, hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWEPixelToCharProcInfo, (pText), (textLength), (slop), (pixelWidth), (edge), (styleRunPosition), (hPos), (hWE))
#define CallWECharToPixelProc(pText, textLength, slop, offset, direction, styleRunPosition, hPos, hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWECharToPixelProcInfo, (pText), (textLength), (slop), (offset), (direction), (styleRunPosition), (hPos), (hWE))
#define CallWELineBreakProc(pText, textLength, textStart, textEnd, textWidth, textOffset, hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWELineBreakProcInfo, (pText), (textLength), (textStart), (textEnd), (textWidth), (textOffset), (hWE))
#define CallWEWordBreakProc(pText, textLength, offset, edge, breakOffsets, script, hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWEWordBreakProcInfo, (pText), (textLength), (offset), (edge), (breakOffsets), (script), (hWE))
#define CallWECharByteProc(pText, textOffset, script, hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWECharByteProcInfo, (pText), (textOffset), (script), (hWE))
#define CallWECharTypeProc(pText, textOffset, script, hWE, userRoutine) \
	CallUniversalProc((userRoutine), uppWECharTypeProcInfo, (pText), (textOffset), (script), (hWE))
#define CallWENewObjectProc(defaultObjectSize, objectRef, userRoutine) \
	CallUniversalProc((userRoutine), uppWENewObjectProcInfo, (defaultObjectSize), (objectRef))
#define CallWEDisposeObjectProc(objectRef, userRoutine) \
	CallUniversalProc((userRoutine), uppWEDisposeObjectProcInfo, (objectRef))
#define CallWEDrawObjectProc(destRect, objectRef, userRoutine) \
	CallUniversalProc((userRoutine), uppWEDrawObjectProcInfo, (destRect), (objectRef))
#define CallWEClickObjectProc(hitPt, modifiers, clickTime, objectRef, userRoutine) \
	CallUniversalProc((userRoutine), uppWEClickObjectProcInfo, (hitPt), (modifiers), (clickTime), (objectRef))

#else

typedef WEClickLoopProcPtr WEClickLoopUPP;
typedef WEScrollProcPtr WEScrollUPP;
typedef WETSMPreUpdateProcPtr WETSMPreUpdateUPP;
typedef WETSMPostUpdateProcPtr WETSMPostUpdateUPP;
typedef WETranslateDragProcPtr WETranslateDragUPP;
typedef WEDrawTextProcPtr WEDrawTextUPP;
typedef WEPixelToCharProcPtr WEPixelToCharUPP;
typedef WECharToPixelProcPtr WECharToPixelUPP;
typedef WELineBreakProcPtr WELineBreakUPP;
typedef WEWordBreakProcPtr WEWordBreakUPP;
typedef WECharByteProcPtr WECharByteUPP;
typedef WECharTypeProcPtr WECharTypeUPP;
typedef WENewObjectProcPtr WENewObjectUPP;
typedef WEDisposeObjectProcPtr WEDisposeObjectUPP;
typedef WEDrawObjectProcPtr WEDrawObjectUPP;
typedef WEClickObjectProcPtr WEClickObjectUPP;

#define NewWEClickLoopProc(userRoutine) ((WEClickLoopUPP) (userRoutine))
#define NewWEScrollProc(userRoutine) ((WEScrollUPP) (userRoutine))
#define NewWETSMPreUpdateProc(userRoutine) ((WETSMPreUpdateUPP) (userRoutine))
#define NewWETSMPostUpdateProc(userRoutine) ((WETSMPostUpdateUPP) (userRoutine))
#define NewWETranslateDragProc(userRoutine) ((WETranslateDragUPP) (userRoutine))
#define NewWEDrawTextProc(userRoutine) ((WEDrawTextUPP) (userRoutine))
#define NewWEPixelToCharProc(userRoutine) ((WEPixelToCharUPP) (userRoutine))
#define NewWECharToPixelProc(userRoutine) ((WECharToPixelUPP) (userRoutine))
#define NewWELineBreakProc(userRoutine) ((WELineBreakUPP) (userRoutine))
#define NewWEWordBreakProc(userRoutine) ((WEWordBreakUPP) (userRoutine))
#define NewWECharByteProc(userRoutine) ((WECharByteUPP) (userRoutine))
#define NewWECharTypeProc(userRoutine) ((WECharTypeUPP) (userRoutine))
#define NewWENewObjectProc(userRoutine) ((WENewObjectUPP) (userRoutine))
#define NewWEDisposeObjectProc(userRoutine) ((WEDisposeObjectUPP) (userRoutine))
#define NewWEDrawObjectProc(userRoutine) ((WEDrawObjectUPP) (userRoutine))
#define NewWEClickObjectProc(userRoutine) ((WEClickObjectUPP) (userRoutine))

#define CallWEClickLoopProc(hWE, userRoutine) \
	(*(userRoutine))((hWE))
#define CallWEScrollProc(hWE, userRoutine) \
	(*(userRoutine))((hWE))
#define CallWETSMPreUpdateProc(hWE, userRoutine) \
	(*(userRoutine))((hWE))
#define CallWETSMPostUpdateProc(hWE, fixLength, inputAreaStart, inputAreaEnd, pinRangeStart, pinRangeEnd, userRoutine) \
	(*(userRoutine))((hWE), (fixLength), (inputAreaStart), (inputAreaEnd), (pinRangeStart), (pinRangeEnd))
#define CallWETranslateDragProc(theDrag, theItem, requestedType, putDataHere, userRoutine) \
	(*(userRoutine))((theDrag), (theItem), (requestedType), (putDataHere))
#define CallWEDrawTextProc(pText, textLength, slop, styleRunPosition, hWE, userRoutine) \
	(*(userRoutine))((pText), (textLength), (slop), (styleRunPosition), (hWE))
#define CallWEPixelToCharProc(pText, textLength, slop, pixelWidth, edge, styleRunPosition, hPos, hWE, userRoutine) \
	(*(userRoutine))((pText), (textLength), (slop), (pixelWidth), (edge), (styleRunPosition), (hPos), (hWE))
#define CallWECharToPixelProc(pText, textLength, slop, offset, direction, styleRunPosition, hPos, hWE, userRoutine) \
	(*(userRoutine))((pText), (textLength), (slop), (offset), (direction), (styleRunPosition), (hPos), (hWE))
#define CallWELineBreakProc(pText, textLength, textStart, textEnd, textWidth, textOffset, hWE, userRoutine) \
	(*(userRoutine))((pText), (textLength), (textStart), (textEnd), (textWidth), (textOffset), (hWE))
#define CallWEWordBreakProc(pText, textLength, offset, edge, breakOffsets, script, hWE, userRoutine) \
	(*(userRoutine))((pText), (textLength), (offset), (edge), (breakOffsets), (script), (hWE))
#define CallWECharByteProc(pText, textOffset, script, hWE, userRoutine) \
	(*(userRoutine))((pText), (textOffset), (script), (hWE))
#define CallWECharTypeProc(pText, textOffset, script, hWE, userRoutine) \
	(*(userRoutine))((pText), (textOffset), (script), (hWE))
#define CallWENewObjectProc(defaultObjectSize, objectRef, userRoutine) \
	(*(userRoutine))((defaultObjectSize), (objectRef))
#define CallWEDisposeObjectProc(objectRef, userRoutine) \
	(*(userRoutine))((objectRef))
#define CallWEDrawObjectProc(destRect, objectRef, userRoutine) \
	(*(userRoutine))((destRect), (objectRef))
#define CallWEClickObjectProc(hitPt, modifiers, clickTime, objectRef, userRoutine) \
	(*(userRoutine))((hitPt), (modifiers), (clickTime), (objectRef))

#endif

typedef Boolean (*SegmentLoopProcPtr)
				(LinePtr pLine,
				 WERunAttributesPtr pAttrs,
				 Ptr pSegment,
				 long segmentStart,
				 long segmentLength,
				 JustStyleCode styleRunPosition,
				 void *callbackData);

struct SLDrawData {
	WEHandle hWE;
	Rect bounds;				/* same rectangle, but in global coords */
	Boolean usingColor;			/* true if drawing in color */
	Boolean usingOffscreen;		/* true if an offscreen graphics world has been set up */
	Boolean drawingOffscreen;	/* true if actually drawing to an offscreen buffer */
	Boolean doErase;

	Rect lineRect;				// rect enclosing current line mf
	Rect drawRect;				// visible portion of line rect mf
	PixMapHandle offscreenPixels; // mf
	GrafPtr screenPort;			// DWC 5/18/95
	GDHandle screenDevice;
};

struct SLCalcSlopData {
	WEPtr pWE;
	short lineWidth;
	short totalSlop;
	Fixed totalProportion;
};

struct SLPixelToCharData {
	WEHandle hWE;
	Fixed hPos;
	Fixed pixelWidth;
	long offset;
	char *edge;
};

struct SLCharToPixelData {
	WEHandle hWE;
	long offset;
	LongPt *thePoint;
};

typedef struct WEFieldDescriptor {
	short fOffset;
	short fLength;
} WEFieldDescriptor;

typedef struct WELookupTable {
	OSType selector;
	WEFieldDescriptor desc;
} WELookupTable;

#if defined(powerc) || defined (__powerc)
#pragma options align=reset
#endif


// Routines in WEUtilities.c

pascal void _WEForgetHandle(Handle *h);
pascal Boolean _WESetHandleLock(Handle h, Boolean lock);
pascal void _WEBlockClr(Ptr block, long blockSize);
pascal Boolean _WEBlockCmp(Ptr block1, Ptr block2, long blockSize);
pascal void _WEReorder(long *a, long *b);
pascal OSErr _WEAllocate(Size blockSize, short allocFlags, Handle *h);

// in WEArrays.c
pascal OSErr _WEInsertSlot(Handle h, Ptr element, long insertAt, long slotSize);
pascal OSErr _WERemoveSlot(Handle h, long removeAt, long slotSize);

// in WEBirthDeath.c
pascal OSErr _WERegisterWithTSM(WEHandle hWE);
pascal void _WESetStandardHooks(WEHandle hWE);
pascal OSErr WENew(LongRect *destRect, LongRect *viewRect, short flags, WEHandle *hWE);
pascal void WEDispose(WEHandle hWE);
pascal void _WEStdDrawText(Ptr pText, long textLength, Fixed slop, 
				JustStyleCode styleRunPosition, WEHandle hWE);
pascal long _WEStdPixelToChar(Ptr pText, long textLength, Fixed slop,
				Fixed *width, char *edge, JustStyleCode styleRunPosition,
				Fixed hPos, WEHandle hWE);
pascal short _WEStdCharToPixel(Ptr pText, long textLength, Fixed slop,
				long offset, short direction, JustStyleCode styleRunPosition,
				long hPos, WEHandle hWE);
pascal StyledLineBreakCode _WEStdLineBreak(Ptr pText, long textLength,
				long textStart, long textEnd, Fixed *textWidth,
				long *textOffset, WEHandle hWE);
pascal void _WEStdWordBreak(Ptr pText, short textLength, short offset,
				char edge, OffsetTable breakOffsets, ScriptCode script,
				WEHandle hWE);
pascal short _WEStdCharByte(Ptr pText, short textLength, ScriptCode script,
				WEHandle hWE);
pascal short _WEStdCharType(Ptr pText, short textLength, ScriptCode script,
				WEHandle hWE);
pascal short _WEScriptToFont(ScriptCode script);
pascal void _WEOldWordBreak(Ptr pText, short textLength, short offset,
				char edge, OffsetTable breakOffsets, ScriptCode script,
				WEHandle hWE);
pascal short _WEOldCharByte(Ptr pText, short textLength, ScriptCode script,
				WEHandle hWE);
pascal short _WEOldCharType(Ptr pText, short textLength, ScriptCode script,
				WEHandle hWE);

// in WEDebug.c
pascal void _WEAssert(Boolean condition, StringPtr message);
pascal void _WESanityCheck(WEHandle hWE);

// in WEInlineInput.c
pascal OSErr _WEHiliteRangeArray(TextRangeArrayHandle hTray, WEHandle hWE);
pascal OSErr _WEHandleUpdateActiveInputArea(AppleEvent *ae, AppleEvent *reply,
		long handlerRefCon);
pascal OSErr _WEHandlePositionToOffset(AppleEvent *ae, AppleEvent *reply,
		long handlerRefCon);
pascal OSErr _WEHandleOffsetToPosition(AppleEvent *ae, AppleEvent *reply, long handlerRefCon);
pascal OSErr WEInstallTSMHandlers(void);
pascal OSErr WERemoveTSMHandlers(void);

// in WELongCoords.c
pascal long _WEPinInRange(long value, long rangeStart, long rangeEnd);
pascal void WELongPointToPoint(const LongPt *lp, Point *p);
pascal void WEPointToLongPoint(Point p, LongPt *lp);
pascal void WESetLongRect(LongRect *lr, long left, long top, long right, long bottom);
pascal void WELongRectToRect(const LongRect *lr, Rect *r);
pascal void WERectToLongRect(const Rect *r, LongRect *lr);
pascal void WEOffsetLongRect(LongRect *lr, long hOffset, long vOffset);
pascal Boolean WELongPointInLongRect(const LongPt *lp, const LongRect *lr);

// in WEDrawing.c
pascal long WEOffsetToLine (long offset, WEHandle hWE);
pascal long _WEPixelToLine(long vOffset, WEHandle hWE);
pascal long _WEOffsetToRun (long offset, WEHandle hWE);
pascal void _WEGetIndStyle(long runIndex, WERunInfo *info, WEHandle hWE);
pascal void WEGetRunInfo(long offset, WERunInfo *info, WEHandle hWE);
pascal OSErr WEGetSelectedObject(WEObjectDescHandle *hObjectDesc, WEHandle hWE);
pascal long WEFindNextObject(long offset, WEObjectDescHandle *hObjectDesc, WEHandle hWE);
pascal void _WEContinuousStyleRange(long rangeStart, long rangeEnd, short *mode,
		WETextStyle *ts, WEHandle hWE);
pascal void _WESynchNullStyle(WEHandle hWE);
pascal Boolean WEContinuousStyle (short *mode, TextStyle *ts, WEHandle hWE);
pascal void _WESegmentLoop(long firstLine, long lastLine, SegmentLoopProcPtr callback, void *callbackData,
		WEHandle hWE);
pascal void _WEDrawTSMHilite(Rect *segmentRect, short tsFlags);
pascal void _WEDrawLines (long firstLine, long lastLine, Boolean doErase, WEHandle hWE);
pascal short _WECalcPenIndent(short slop, short alignment);
pascal void _WESaveQDEnvironment(GrafPtr port, Boolean saveColor, QDEnvironment *theEnvironment);
pascal void _WERestoreQDEnvironment(QDEnvironment *theEnvironment);
pascal void _WEFillFontInfo (GrafPtr port, WERunAttributes *targetStyle);
pascal void _WECopyStyle (WETextStyle *sourceStyle, WETextStyle *targetStyle, short offStyles,
		short mode);
pascal Boolean _WEOffsetInRange(long offset, char edge, long rangeStart, long rangeEnd);

/*** in WELineLayout.c ***/
pascal void WEStopInlineSession(WEHandle hWE);
pascal OSErr _WERemoveLine(long lineIndex, WEPtr pWE);
pascal OSErr _WEInsertLine(long lineIndex, LineRec *theLine, WEPtr pWE);
pascal void _WEBumpOrigin(long lineIndex, long deltaOrigin, WEPtr pWE);
pascal long _WEFindLineBreak(long lineStart, WEHandle hWE);
pascal void _WECalcHeights(long rangeStart, long rangeEnd, short *lineAscent, short *lineDescent,
		WEHandle hWE);
pascal OSErr _WERecalBreaks(long *startLine, long *endLine, WEHandle hWE);
pascal void _WERecalSlops(long firstLine, long lastLine, WEHandle hWE);
pascal OSErr WECalText(WEHandle hWE);
pascal OSErr WEUseText(Handle text, WEHandle hWE);
pascal char WEGetAlignment(WEHandle hWE);
pascal void WEGetSelection(long *selStart, long *selEnd, WEHandle hWE);
pascal void WESetDestRect(LongRect *destRect, WEHandle hWE);
pascal void WEGetDestRect(LongRect *destRect, WEHandle hWE);
pascal void WESetViewRect(LongRect *viewRect, WEHandle hWE);
pascal void WEGetViewRect(LongRect *viewRect, WEHandle hWE);
pascal long WEGetTextLength(WEHandle hWE);
pascal long WECountLines(WEHandle hWE);
pascal long WEGetHeight(long startLine, long endLine, WEHandle hWE);
pascal Handle WEGetText(WEHandle hWE);
pascal short WEGetChar(long offset, WEHandle hWE);
pascal short WEFeatureFlag(short feature, short action, WEHandle hWE);

// WESelecting.c
pascal void _WEClearHiliteBit(void);
pascal long WEGetOffset(const LongPt *thePoint, char *edge, WEHandle hWE);
pascal void WEGetPoint(long offset, LongPt *thePoint, short *lineHeight, WEHandle hWE);
pascal void WEFindLine(long offset, char edge, long *lineStart, long *lineEnd, WEHandle hWE);
pascal long _WEGetLineStart(long lineNo, WEHandle hWE);
pascal short _WEGetContext(long offset, long *contextStart, long *contextEnd,
						WEHandle hWE);
pascal short _WEGetRestrictedContext(long offset, long *contextStart, long *contextEnd,
						WEHandle hWE);
pascal void WEFindWord(long offset, char edge, long *wordStart, long *wordEnd, WEHandle hWE);
pascal short WECharByte(long offset, WEHandle hWE);
pascal short WECharType(long offset, WEHandle hWE);
pascal void _WEDrawCaret(long offset, WEHandle hWE);
pascal void _WEBlinkCaret(WEHandle hWE);
pascal RgnHandle WEGetHiliteRgn(long rangeStart, long rangeEnd, WEHandle hWE);
pascal void _WEHiliteRange(long rangeStart, long rangeEnd, WEHandle hWE);
pascal void WESetSelection(long selStart, long selEnd, WEHandle hWE);
pascal void WESetAlignment(char alignment, WEHandle hWE);
pascal long _WEArrowOffset(short action, long offset, WEHandle hWE);
pascal void _WEDoArrowKey (short arrow, short modifiers, WEHandle hWE);
pascal Boolean WEAdjustCursor(Point mouseLoc, RgnHandle mouseRgn, WEHandle hWE);
pascal void WEIdle(long *maxSleep, WEHandle hWE);
pascal void WEUpdate(RgnHandle updateRgn, WEHandle hWE);
pascal void WEDeactivate(WEHandle hWE);
pascal void WEActivate(WEHandle hWE);
pascal Boolean WEIsActive(WEHandle hWE);
pascal void WEScroll(long hOffset, long vOffset, WEHandle hWE);
pascal Boolean _WEScrollIntoView (long offset, WEHandle hWE);
pascal void WESelView(WEHandle hWE);

// WELowLevelEditing.c
pascal Boolean _WEIsWordRange(long rangeStart, long rangeEnd, WEHandle hWE);
pascal Boolean _WEIsPunct(long offset, WEHandle hWE);
pascal void _WEIntelligentCut(long *rangeStart, long *rangeEnd, WEHandle hWE);
pascal short _WEIntelligentPaste(long rangeStart, long rangeEnd, WEHandle hWE);
pascal OSErr _WEInsertRun(long runIndex, long offset, long styleIndex, WEPtr pWE);
pascal OSErr _WERemoveRun(long runIndex, WEPtr pWE);
pascal void _WEChangeRun(long runIndex, long newStyleIndex, Boolean keepOld, WEPtr pWE);
pascal OSErr _WENewStyle(WERunAttributes *ts, long *styleIndex, WEPtr pWE);
pascal OSErr _WERedraw(long rangeStart, long rangeEnd, WEHandle hWE);
pascal OSErr _WESetStyleRange(long rangeStart, long rangeEnd, short mode, WETextStyle *ts, WEHandle hWE);
pascal OSErr _WEApplyStyleScrap(long rangeStart, long rangeEnd, StScrpHandle styleScrap, WEHandle hWE);
pascal OSErr _WEApplySoup(long offset, Handle hSoup, WEHandle hWE);
pascal void _WEBumpRunStart(long runIndex, long deltaRunStart, WEPtr pWE);
pascal OSErr _WERemoveRunRange(long rangeStart, long rangeEnd, WEHandle hWE);
pascal void _WEBumpLineStart(long lineIndex, long deltaLineStart, WEPtr pWE);
pascal OSErr _WERemoveLineRange(long rangeStart, long rangeEnd, WEHandle hWE);
pascal OSErr _WEDeleteRange(long rangeStart, long rangeEnd, WEHandle hWE);
pascal OSErr _WEInsertText(long offset, Ptr textPtr, long textLength, WEHandle hWE);

// WEHighLevelEditing.c
pascal WEActionHandle WEGetActionStack(WEHandle hWE);
pascal OSErr WEPushAction(WEActionHandle hAction);
pascal OSErr WENewAction(long rangeStart, long rangeEnd, long newTextLength,
							WEActionKind actionKind, WEActionFlags actionFlags,
							WEHandle hWE, WEActionHandle *hAction);
pascal void WEDisposeAction(WEActionHandle hAction);
pascal void WEForgetAction(WEActionHandle *hAction);
pascal OSErr WEDoAction(WEActionHandle hAction);
pascal OSErr WEUndo(WEHandle hWE);
pascal void WEClearUndo(WEHandle hWE);
pascal WEActionKind WEGetUndoInfo(Boolean *redoFlag, WEHandle hWE);
pascal long WEGetModCount(WEHandle hWE);
pascal void WEResetModCount(WEHandle hWE);
pascal void _WEAdjustUndoRange(long moreBytes, WEHandle hWE);
pascal OSErr _WETypeChar(char theByte, WEHandle hWE);
pascal OSErr _WEBackspace(WEHandle hWE);
pascal OSErr _WEForwardDelete(WEHandle hWE);
pascal Boolean WEIsTyping(WEHandle hWE);
pascal void WEKey(short key, short modifiers, WEHandle hWE);
pascal OSErr WEInsert(Ptr textPtr, long textLength, StScrpHandle hStyles, Handle hSoup, WEHandle hWE);
pascal OSErr WEInsertObject(OSType objectType, Handle objectDataHandle, Point objectSize, WEHandle hWE);
pascal OSErr WEDelete(WEHandle hWE);
pascal OSErr WECut(WEHandle hWE);
pascal Boolean WECanPaste(WEHandle hWE);
pascal OSErr WEPaste(WEHandle hWE);
pascal OSErr WESetStyle(short mode, TextStyle *ts, WEHandle hWE);
pascal OSErr WEUseStyleScrap(StScrpHandle hStyles, WEHandle hWE);

// WEMouse.c
pascal Boolean _WEIsOptionDrag(DragReference theDrag);
pascal OSErr _WEGetFlavor(DragReference theDrag, ItemReference theItem,
				FlavorType requestedType, Handle hFlavor,
				WETranslateDragUPP translateDragHook);
pascal OSErr _WEExtractFlavor(DragReference theDrag, ItemReference theItem,
						FlavorType theType, Handle *hFlavor,
						WETranslateDragUPP translateDragHook);
pascal Boolean WECanAcceptDrag(DragReference theDrag, WEHandle hWE);
pascal void _WEUpdateDragCaret(long offset, WEHandle hWE);
pascal OSErr WETrackDrag(DragTrackingMessage theMessage,DragReference theDrag,
									WEHandle hWE);
pascal OSErr WEReceiveDrag(DragReference theDrag, WEHandle hWE);
pascal OSErr _WESendFlavor(FlavorType theType, void *dragSendRefCon, Handle hWE,
					DragReference theDrag);
pascal Boolean WEDraggedToTrash(DragReference theDrag);
pascal OSErr _WEDrag(Point mouseLoc, short modifiers, long clickTime, WEHandle hWE);
pascal void WEClick(Point mouseLoc, short modifiers, long clickTime, WEHandle hWE);

// WEObjects.c
pascal OSType WEGetObjectType(WEObjectDescHandle hObjectDesc);
pascal Handle WEGetObjectDataHandle(WEObjectDescHandle hObjectDesc);
pascal Point WEGetObjectSize(WEObjectDescHandle hObjectDesc);
pascal WEHandle WEGetObjectOwner(WEObjectDescHandle hObjectDesc);
pascal long WEGetObjectRefCon(WEObjectDescHandle hObjectDesc);
pascal void WESetObjectRefCon(WEObjectDescHandle hObjectDesc, long refCon);
pascal short _WELookupObjectType(OSType objectType, Handle hTable);
pascal OSErr _WEGetIndObjectType(short index, OSType *objectType, WEHandle hWE);
pascal OSErr _WENewObject(OSType objectType, Handle objectDataHandle, WEHandle hWE,
					WEObjectDescHandle *hObjectDesc);
pascal OSErr _WEFreeObject(WEObjectDescHandle hObjectDesc);
pascal OSErr _WEDrawObject(WEObjectDescHandle hObjectDesc);
pascal Boolean _WEClickObject(Point hitPt, short modifiers, long clickTime,
								WEObjectDescHandle hObjectDesc);
pascal OSErr WEInstallObjectHandler(OSType objectType, OSType handlerSelector, UniversalProcPtr handler,
				WEHandle hWE);

// WEScraps.c
pascal OSErr _WEPrependStyle(Handle hStyleScrap, WERunInfo *info, long offsetDelta);
pascal OSErr _WEAppendStyle(Handle hStyleScrap, WERunInfo *info, long offset);
pascal OSErr _WEPrependObject(Handle hSoup, WERunInfo *info, long offsetDelta);
pascal OSErr _WEAppendObject(Handle hSoup, WERunInfo *info, long offset);
pascal OSErr WECopyRange(long rangeStart, long rangeEnd, Handle hText, Handle
					hStyles, Handle hSoup, WEHandle hWE);
pascal OSErr WECopy(WEHandle hWE);


/*** in WESelectors.c ***/
#define weUndefinedSelectorErr			-50 /* paramErr */

pascal void _WELookupSelector(WELookupTable *table, OSType selector, WEFieldDescriptor *desc);
pascal OSErr _WEGetField(WELookupTable *table, OSType selector, long *info, void *structure);
pascal OSErr _WESetField(WELookupTable *table, OSType selector, long *info, void *structure);
pascal OSErr WEGetInfo(OSType selector, void *info, WEHandle hWE);
pascal OSErr WESetInfo(OSType selector, const void *info, WEHandle hWE);
pascal OSErr _WESetHandler(OSType selector, long *info, void *structure);


