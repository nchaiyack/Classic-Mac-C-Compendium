#include "kant parser dispatch.h"
#include "kant parser.h"
#include "window layer.h"
#include "environment.h"
#include "menus.h"
#include "resource layer.h"
#include "text layer.h"
#include "music layer.h"

ErrorTypes ParserDispatch(WindowRef theWindow)
{
	ParserError		parseError;
	TEHandle		hTE;
	short			index;
	OSErr			oe;
	short			oldRefNum, refNum;
	Boolean			alreadyOpen;
	short			oldViewBottom;
	
	if (!gUseDefault)
		if ((oe=OpenTheResFile(&gModuleFS, &oldRefNum, &refNum, &alreadyOpen, TRUE))!=noErr)
			return kModuleNotFound;
	
	gCustomCursor=FALSE;
	gInProgress=TRUE;
	AdjustMenus();
	DrawMenuBar();
	
	index=GetWindowIndex(theWindow);
	InitTheParser(theWindow, 0, 32767);
	if (gMusicStatus==kMusicGeneratingOnly)
		StartTheMusic();
	if (gFastResolve)
	{
		oldViewBottom=MyHideTE(GetWindowTE(theWindow));
		gCustomCursor=TRUE;
	}
	parseError=ParseLoop();
	
	if (gMusicStatus==kMusicGeneratingOnly)
		CloseTheMusicChannel();
	
	if (!gUseDefault)
		CloseTheResFile(oldRefNum, refNum, alreadyOpen);

	gCustomCursor=FALSE;
	if (IndWindowExistsQQ(index))
	{
		hTE=GetWindowTE(theWindow);
		if (gFastResolve)
		{
			MyShowTE(hTE, oldViewBottom);
			UpdateTheWindow(theWindow);
		}
		TESelView(hTE);
		AdjustVScrollBar(GetWindowVScrollBar(theWindow), hTE);
	}
	
	gInProgress=FALSE;
	AdjustMenus();
	DrawMenuBar();
	
	switch (parseError)
	{
		case kNoReferenceName:		return kParserNullReferenceName;
		case kBadReference:			return kParserReferenceNotFound;
		case kBadChoiceFormat:		return kParserBadChoiceFormat;
		case kBadOptionalFormat:	return kParserBadOptionalFormat;
		case kNoInstantiations:		return kParserNoInstantiations;
		case kBadBackslash:			return kParserIllegalBackslash;
		case kUserCancelled:		return userCancelErr;
	}
	
	return allsWell;
}
