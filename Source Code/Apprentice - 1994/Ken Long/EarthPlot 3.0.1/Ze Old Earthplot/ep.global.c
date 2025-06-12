/* global data declarations */

#include "Quickdraw.h"
#include "ControlMgr.h"
#include "DialogMgr.h"
#include "MemoryMgr.h"

Boolean
	hasColorQD;

CursHandle
	watchCursorHand;

PicHandle
	ePict,ebmPict;

Handle
	iconHandle;


int	draw;	/* != 0 if we want to draw the line.  ==0 if move to point */
int	over;	/* != 0 when point is outside visible area */

double	xsize,ysize;
double	half_xsize,half_ysize;
int		ixsize,iysize;

DialogPtr	optionsDialogPtr;

int
	npictButton,		/* options dialog items */
	nbitmapButton,
	nintButton,
	nfpButton,
	nhiddenlinesBox,
	nlnlBox,
	nsquareBox;
long
	backgroundC,
	earthbackgroundC,
	earthoutlineC,
	latC,
	longC,
	landC;

Rect
	latSBRect,
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

ControlHandle
	latSB,			/* control window items */
	lonSB,
	altSB,
	northCheck,
	southCheck,
	eastCheck,
	westCheck,
	mileCheck,
	kmCheck;
