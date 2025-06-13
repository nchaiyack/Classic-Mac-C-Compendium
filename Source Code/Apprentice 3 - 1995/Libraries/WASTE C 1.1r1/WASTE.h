/*
 *	WASTE.h
 *
 *	C/C++ interface to the WASTE text engine
 *
 *	version 1.1b1 (June 1995)
 *
 *	Copyright (c) 1993-1995 Marco Piovanelli
 *	All Rights Reserved
 * 
 */

#ifndef _WASTE_
#define _WASTE_

#ifndef __CONDITIONALMACROS__
#include <ConditionalMacros.h>
#endif

#ifndef __TYPES__
#include <Types.h>
#endif

#ifndef __MIXEDMODE__
#include <MixedMode.h>
#endif

#ifndef __QUICKDRAWTEXT__
#include <QuickdrawText.h>
#endif

#ifndef __QUICKDRAW__
#include <Quickdraw.h>
#endif

#ifndef __SCRIPT__
#include <Script.h>
#endif

#ifndef __TEXTUTILS__
#include <TextUtils.h>
#endif

#ifndef __TEXTEDIT__
#include <TextEdit.h>
#endif

#ifndef __DRAG__
#include <Drag.h>
#endif

#ifndef __LONGCOORDINATES__
#ifndef _LongCoords_
#include "LongCoords.h"
#endif
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=mac68k
#endif

#define WASTE11

/*	result codes */

enum {
	weCantUndoErr				=	-10015,	/* undo buffer is clear (= errAECantUndo) */
	weEmptySelectionErr			=	-10013,	/* selection range is empty (= errAENoUserSelection) */
	weUnknownObjectTypeErr		=	-9478,	/* specified object type is not registered */
	weObjectNotFoundErr			=	-9477,	/* no object found at specified offset */
	weReadOnlyErr				=	-9476,	/* instance is read-only */
	weUndefinedSelectorErr		=	-50		/* unknown selector (= paramErr) */
};

/*	alignment styles */

enum {
	weFlushLeft 		=	-2,		/* flush left */
	weFlushRight		=	-1,		/* flush right */
	weFlushDefault		=	 0,		/* flush according to system direction */
	weCenter			=	 1,		/* centered */
	weJustify			=	 2		/* fully justified */
};

/*	values for the mode parameter in WESetStyle and WEContinuousStyle */

enum {
	weDoFont			=	0x0001,
	weDoFace			=	0x0002,
	weDoSize			=	0x0004,
	weDoColor			=	0x0008,
	weDoAll				=	weDoFont + weDoFace + weDoSize + weDoColor,
	weDoAddSize			=	0x0010,
	weDoToggleFace		=	0x0020,
	weDoReplaceFace		=	0x0040
};

/*	values for the edge parameter in WEGetOffset etc. */

enum {
	kLeadingEdge = -1,				/* point is on the leading edge of a glyph */
	kTrailingEdge = 0,				/* point is on the trailing edge of a glyph */
	kObjectEdge = 2					/* point is in the middle of an embedded object */
};

/*	values for WEFeatureFlag feature parameter */

enum {
	weFAutoScroll		=	0,		/* automatically scroll the selection range into view */
	weFOutlineHilite	=	2,		/* frame selection when deactivated */
	weFReadOnly			=	5,		/* disallow modifications */
	weFUndo				=	6,		/* support WEUndo() */
	weFIntCutAndPaste	=	7,		/* use intelligent cut-and-paste rules */
	weFDragAndDrop		=	8,		/* support drag-and-drop text editing */
	weFInhibitRecal		=	9,		/* don't recalculate line starts and don't redraw text */
	weFUseTempMem		=	10,		/* use temporary memory for main data structures */
	weFDrawOffscreen	=	11		/* draw text offscreen for smoother visual results */
};

/*	values for WENew flags parameter */

enum {
	weDoAutoScroll		=	1 << weFAutoScroll,
	weDoOutlineHilite	=	1 << weFOutlineHilite,
	weDoReadOnly		=	1 << weFReadOnly,
	weDoUndo			=	1 << weFUndo,
	weDoIntCutAndPaste	=	1 << weFIntCutAndPaste,
	weDoDragAndDrop		=	1 << weFDragAndDrop,
	weDoInhibitRecal	=	1 << weFInhibitRecal,
	weDoUseTempMem		=	1 << weFUseTempMem,
	weDoDrawOffscreen	=	1 << weFDrawOffscreen
};

/*	values for WEFeatureFlag action parameter */

enum {
	weBitToggle = -2,			/* toggles the specified feature */
	weBitTest,					/* returns the current setting of the specified feature */
	weBitClear,					/* disables the specified feature */
	weBitSet					/* enables the specified feature */
};

