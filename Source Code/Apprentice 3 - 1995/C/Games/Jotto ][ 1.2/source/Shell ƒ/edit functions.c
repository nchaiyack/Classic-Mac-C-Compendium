#include "edit functions.h"
#include "text twiddling.h"
#include "window layer.h"

void GenericCut(WindowPtr theWindow)
{
	TEHandle		hTE;
	ControlHandle	vScrollBar;
	
	hTE=GetWindowTE(theWindow);
	vScrollBar=GetWindowVScrollBar(theWindow);
	if (AnyHighlightedQQ(theWindow))
		SetWindowIsModified(theWindow, TRUE);
	TECut(hTE);
	ZeroScrap();
	TEToScrap();
	if (vScrollBar!=0L)
		AdjustForEndScroll(vScrollBar, hTE);
	TESelView(hTE);
	if (vScrollBar!=0L)
		AdjustVScrollBar(vScrollBar, hTE);
}

void GenericCopy(WindowPtr theWindow)
{
	TEHandle		hTE;
	ControlHandle	vScrollBar;
	
	hTE=GetWindowTE(theWindow);
	vScrollBar=GetWindowVScrollBar(theWindow);
	TECopy(hTE);
	ZeroScrap();
	TEToScrap();
	TESelView(hTE);
	if (vScrollBar!=0L)
		AdjustVScrollBar(vScrollBar, hTE);
}

void GenericPaste(WindowPtr theWindow)
{
	TEHandle		hTE;
	ControlHandle	vScrollBar;
	Handle			scrapHandle;
	long			dummy;
	unsigned long	scrapLength;
	
	hTE=GetWindowTE(theWindow);
	vScrollBar=GetWindowVScrollBar(theWindow);
	scrapHandle=NewHandle(0L);
	if (GetScrap(scrapHandle, 'TEXT', &dummy)!=noTypeErr)
	{
		scrapLength=GetHandleSize(scrapHandle);
		if (scrapLength+(**hTE).teLength>32767)
		{
			SysBeep(7);
			return;
		}
		
		if (scrapLength!=0L)
			SetWindowIsModified(theWindow, TRUE);
		TEFromScrap();
		TEPaste(hTE);
		TESelView(hTE);
		if (vScrollBar!=0L)
			AdjustVScrollBar(vScrollBar, hTE);
	}
	
	DisposeHandle(scrapHandle);
}

void GenericClear(WindowPtr theWindow)
{
	TEHandle		hTE;
	ControlHandle	vScrollBar;
	
	hTE=GetWindowTE(theWindow);
	vScrollBar=GetWindowVScrollBar(theWindow);
	if (AnyHighlightedQQ(theWindow))
		SetWindowIsModified(theWindow, TRUE);
	TEDelete(hTE);
	if (vScrollBar!=0L)
		AdjustForEndScroll(vScrollBar, hTE);
	TESelView(hTE);
	if (vScrollBar!=0L)
	AdjustVScrollBar(vScrollBar, hTE);
}

void GenericSelectAll(WindowPtr theWindow)
{
	TEHandle		hTE;
	ControlHandle	vScrollBar;
	
	hTE=GetWindowTE(theWindow);
	vScrollBar=GetWindowVScrollBar(theWindow);
	TESetSelect(0, 32767, hTE);
	TESelView(hTE);
	if (vScrollBar!=0L)
		AdjustVScrollBar(vScrollBar, hTE);
}
