/************************************************************************************
 * AboutBox.c
 *
 * from HexEdit, a simple hex editor
 * copyright 1993, Jim Bumgardner
 *
 ************************************************************************************/

#include "HexEdit.h"

pascal	Boolean MyAboutFilter ( DialogPtr, EventRecord *, short *);
long	gStartTime;

#define AboutDLOG		130
#define AutoCloseTime	5*60

HexEditAboutBox()
{
	GrafPtr			savePort;
	DialogPtr		dp;
	short			itemHit;
	VersRecHndl		vr;
	StringPtr		myVersStr;
	
	if ((vr = (VersRecHndl) GetResource('vers',1)) != NULL) {
		HLock((Handle) vr);
		myVersStr = (StringPtr) (((unsigned long) &(**vr).shortVersion[1]) + 
						  ((**vr).shortVersion[0]));
	}
	else
		myVersStr = "\p???";

	ParamText(myVersStr,"\p","\p","\p");
	
	GetPort(&savePort);
	dp = GetNewDialog(AboutDLOG, NULL, (WindowPtr) -1L);
	gStartTime = TickCount();
	ModalDialog(MyAboutFilter, &itemHit);
	DisposeDialog(dp);
	SetPort(savePort);
	if (vr) {
		HUnlock((Handle) vr);
		ReleaseResource((Handle) vr);
	}
}

pascal Boolean MyAboutFilter(DialogPtr dp, EventRecord *ep, short *itemHit)
{
	char tempChar;
	switch (ep->what) {
	  case mouseDown:		// For About Box - trap all events.
	  case keyDown:
	  case autoKey:
	  	return true;
		break;
	  case updateEvt:
	  	SetPort(dp);
	  	TextFont(1);
	  	TextSize(9);
		break;
	  case nullEvent:
	  	if (TickCount() - gStartTime > AutoCloseTime)
	  		return true;
	  default:
	  	break;

	}
	return false;
}
