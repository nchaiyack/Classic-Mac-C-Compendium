/* #include file for the C interface to SAT */

/* Comment out under TC5: */
/*#include <Color.h>*/
/*#include <WindowMgr.h>*/
/*#include <DialogMgr.h>*/



#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef nil
#define nil 0L
#endif

	typedef struct Face {
		char			*colorData;
		int			resNum;
		BitMap		iconMask;
		int			rowBytes; /* Note! No flags in the highest bits!*/
		struct face	*next;
		RgnHandle		maskRgn;
		} Face, *FacePtr;

	typedef struct Sprite {
/* Variables that you should change as appropriate */
		int			kind;
		Point			position;	
		Rect			hotRect, hotRect2;
		FacePtr		face;
		ProcPtr		task;
		ProcPtr		hitTask;
/* SAT variables that you shouldn't change: */
		Point			oldpos;
		struct Sprite	*next, *prev;
		Rect			r, oldr;
/* Variables for free use by the application: */
		int			layer;
		Point			speed;
		int			mode;
		Ptr			appPtr;
		long			appLong;
		} Sprite, *SpritePtr;

typedef struct{
		int	patternType; 	/* 1 = Pattern, PatHandle, 2 = PixPat, PixPatHandle */
		PixPatHandle	thePat; 		/* or PatHandle */
		}	SATPattern, *SATPatPtr, **SATPatHandle;

#define	UpdatePtr	Ptr

typedef struct {
		WindowPtr	wind;
		int			offSizeH, offSizeV;
		GrafPtr		offScreen, backScreen;
		GDHandle		offScreenGD, BackScreenGD;
		long			ox, oy;
		int			pict, bwpict;
		Boolean		fitThePICTs;
		int			sorting, collision, searchWidth;
		GDHandle		device;
		PixMapHandle	screen;
		Rect			bounds;
		int			initDepth;
		ProcPtr		synchHook;
		SpritePtr		sRoot;
		UpdatePtr		updateRoot;
		Boolean		anyMonsters;

		CGrafPtr		ditherOff;
		GDHandle		ditherOffGD;
		CGrafPtr		iconPort;
		GDHandle		iconPortGD;
		CGrafPtr		iconPort2;
		GDHandle		iconPort2GD;
		GrafPtr		bwIconPort;
		} SATglobalsRec;

/* Constants for ConfigureSAT */
	enum{kVPositionSort=0, kLayerSort, kNoSort};
	enum{kKindCollision=0, kForwardCollision, kBackwardCollision, kNoCollision};

/* Global variables */
	extern SATglobalsRec	gSAT;	/* Most globals */
	extern FacePtr			faceRoot;	/* Face list */
	extern Boolean			colorFlag; /* Is this Mac color capable? */
	extern ProcPtr			gSoundErrorProc; /* Sound errors reported here */

/* SAT routines */

/* Initialization */
	pascal void ConfigureSAT(Boolean, int, int, int);
	pascal void InitSAT (int, int, int, int);
	pascal void CustomInitSAT (int, int, Rect *, WindowPtr, GDHandle, Boolean, Boolean, Boolean, Boolean, Boolean);
/* Maintainance, background manipulation etc. */
	pascal Boolean SATDepthChangeTest();
	pascal void SATDrawPICTs (int, int);
	pascal void PeekOffscreen();
/* Drawing */
	pascal void SATPlotFace(FacePtr, GrafPtr, GDHandle, Point, Boolean); /*Draw a Face (icon).*/
	pascal void SATPlotFaceToScreen(FacePtr, Point, Boolean); /*NEW*/
	pascal void SATCopyBits(GrafPtr, GrafPtr, GDHandle, Point, Point, int,int,Boolean); /*NEW*/
	pascal void SATCopyBitsToScreen(GrafPtr,Point,Point,int,int,Boolean); /*NEW*/
	pascal void SATBackChanged (Rect *); /*Tell SAT about changes in BackScreen*/
	pascal void SATSetPortOffScreen(); /*Use before Quick-Drawing on offScreen*/
	pascal void SATSetPortBackScreen(); /*Use before Quick-Drawing on BackScreen*/
	pascal void SATSetPortScreen(); /*Use to restore after drawing off/backscreen*/
/* Sprite handling */
	pascal FacePtr GetFace (int);
	pascal void DisposeFace (FacePtr);
	pascal SpritePtr NewSprite (int,int,int, ProcPtr, ProcPtr, ProcPtr);
	pascal SpritePtr NewSpriteAfter (SpritePtr,int,int,int, ProcPtr, ProcPtr, ProcPtr);
	pascal void KillSprite (SpritePtr);
/* Animating */
	pascal void RunSAT (Boolean);
/* Advanced special-purpose calls */
	pascal void SATInstallSynch (ProcPtr);
	pascal void SATSetSpriteRecSize (long);
	pascal void SkipSAT();
/* Offscreen - use only if you need an *extra* offscreen buffer. These calls are likely to change in the future!*/
	pascal void SATMakeOffscreen (GrafPtr*, Rect*, GDHandle*); /*Make offscreen buffer in current screen depth and CLUT.*/
	pascal void SATDisposeOffScreen (GrafPtr, GDHandle); /*Get rid of offscreen*/
	pascal OSErr CreateOffScreen (Rect*, int, CTabHandle, CGrafPtr*, GDHandle*); /*From Principia Offscreen*/
	pascal void DisposeOffScreen (CGrafPtr, GDHandle);/*From Principia Offscreen*/
/* New procedures */
	pascal void SetPortMask (FacePtr);
	pascal void SetPortFace (FacePtr);
	pascal FacePtr NewFace (Rect*);
	pascal void ChangedFace (FacePtr);
	pascal void SATSetStrings (Str255, Str255, Str255, Str255, Str255, Str255, Str255, Str255);
	pascal Boolean TrapAvailable (int);
	pascal CIconHandle SATGetCicn (int);
	pascal void SATPlotCicn (CIconHandle, GrafPtr, GDHandle, Rect*);

/*Utilities*/
	pascal void DrawInt (int);
	pascal void DrawLong (long);
	pascal int Rand (int);
	pascal int Rand10();
	pascal int Rand100();
	pascal void ReportStr (Str255);
	pascal Boolean QuestionStr(Str255);
	pascal void CheckNoMem (Ptr); /*If the Ptr is nil, out of memory emergency exit*/
	pascal int SATFakeAlert (Str255, Str255, Str255, Str255, int, int, int, Str255, Str255, Str255);
	pascal void SetMouse (Point);
/* Pattern utilities */
	pascal void SATPenPat (SATPatHandle);
	pascal void SATBackPat (SATPatHandle);
	pascal SATPatHandle SATGetPat (int);
	pascal void SATDisposePat (SATPatHandle);
/* Menu bar utilities */
	pascal void ShowMBar();
	pascal void HideMBar(WindowPtr);
/*Sound*/
	pascal void InitSATSound();			/* No longer necessary */
	pascal void SATSoundPlay (Handle, int, Boolean);	/* Play sounds with priority handling */
	pascal void SATSoundShutup();		/*Silence, dispose of sound channel*/
	pascal void SATSoundEvents();		/* Call this once in a while when not calling RunSAT often */
	pascal Boolean SATSoundDone();	/* Any sound going on?*/
	pascal Handle SATGetSound (int);
	pascal Handle SATGetNamedSound (Str255);
	pascal void SATDisposeSound (Handle);
	pascal void SATSoundOn();
	pascal void SATSoundOff();