/*	selectors for WEGetInfo and WESetInfo */

enum {
	weCharToPixelHook	=	'c2p ',	/* CharToPixel hook */
	weClickLoop			=	'clik',	/* click loop callback */
	weCurrentDrag		=	'drag',	/* drag currently being tracked from WEClick() */
	weDrawTextHook		=	'draw', /* text drawing hook */
	weLineBreakHook		=	'lbrk',	/* line breaking hook */
	wePixelToCharHook	=	'p2c ', /* PixelToChar hook */
	wePort				=	'port',	/* graphics port */
	weRefCon			=	'refc',	/* reference constant for use by application */
	weScrollProc		=	'scrl',	/* auto-scroll callback */
	weText				=	'text',	/* text handle */
	weTranslateDragHook =	'xdrg', /* drag translation callback */
	weTSMDocumentID		=	'tsmd',	/* Text Services Manager document ID */
	weTSMPreUpdate		=	'pre ',	/* Text Services Manager pre-update callback */
	weTSMPostUpdate		=	'post'	/* Text Services Manager post-update callback */
};

/*	values for WEInstallObjectHandler handlerSelector parameter */

enum {
	weNewHandler		=	'new ',		/* new handler */
	weDisposeHandler	=	'free',		/* dispose handler */
	weDrawHandler		=	'draw',		/* draw handler */
	weClickHandler		=	'clik'		/* click handler */
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

typedef Handle WEHandle;
typedef Handle WEObjectReference;
typedef Handle WESoupHandle;
typedef short WEActionKind;

typedef struct WERunInfo {
	long 				runStart;	/* byte offset to first character of style run */
	long 				runEnd;		/* byte offset past last character of style run */
	short 				runHeight;	/* line height (ascent + descent + leading) */
	short 				runAscent;	/* font ascent */
	TextStyle 			runStyle;	/* text attributes */
	WEObjectReference	runObject;	/* either NULL or reference to embedded object */
} WERunInfo;


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


/*	WASTE public calls */

#ifdef __cplusplus
extern "C" {
#endif

/*	creation and destruction */

pascal OSErr WENew(const LongRect *destRect, const LongRect *viewRect, short flags, WEHandle *hWE);
pascal void WEDispose(WEHandle hWE);

/*	getting variables */

pascal Handle WEGetText(WEHandle hWE);
pascal short WEGetChar(long offset, WEHandle hWE);
pascal long WEGetTextLength(WEHandle hWE);
pascal long WECountLines(WEHandle hWE);
pascal long WEGetHeight(long startLine, long endLine, WEHandle hWE);
pascal void WEGetSelection(long *selStart, long *selEnd, WEHandle hWE);
pascal void WEGetDestRect(LongRect *destRect, WEHandle hWE);
pascal void WEGetViewRect(LongRect *viewRect, WEHandle hWE);
pascal Boolean WEIsActive(WEHandle hWE);
pascal long WEOffsetToLine (long offset, WEHandle hWE);

/*	setting variables */

pascal void WESetSelection(long selStart, long selEnd, WEHandle hWE);
pascal void WESetDestRect(const LongRect *destRect, WEHandle hWE);
pascal void WESetViewRect(const LongRect *viewRect, WEHandle hWE);

/*	accessing style run information */

pascal Boolean WEContinuousStyle(short *mode, TextStyle *ts, WEHandle hWE);
pascal void WEGetRunInfo(long offset, WERunInfo *runInfo, WEHandle hWE);

/*	converting byte offsets to screen position and vice versa */

pascal long WEGetOffset(const LongPt *thePoint, char *edge, WEHandle hWE);
pascal void WEGetPoint(long offset, LongPt *thePoint, short *lineHeight, WEHandle hWE);

/*	finding words and lines */

pascal void WEFindWord(long offset, char edge, long *wordStart, long *wordEnd, WEHandle hWE);
pascal void WEFindLine(long offset, char edge, long *lineStart, long *lineEnd, WEHandle hWE);

/*	making a copy of a text range */

pascal OSErr WECopyRange(long rangeStart, long rangeEnd, Handle hText, StScrpHandle hStyles, WESoupHandle hSoup, WEHandle hWE);

/*	getting and setting the alignment style */

pascal char WEGetAlignment(WEHandle hWE);
pascal void WESetAlignment(char alignment, WEHandle hWE);

/*	recalculating line breaks, drawing and scrolling */

pascal OSErr WECalText(WEHandle hWE);
pascal void WEUpdate(RgnHandle updateRgn, WEHandle hWE);
pascal void WEScroll(long hOffset, long vOffset, WEHandle hWE);
pascal void WESelView(WEHandle hWE);

/*	handling activate / deactivate events */

pascal void WEActivate(WEHandle hWE);
pascal void WEDeactivate(WEHandle hWE);

/* 	handling key-down events */

pascal void WEKey(short key, short modifiers, WEHandle hWE);

/*	handling mouse-down events and mouse tracking */

pascal void WEClick(Point hitPt, short modifiers, long clickTime, WEHandle hWE);

/*	adjusting the cursor shape */

pascal Boolean WEAdjustCursor(Point mouseLoc, RgnHandle mouseRgn, WEHandle hWE);

/*	blinking the caret */

pascal void WEIdle(long *maxSleep, WEHandle hWE);

/*	modifying the text and the styles */

pascal OSErr WEInsert(const void *pText, long textLength, StScrpHandle hStyles, WESoupHandle hSoup, WEHandle hWE);
pascal OSErr WEDelete(WEHandle hWE);
pascal OSErr WESetStyle(short mode, const TextStyle *ts, WEHandle hWE);
pascal OSErr WEUseStyleScrap(StScrpHandle hStyles, WEHandle hWE);
pascal OSErr WEUseText(Handle hText, WEHandle hWE);

/*	undo */

pascal OSErr WEUndo(WEHandle hWE);
pascal void WEClearUndo(WEHandle hWE);
pascal WEActionKind WEGetUndoInfo(Boolean *redoFlag, WEHandle hWE);
pascal Boolean WEIsTyping(WEHandle hWE);

/*	keeping track of changes */

pascal long WEGetModCount(WEHandle hWE);
pascal void WEResetModCount(WEHandle hWE);

/*	embedded objects */

pascal OSErr WEInstallObjectHandler(OSType objectType, OSType handlerSelector, UniversalProcPtr handler, WEHandle hWE);
pascal OSErr WEInsertObject(OSType objectType, Handle objectDataHandle, Point objectSize, WEHandle hWE);
pascal OSErr WEGetSelectedObject(WEObjectReference *objectRef, WEHandle hWE);
pascal long WEFindNextObject(long offset, WEObjectReference *objectRef, WEHandle hWE);

/*	accessing embedded object attributes */

pascal OSType WEGetObjectType(WEObjectReference objectRef);
pascal Handle WEGetObjectDataHandle(WEObjectReference objectRef);
pascal Point WEGetObjectSize(WEObjectReference objectRef);
pascal WEHandle WEGetObjectOwner(WEObjectReference objectRef);
pascal long WEGetObjectRefCon(WEObjectReference objectRef);
pascal void WESetObjectRefCon(WEObjectReference objectRef, long refCon);

/*	clipboard operations */

pascal OSErr WECut(WEHandle hWE);
pascal OSErr WECopy(WEHandle hWE);
pascal OSErr WEPaste(WEHandle hWE);
pascal Boolean WECanPaste(WEHandle hWE);

/*	Drag Manager support */

pascal RgnHandle WEGetHiliteRgn(long rangeStart, long rangeEnd, WEHandle hWE);
pascal OSErr WETrackDrag(DragTrackingMessage message, DragReference drag, WEHandle hWE);
pascal OSErr WEReceiveDrag(DragReference drag, WEHandle hWE);
pascal Boolean WECanAcceptDrag(DragReference drag, WEHandle hWE);
pascal Boolean WEDraggedToTrash(DragReference drag);

/*	Script Manager utilities */

pascal short WECharByte(long offset, WEHandle hWE);
pascal short WECharType(long offset, WEHandle hWE);

/*	Text Services Manager support */

pascal OSErr WEInstallTSMHandlers(void);
pascal OSErr WERemoveTSMHandlers(void);
pascal void WEStopInlineSession(WEHandle hWE);

/*	additional features */

pascal short WEFeatureFlag(short feature, short action, WEHandle hWE);
pascal OSErr WEGetInfo(OSType selector, void *info, WEHandle hWE);
pascal OSErr WESetInfo(OSType selector, const void *info, WEHandle hWE);

/*	long coordinate utilities */

pascal void WELongPointToPoint(const LongPt *lp, Point *p);
pascal void WEPointToLongPoint(Point p, LongPt *lp);
pascal void WESetLongRect(LongRect *lr, long left, long top, long right, long bottom);
pascal void WELongRectToRect(const LongRect *lr, Rect *r);
pascal void WERectToLongRect(const Rect *r, LongRect *lr);
pascal void WEOffsetLongRect(LongRect *lr, long hOffset, long vOffset);
pascal Boolean WELongPointInLongRect(const LongPt *lp, const LongRect *lr);

#ifdef __cplusplus
}
#endif

#if defined(powerc) || defined (__powerc)
#pragma options align=reset
#endif

#endif
