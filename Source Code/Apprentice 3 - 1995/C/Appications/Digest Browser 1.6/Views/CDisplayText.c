/******************************************************************************
	CDisplayText.c
	
	Methods for a text editing pane.
		
	Copyright � 1989 Symantec Corporation. All rights reserved.
    Copyright � 1991 Manuel A. P�rez.  All rights reserved.

	2/29/92 - Set Scroll Bars back to the top when a new item is shown
 ******************************************************************************/


#include "CDisplayText.h"
#include "Browser.h"

void CDisplayText::IDisplayText(CView *anEnclosure, CBureaucrat *aSupervisor,
		short vLoc, short heigth)
{
Rect	margin;

// JRB change from CEditText::IEditText to CBrowserEditText::IBrowserEditText
	CBrowserEditText::IBrowserEditText(anEnclosure, aSupervisor,
		1, height,		 				// aWidth, aHeight
		0, vLoc,						// aHEncl, aVEncl
		sizELASTIC, sizELASTIC,			// aHSizing, aVSizing
		-1);							// lineWidth
	FitToEnclosure(TRUE, TRUE);			// fit horiz, but not vert

		/**
		 **	Give the edit pane a little margin.
		 **	Each element of the margin rectangle
		 **	specifies by how much to change that
		 **	edge. Positive values are down and to
		 **	right, negative values are up and to
		 **	the left.
		 **
		 **/

	SetRect(&margin, 2, 2, -2, -2);
	ChangeSize(&margin, FALSE);
	Specify(false, true, false);	// edit, select, style
	SetCanBeGopher(true);

	// set default font and font size for this display
	SetFontNumber(geneva);
	SetFontSize(10);
	(*macTE)->crOnly = 1;			// do wrap around


}	/* IDisplayText */

void CDisplayText::DoKeyDown(char theChar, Byte keyCode, EventRecord *macEvent)
{
}

/******************************************************************************
 ProviderChanged
 
 	Respond to the provider change.
 	
******************************************************************************/

void CDisplayText::ProviderChanged( CCollaborator *aProvider, long reason, void* info)

{
BrowserItemPtr p;
long line;
long len;
Handle text;
long offset;
LongPt aPosition;

	switch (reason) {
		case textSelectionChanged:
			p = (BrowserItemPtr) info;

			// get info from file and draw it on screen
			if (p) {

				//len = p->endAt - p->startAt;
				len = brGetEnd(p) - brGetStart(p);
				FailNIL(text = NewHandle(len));		// fail if no memory
				HLock(text);						// lock handle
				//fseek(p->fp, p->startAt, 0);		// read in text
				//fread(*text, 1, len, p->fp);
				fseek(brGetFP(p), brGetStart(p), 0);		// read in text
				fread(*text, 1, len, brGetFP(p));

				offset = 0;							// replace '\n' with '\r'
				do {
					offset = Munger(text, offset, "\n", 1, "\r", 1);
				} while (offset > 0);

				HUnlock(text);						// unlock handle
				SetTextHandle(text);				// put text on window
				DisposHandle(text);					// release handle 

				// Set the scrollers back to the top
				aPosition.h = 0;
				aPosition.v = 0;
				ScrollTo(&aPosition, true);
			}

			break;
	}
}	/* ProviderChanged */
