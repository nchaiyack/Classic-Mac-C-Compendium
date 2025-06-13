/* Utils.h
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */
#ifndef _H_UTILS
#define _H_UTILS

typedef void (*ThumbInitProcPtr)(ControlHandle);
typedef void (*ThumbDoneProcPtr)(ControlHandle, Integer);

/* The following was left out of Controls.h:
 */
typedef pascal void (*ControlIndicatorProcPtr)(void);

enum {
	uppControlIndicatorProcInfo = kPascalStackBased
};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr ControlIndicatorUPP;

#define CallControlIndicatorProc(userRoutine)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppControlIndicatorProcInfo)
#define NewControlIndicatorProc(userRoutine)		\
		(ControlIndicatorUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppControlIndicatorProcInfo, GetCurrentISA())
#else
typedef ControlIndicatorProcPtr ControlIndicatorUPP;

#define CallControlIndicatorProc(userRoutine)		\
		(*(userRoutine))()
#define NewControlIndicatorProc(userRoutine)		\
		(ControlIndicatorUPP)(userRoutine)
#endif

/* Handle scroll bars, using universal procedure pointers:
 */
typedef struct ScrollBarClassRec{
	ControlActionProcPtr	upProc;
	ControlActionProcPtr	downProc;
	ControlActionProcPtr	pageUpProc;
	ControlActionProcPtr	pageDownProc;
	ControlIndicatorProcPtr	thumbProc;
	ThumbInitProcPtr		thumbInit;
	ThumbDoneProcPtr		thumbDone;
	ControlActionUPP		up;			/* filled in at runtime by InitScrollBarClass */
	ControlActionUPP		down;
	ControlActionUPP		pageUp;
	ControlActionUPP		pageDown;
	ControlIndicatorUPP		thumb;
}ScrollBarClassRec, *ScrollBarClassPtr;

#define kEnable 0	/* used for control hiliting */
#define kDisable 255

/* *** arithmetic functions
 */
Integer Min(Integer a, Integer b);

Integer Max(Integer a, Integer b);

/* *** memory functions
 */
SignedByte HSetLockState(Handle h);

void BlockClear(void *xp, LongInt len);

Boolean EqualBlock(void * a, void *b, LongInt len);

Boolean EqualHandle(Handle a, Handle b);

/* *** string functions.
 */
void StrMove(const StringPtr src, StringPtr dest);

StringHandle StrHandle(const StringPtr s);

void AppendChar(StringPtr s, char c);

void Concat(StringPtr s, StringPtr tail);

/* *** dialog fucntions
 */
pascal void NullUpdate(DialogPtr dp, Integer i);

void SetWDIHandle(WindowPtr win, Integer item, Handle h);

void SetDIHandle(Integer item, Handle h);

Handle GetWDIHandle(WindowPtr win, Integer item);

Handle GetDIHandle(Integer item);

ControlHandle GetCIHandle(Integer item);

void GetDIRect(Integer item, Rect *rp);

Boolean IsCancel(EventRecord *theEvent);

Boolean IsReturnEnter(char c);

void MirrorIt(Integer itemHit);

/* *** window functions.
 */
void GetContentsRect(Rect *rp);

RgnHandle RestrictClipRect(const Rect *r);

void RestoreClip(RgnHandle rgn);

void InitScrollBarClass(ScrollBarClassPtr p);

Boolean TrackScroll(Point where);

Boolean TitleBarOnScreen(const Rect *rp);

/* *** File functions
 */
Boolean EqualFSSpec(FSSpecPtr a, FSSpecPtr b);

Handle GetPreferencesHandle(OSType type, Integer id);

void SavePreferencesResource(Handle h, OSType type, Integer id);

#endif /* _H_UTILS */
