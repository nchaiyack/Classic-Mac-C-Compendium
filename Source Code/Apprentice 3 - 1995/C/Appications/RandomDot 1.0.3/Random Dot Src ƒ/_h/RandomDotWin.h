/* RandomDotWin.h
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */

/* These bits are in the flags long, below.
 */
#define bSaved			(1L << 0)
#define bShowGray		(1L << 1)	/* mutually exclusive */
#define bShowGrayStereo	(1L << 2)	/* mutually exclusive */
#define bShowBWStereo	(1L << 3)	/* mutually exclusive */
#define bIsShimmer		(1L << 4)	/* true if we should animate the palette */
#define kShow			(bShowGray|bShowGrayStereo|bShowBWStereo|bIsShimmer)
#define bIsGray			(1L << 5)	/* true if the stereogram is gray */

typedef struct RandomDotWindowRec {
	WindowRecord	win;
	LongInt			flags;
	Rect			frame;
	CGrafPtr		grayImage;		/* actually GWorld, but don't require that include file be all ready read in */
	CGrafPtr		stereoImage;	/* actually GWorld, but don't require that include file be all ready read in */
	ControlHandle	hScroll;
	ControlHandle	vScroll;
	FSSpec			fs;
	ScriptCode		code;
}RandomDotWindowRec, *RandomDotWindowPtr;

extern CTabHandle		grayCTab;

OSErr RandomDotOpen(FSSpecPtr fs, ScriptCode code);

OSErr InitRandomDot(void);


void RandomDotDisposeWindow(WindowPtr win);
void RandomDotKey(EventRecord *e);
void RandomDotClick(EventRecord *e);
void RandomDotIdle(EventRecord *e);
void RandomDotUpdate(void);
void RandomDotActivate(void);
void RandomDotDeactivate(void);
void RandomDotGrow(void);
void RandomDotGrowBounds(Rect *rp);
void RandomDotPalette(void);
void RandomDotCopybitsWin(RandomDotWindowPtr win, Rect *framep);
void RandomDotIdealSize(WindowPtr win, Rect *rp);

/* shimmer mode support.
 */
void ShimmerDotPalette(void);
