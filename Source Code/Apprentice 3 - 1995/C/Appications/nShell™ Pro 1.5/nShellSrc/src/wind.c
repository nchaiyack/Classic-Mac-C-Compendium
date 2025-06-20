/* ==========================================

	wind.c
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

#include "multi.h"
#include "nsh.h"
#include "resource.h"
#include "windows.h"

#include "multi.proto.h"
#include "utility.proto.h"
#include "wind.proto.h"

#define SBarWidth		15		// pixels to leave for scroll bar

ShellH gScrollShell;

int wind_new( ShellH shell, int w_offset )
{
	Rect			destRect;
	Rect			viewRect;
	Rect			vScrollRect;
	FontInfo		myInfo;
	long			temp;
	int				hpos;
	int				vpos;
	ControlHandle	kontrol;
	WindowPtr		wynd;
	TEHandle		tex;
	
#if FULL_APP
	hpos = 5;
	vpos = 35;
#else
	temp = 2 * (TickCount() & 0x0F);
	hpos = temp + 10;
	vpos = temp + 40;
#endif
	
	
	while (w_offset > 10) {
		vpos += 20;
		w_offset -= 10;
		}
	
	w_offset *= 10;
	hpos += w_offset;
	vpos += w_offset;
	
	wynd = GetNewWindow(windowID, NULL, (WindowPtr) -1L);

	if (!wynd) {
		error_note("\pGetNewWindow for shell failed");
		return(1);
		}
		
	SetPort(wynd);
	SetWTitle( wynd, (**shell).shell_fss.name );
	MoveWindow( wynd, hpos, vpos, 0 );
	
	(**shell).WindPtr = wynd;
		
	TextFont(monaco);
	TextSize(9);
	vScrollRect = (*wynd).portRect;
	vScrollRect.left = vScrollRect.right-15;
	vScrollRect.right += 1;
	vScrollRect.bottom -= 14;
	vScrollRect.top -= 1;
	
	kontrol = NewControl( wynd, &vScrollRect, "\p", 1, 0, 0, 0, scrollBarProc, 0L);
	
	if (!kontrol) {
		error_note("\pNewControl for shell failed");
		DisposeWindow( wynd );
		return(1);
		}
		
	(**shell).Scroll = kontrol;
		
	viewRect = thePort->portRect;
	viewRect.right -= SBarWidth;
	viewRect.bottom -= SBarWidth;
	InsetRect(&viewRect, 4, 4);

	tex = TENew(&viewRect, &viewRect);
	
	if (!tex) {
		error_note("\pTENew for shell failed");
		DisposeControl( (**shell).Scroll );
		DisposeWindow( wynd );
		return(1);
		}
		
	(**tex).clikLoop = (ProcPtr)wind_track_click;
	
	(**shell).Text = tex;
		
	wind_set_view(shell);
	return(noErr);
}

void wind_text_adjust (ShellH shell)
{
	int oldScroll, newScroll, delta;

	oldScroll = (**(**shell).Text).viewRect.top - (**(**shell).Text).destRect.top;
	newScroll = GetCtlValue((**shell).Scroll) * (**(**shell).Text).lineHeight;
	delta = oldScroll - newScroll;
	if (delta != 0)
		TEScroll(0, delta, (**shell).Text);
}

void wind_set_scroll(ShellH shell)
{
	int	n;
	
	n = (**(**shell).Text).nLines-(**shell).Lines;
	if ((**(**shell).Text).teLength > 0 && (*((**(**shell).Text).hText))[(**(**shell).Text).teLength-1]==RETURN_KEY)
		n++;
	SetCtlMax((**shell).Scroll, n > 0 ? n : 0);
}

void wind_show_sel(ShellH shell)
{
	int topLine, bottomLine, theLine;
	
	wind_set_scroll(shell);
	wind_text_adjust(shell);
	
	topLine = GetCtlValue((**shell).Scroll);
	bottomLine = topLine + (**shell).Lines;
	
	if ((**(**shell).Text).selStart < (**(**shell).Text).lineStarts[topLine] ||
			(**(**shell).Text).selStart >= (**(**shell).Text).lineStarts[bottomLine]) {
		for (theLine = 0; (**(**shell).Text).selStart >= (**(**shell).Text).lineStarts[theLine]; theLine++)
			;
		SetCtlValue((**shell).Scroll, theLine - (**shell).Lines / 2);
		wind_text_adjust(shell);
		}
}

void wind_set_view (ShellH shell)
{
	Rect	myRect;
	int		myLine;
	int		lineHeight;
	
	myRect = (**shell).WindPtr->portRect;
	myRect.right -= SBarWidth;
	myRect.bottom -= SBarWidth;
	InsetRect(&myRect, 4, 4);

	lineHeight = (**(**shell).Text).lineHeight;
	myLine = ( myRect.bottom - myRect.top ) / lineHeight;

	myRect.bottom = myRect.top + lineHeight * myLine;

	(**shell).Lines = myLine;
	(**(**shell).Text).viewRect = myRect;
	(**(**shell).Text).destRect.right = myRect.right;
	TECalText((**shell).Text);
}

void wind_update(ShellH shell)

{
	GrafPtr		savePort;
			
	GetPort(&savePort);
	SetPort((**shell).WindPtr);
	BeginUpdate((**shell).WindPtr);
	EraseRect(&(**shell).WindPtr->portRect);
	DrawControls((**shell).WindPtr);
	DrawGrowIcon((**shell).WindPtr);
	TEUpdate(&(**shell).WindPtr->portRect, (**shell).Text);
	EndUpdate((**shell).WindPtr);
	SetPort(savePort);
}

pascal void wind_scroll_proc (ControlHandle theControl, int theCode)
{
	ShellH	shell;
	TEHandle shellTE;
	int	pageSize;
	int	scrollAmt;
	int oldCtl;
	
	if (theCode == 0)
		return ;

	shell = multi_front();

	if (!shell) error_note("\pwind_scroll_proc has no shell");
	
	shellTE = (**shell).Text;
	
	if (!shellTE) error_note("\pwind_scroll_proc has no text");
	
	pageSize = ((**shellTE).viewRect.bottom-(**shellTE).viewRect.top) / (**shellTE).lineHeight - 1;
			
	switch (theCode) {
		case inUpButton: 
			scrollAmt = -1;
			break;
		case inDownButton: 
			scrollAmt = 1;
			break;
		case inPageUp: 
			scrollAmt = -pageSize;
			break;
		case inPageDown: 
			scrollAmt = pageSize;
			break;
	}

	oldCtl = GetCtlValue(theControl);
	SetCtlValue(theControl, oldCtl+scrollAmt);

	wind_text_adjust(shell);
}

void wind_action(ShellH shell, EventRecord *theEvent)
{
	int				cntlCode;
	ControlHandle 	theControl;
	int				pageSize;
	GrafPtr			savePort;
	TEHandle 		shellTE;
	
	shellTE = (**shell).Text;
	
	GetPort(&savePort);
	SetPort((**shell).WindPtr);

	GlobalToLocal(&theEvent->where);
	cntlCode = FindControl(theEvent->where, (**shell).WindPtr, &theControl);
	if (cntlCode == 0) {
		if (PtInRect(theEvent->where, &(**shellTE).viewRect)) {
			gScrollShell = shell; // data for clikloop callback
			TEClick(theEvent->where, (theEvent->modifiers & shiftKey)!=0, shellTE);
			}
	} else if (cntlCode == inThumb) {
		TrackControl(theControl, theEvent->where, 0L);
		wind_text_adjust(shell);
	} else
		TrackControl(theControl, theEvent->where, &wind_scroll_proc);

	SetPort(savePort);
}

void wind_grow(ShellH shell, Point p)

{
	GrafPtr	savePort;
	long	theResult;
	Rect	oldHorizBar;
	Rect 	r;
	TEHandle shellTE;

	shellTE = (**shell).Text;
		
	GetPort(&savePort);
	SetPort((**shell).WindPtr);
	
	oldHorizBar = (**shell).WindPtr->portRect;
	oldHorizBar.top = oldHorizBar.bottom - (SBarWidth+1);

	SetRect(&r, 80, 80, screenBits.bounds.right, screenBits.bounds.bottom);
	theResult = GrowWindow((**shell).WindPtr, p, &r);
	if (theResult == 0)
	  return;
	SizeWindow( (**shell).WindPtr, LoWord(theResult), HiWord(theResult), false);

	InvalRect(&(**shell).WindPtr->portRect);
	
	wind_set_view(shell);

	EraseRect(&oldHorizBar);
	
	MoveControl((**shell).Scroll, (**shell).WindPtr->portRect.right - SBarWidth, (**shell).WindPtr->portRect.top-1);
	SizeControl((**shell).Scroll, SBarWidth+1, (**shell).WindPtr->portRect.bottom - (**shell).WindPtr->portRect.top-(SBarWidth-2));
	r = (**(**shell).Scroll).contrlRect;
	ValidRect(&r);

	wind_set_scroll(shell);
	wind_text_adjust(shell);
	
	SetPort(savePort);
}

void wind_zoom(ShellH shell, short part)
{
	GrafPtr	savePort;
	Rect	oldHorizBar;
	Rect	newHorizBar;

	GetPort(&savePort);
	SetPort((**shell).WindPtr);
	
	oldHorizBar = (**shell).WindPtr->portRect;
	oldHorizBar.top = oldHorizBar.bottom - (SBarWidth+1);

	EraseRect(&((**shell).WindPtr->portRect));
	ZoomWindow((**shell).WindPtr, part, (**shell).WindPtr == FrontWindow());

	InvalRect(&(**shell).WindPtr->portRect);
	
	wind_set_view(shell);

	EraseRect(&oldHorizBar);
	
	MoveControl((**shell).Scroll, (**shell).WindPtr->portRect.right - SBarWidth, (**shell).WindPtr->portRect.top-1);
	SizeControl((**shell).Scroll, SBarWidth+1, (**shell).WindPtr->portRect.bottom - (**shell).WindPtr->portRect.top-(SBarWidth-2));
	newHorizBar = (**(**shell).Scroll).contrlRect;
	ValidRect(&newHorizBar);

	wind_set_scroll(shell);
	wind_text_adjust(shell);
	
	SetPort(savePort);
}

pascal Boolean wind_track_click( void )
{
	Point			where;
	Rect			viewRect;
	TEHandle		textH;
	ControlHandle	ctlHdl;
	GrafPtr			curPort;
	RgnHandle		oldClip;
	Rect			scrollRect;
	Boolean			needRedraw;
	
	/* Track the mouse while it is clicked in the window, and scroll the textEdit
	 * field if necessary.  We must always return true.  We have to set the port
	 * so that GetMouse will have the right window's local coordinates.
	 */

	GetPort( &curPort );
	SetPort( (**gScrollShell).WindPtr );

	GetMouse( &where );

	textH = (**gScrollShell).Text;
	
	viewRect = (*textH)->viewRect;
	
	ctlHdl = (**gScrollShell).Scroll;
	
	needRedraw = false;

	/* If the mouse is above or below the window, we fake a mouse click on the scroll bar */
	if ( where.v < viewRect.top ){
		wind_scroll_proc( ctlHdl, inUpButton );
		needRedraw = true;
	} else if ( where.v > viewRect.bottom ){
		wind_scroll_proc( ctlHdl, inDownButton );
		needRedraw = true;
	}
	
	/* We need to redraw the control so that the user can see the thumb move.
	 * This involved setting the clip region to include scroll bar.
	 */

	if ( needRedraw ){
		oldClip = NewRgn();
		if ( oldClip ){
			GetClip( oldClip );
			scrollRect = (*ctlHdl)->contrlRect;
			ClipRect( &scrollRect );
			Draw1Control( ctlHdl );
			SetClip( oldClip );
			DisposeRgn( oldClip );
		}
	}

	SetPort( curPort );

	return true;
}
