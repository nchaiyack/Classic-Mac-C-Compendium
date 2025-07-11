#include	"DialogMgr.h"
#include	"ControlMgr.h"
#include	"stdio.h"

extern Boolean
	hasColorQD;

extern	WindowPtr	cWindow;
extern	WindowPtr	eWindow;

extern	PicHandle
	ePict,ebmPict;

extern	int	draw;	/* != 0 if we want to draw the line.  ==0 if move to point */
extern	int	over;	/* != 0 when point is outside visible area */

extern	CursHandle
	watchCursorHand;

extern	Handle
	iconHandle;

extern	double	xsize,ysize;
extern	double	half_xsize,half_ysize;
extern	int		ixsize,iysize;

extern	DialogPtr	optionsDialogPtr;

extern	int
	npictButton,		/* options dialog items */
	nbitmapButton,
	nintButton,
	nfpButton,
	nhiddenlinesBox,
	nlnlBox,
	nsquareBox;
extern	long
	backgroundC,
	earthbackgroundC,
	earthoutlineC,
	latC,
	longC,
	landC;

extern	Rect	latSBRect,
				lonSBRect,
				altSBRect,
				latDataRect,
				lonDataRect,
				altDataRect,
				northRect,
				southRect,
				eastRect,
				westRect,
				mileRect,
				kmRect,
				earthWindowRect,
				controlWindowRect,
				iconRect;

extern	ControlHandle
	latSB,
	lonSB,
	altSB,
	northCheck,
	southCheck,
	eastCheck,
	westCheck,
	mileCheck,
	kmCheck;
