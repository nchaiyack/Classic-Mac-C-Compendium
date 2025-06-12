#include "ep.const.h"
#include "ep.extern.h"
#include "ToolBoxUtil.h"

extern	cMouse();
extern	cKey();
extern	cUpdate();
extern	cActivate();
extern	cClose();
extern	cClobber();

extern	eMouse();
extern	eKey();
extern	eUpdate();
extern	eActivate();
extern	eClose();
extern	eClobber();

epInit()
{
	watchCursorHand = GetCursor(watchCursor);
	HLock(watchCursorHand);

	iconHandle = GetIcon(iconID);

	initGlobalRects();

	ebmPict = GetPicture(110);
	ePict   = ebmPict;

	eWindow = NewWindow (nil, &earthWindowRect, "\pEarthView", 
						 true, 8, -1L, true, 0L);

	SkelWindow (eWindow, eMouse, eKey, eUpdate, eActivate, eClose,
                		 eClobber, nil, false);

	cWindow = NewWindow (nil, &controlWindowRect, "\pSettings",
						 true, documentProc, -1L, true, 0L);
	SkelWindow (cWindow, cMouse, cKey, cUpdate, cActivate, cClose,
                		 cClobber, nil, false);

	latSB		= NewControl(cWindow,&latSBRect,"",TRUE,  0,0, 90,16,0L);	
	lonSB		= NewControl(cWindow,&lonSBRect,"",TRUE,  0,0,180,16,0L);	
	altSB		= NewControl(cWindow,&altSBRect,"",TRUE,160,1,160,16,0L);	

	northCheck	= NewControl(cWindow,&northRect,"\pNorth",	TRUE,1,0,1,2,0L);
	southCheck	= NewControl(cWindow,&southRect,"\pSouth",	TRUE,0,0,1,2,0L);
	eastCheck	= NewControl(cWindow,&eastRect,	"\pEast",	TRUE,1,0,1,2,0L);
	westCheck	= NewControl(cWindow,&westRect,	"\pWest",	TRUE,0,0,1,2,0L);
	mileCheck	= NewControl(cWindow,&mileRect,	"\pMiles",	TRUE,1,0,1,2,0L);
	kmCheck		= NewControl(cWindow,&kmRect,"\pKilometers",TRUE,0,0,1,2,0L);

	npictButton		= 0;
	nbitmapButton	= 1;
	nintButton		= 1;
	nfpButton		= 0;
	nhiddenlinesBox	= 1;
	nlnlBox			= 0;
	nsquareBox		= 1;

	backgroundC			= whiteColor;
	earthbackgroundC	= whiteColor;
	earthoutlineC		= blackColor;
	latC				= redColor;
	longC				= redColor;
	landC				= blueColor;
}

