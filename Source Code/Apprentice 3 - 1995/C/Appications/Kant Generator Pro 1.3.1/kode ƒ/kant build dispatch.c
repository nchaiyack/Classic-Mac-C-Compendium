#include "kant build dispatch.h"
#include "kant build dialogs.h"
#include "kant build files.h"
#include "kant build lists.h"
#include "kant build window.h"
#include "text layer.h"
#include "menus.h"
#include "window layer.h"
#include "generic window handlers.h"
#include "program globals.h"

void DoNewDispatch(WindowRef theWindow, Boolean isRef)
{
	if (isRef)
		DoNewRef(theWindow);
	else
		DoNewInstantiation(theWindow);
}

void DoEditDispatch(WindowRef theWindow)
{
	if (ReferenceListActiveQQ())
		DoEditRef(theWindow);
	else
		DoEditInstantiation(theWindow);
}

void DoDeleteDispatch(WindowRef theWindow)
{
	if (ReferenceListActiveQQ())
		DoDeleteRef(theWindow);
	else
		DoDeleteInstantiation(theWindow);
}

void DoMarkInterestingDispatch(WindowRef theWindow)
{
	Boolean			oldState;
	Str255			referenceName;
	
	GetHighlightedReferenceName(referenceName);
	oldState=HighlightedReferenceInterestingQQ();
	SetInterestingBit(GetWindowFS(theWindow), referenceName, !oldState);
	RebuildInsertMenu();
	SetHighlightedReferenceInteresting(!oldState);
}

void DoShowMessageDispatch(WindowRef theWindow)
{
	gShowMessageBox=!gShowMessageBox;
	EraseRect(&(theWindow->portRect));
	ResizeTheBuildWindow(theWindow);
	InvalRect(&(theWindow->portRect));
}

void DoShowToolbarDispatch(WindowRef theWindow)
{
	gShowToolbar=!gShowToolbar;
	EraseRect(&(theWindow->portRect));
	ResizeTheBuildWindow(theWindow);
	InvalRect(&(theWindow->portRect));
}
