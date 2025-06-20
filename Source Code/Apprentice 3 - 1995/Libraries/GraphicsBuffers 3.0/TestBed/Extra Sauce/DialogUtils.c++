/*
	DialogUtils.c++
	Written by Hiep Dam; one or two routines written by others.
	
	Version history:
		6/6/95: Added GetRadioBtn
*/

#include "DialogUtils.h"

// ---------------------------------------------------------------------------

#define DIALOGUTILS_USE_COLOR			// Comment out to remove dependency on Compat.h

#if defined(DIALOGUTILS_USE_COLOR)		// Include Compatibility routines, access to gEnviron
#include "Compat.h"
#include "Palettes.h"
#endif

#if defined(DIALOGUTILS_USE_COLOR)
GDHandle GetDeviceFromLocalRect(Rect *theRect);

GDHandle GetDeviceFromLocalRect(Rect *theRect) {
	Rect temp;
	Point tpLeft, btRight;

	tpLeft.h = theRect->left;
	tpLeft.v = theRect->top;
	btRight.h = theRect->right;
	btRight.v = theRect->bottom;
	LocalToGlobal(&tpLeft);
	LocalToGlobal(&btRight);
	
	temp.left = tpLeft.h;
	temp.top = tpLeft.v;
	temp.right = btRight.h;
	temp.bottom = btRight.v;
	
	return(GetMaxDevice(&temp));
} // END GetDeviceFromLocalRect
#endif

// ---------------------------------------------------------------------------

void OutlineDefaultBorder(Rect *border, Boolean activate) {
	RGBColor oldForeColor, newForeColor;
	RGBColor backColor;
	Rect theBorder = *border;
	PenState oldPen;
	GetPenState(&oldPen);			// preserve current pen state
	PenNormal();					// reset the pen state

	PenSize(3,3);					// set thicker pen size
  	InsetRect(&theBorder, -4, -4);	// inset outside of default item

	if (!activate) {
#if defined(DIALOGUTILS_USE_COLOR)
		if (gEnviron.hasColor) {
			// OK, do it in color			
			GetBackColor(&backColor);
			GetForeColor(&oldForeColor);
			newForeColor = oldForeColor;
			if (GetGray(GetDeviceFromLocalRect(border), &backColor, &newForeColor))
				RGBForeColor(&newForeColor);
			else
				// Gray color not avail, so use gray pattern (ugh!)
				PenPat((ConstPatternParam)&qd.gray);
		}
		else
			// If no color, just use standard gray pattern
			PenPat((ConstPatternParam)&qd.gray);
#else
		PenPat((ConstPatternParam)&qd.gray);
#endif
	}

	FrameRoundRect(&theBorder, 16, 16);	// draw the border

#if defined(DIALOGUTILS_USE_COLOR)
	if (!activate && gEnviron.hasColor)
		RGBForeColor(&oldForeColor);
#endif

	SetPenState(&oldPen);				// restore the pen state
} // END OutlineDefaultBorder

// ---------------------------------------------------------------------------

pascal void DrawDefaultBorder(DialogPtr theDialog, short itemNo) {
	GrafPtr	oldPort;
	short iType;
	Handle h;
	Rect box;

	GetPort(&oldPort);							/* preserve current grafport		*/
	SetPort(theDialog);
	GetDItem(theDialog, itemNo, &iType, &h, &box);	/* get the default (#1) item's info	*/
	OutlineDefaultBorder(&box, (**(ControlHandle)h).contrlHilite != 255);
	SetPort(oldPort);							/* restore the current grafport		*/
} // END DrawDefaultBorder

// ---------------------------------------------------------------------------

/*
	Note: assumes rect derived from the user item encompasses the scroll
	bar also; i.e. it doesn't make any adjustments.
*/
void DrawDefaultListBorder(DialogPtr theDialog, short itemNo, Boolean active) {
	GrafPtr savePort;
	PenState savePen;
	Rect listRect;
	
	GetPort(&savePort);
	SetPort(theDialog);
	GetPenState(&savePen);

	GetDItemRect(theDialog, itemNo, &listRect);
	InsetRect(&listRect, -4, -4);
	if (active)
		PenPat((ConstPatternParam)&qd.black);	// Outline
	else
		PenPat((ConstPatternParam)&qd.white);	// Erase
	PenSize(2, 2);
	FrameRect(&listRect);
	
	SetPenState(&savePen);
	SetPort(savePort);
} // END DrawDefaultListBorder

// ---------------------------------------------------------------------------

// Not used or tested yet.
void DrawDimmedDialogText(DialogPtr theDialog, short item, Boolean dimIt, Boolean isEditable) {
	GrafPtr savePort;
	Str255 dimStr;
	Rect frame;
	RGBColor oldForeColor, newForeColor;
	RGBColor backColor;
	PenState savePen;

	GetPort(&savePort);
	SetPort(theDialog);
	GetPenState(&savePen);

	GetIText(GetDItemHdl(theDialog, item), dimStr);

	GetDItemRect(theDialog, item, &frame);
	MoveTo(frame.left + 1, frame.bottom - 4);

	if (dimIt) {
#if defined(DIALOGUTILS_USE_COLOR)
		if (gEnviron.hasColor) {
			// OK, do it in color			
			GetBackColor(&backColor);
			GetForeColor(&oldForeColor);
			newForeColor = oldForeColor;
			if (GetGray(GetDeviceFromLocalRect(&frame), &backColor, &newForeColor))
				RGBForeColor(&newForeColor);
			else
				// Gray color not avail, so use gray pattern (ugh!)
				PenPat((ConstPatternParam)&qd.gray);
				PenMode(patBic);
		}
		else
			// If no color, just use standard gray pattern
			PenPat((ConstPatternParam)&qd.gray);
			PenMode(patBic);
#else
		PenPat((ConstPatternParam)&qd.gray);
		PenMode(patBic);
#endif
		DrawString(dimStr);
		PaintRect(&frame);
	}
	else
		DrawString(dimStr);

	// Is it an editable text item or static text item?
	if (isEditable) {
		InsetRect(&frame, -3, -3);	// Enlarge by 3 pixels
		PenMode(patCopy);
		FrameRect(&frame);
	}

#if defined(DIALOGUTILS_USE_COLOR)
	if (dimIt && gEnviron.hasColor)
		RGBForeColor(&oldForeColor);
#endif

	SetPenState(&savePen);
	SetPort(savePort);
} // END DrawDimmedDialogText

// ---------------------------------------------------------------------------

void GetDItemRect(DialogPtr theDialog, short theItem, Rect *theRect) {
	Handle itemH; short itemT;
	GetDItem(theDialog, theItem, &itemT, &itemH, theRect);
} // END GetDItemRect

// ---------------------------------------------------------------------------

void SetUserProc(DialogPtr theDialog, short theItem, ProcPtr theProc) {
	Handle itemH; short itemT; Rect itemR;
	GetDItem(theDialog, theItem, &itemT, &itemH, &itemR);
	SetDItem(theDialog, theItem, itemT, (Handle)theProc, &itemR);
} // END SetUserProc;

// ---------------------------------------------------------------------------

Handle GetDItemHdl(DialogPtr theDialog, short theItem) {
	Handle itemH = nil;
	short itemT; Rect itemR;
	GetDItem(theDialog, theItem, &itemT, &itemH, &itemR);
	return(itemH);
} // END GetDItemHdl

// ---------------------------------------------------------------------------

ControlHandle GetCtlHdl(DialogPtr theDialog, short theItem) {
	return((ControlHandle)GetDItemHdl(theDialog, theItem));
} // END GetCtlHdl

// ---------------------------------------------------------------------------

void FlipCtlValue(DialogPtr theDialog, short theItem) {
	short maxVal = GetCtlMax((ControlHandle)GetDItemHdl(theDialog, theItem));
	short minVal = GetCtlMin((ControlHandle)GetDItemHdl(theDialog, theItem));
	short curVal = GetCtlValue((ControlHandle)GetDItemHdl(theDialog, theItem));
	if (curVal == minVal)
		SetCtlValue((ControlHandle)GetDItemHdl(theDialog, theItem), maxVal);
	else
		SetCtlValue((ControlHandle)GetDItemHdl(theDialog, theItem), minVal);
} // END FlipCtlValue

// ---------------------------------------------------------------------------

void SetDlogCtlValue(DialogPtr theDialog, short theItem, short theValue) {
	SetCtlValue(GetCtlHdl(theDialog, theItem), theValue);
} // END SetDlogCtlValue

short GetDlogCtlValue(DialogPtr theDialog, short theItem) {
	return GetCtlValue(GetCtlHdl(theDialog, theItem));
} // END GetDlogCtlValue

// ---------------------------------------------------------------------------

void EnableDlogCtl(DialogPtr theDialog, short theItem) {
	HiliteControl(GetCtlHdl(theDialog, theItem), 0);
} // END EnableDlogCtl

void DisableDlogCtl(DialogPtr theDialog, short theItem) {
	HiliteControl(GetCtlHdl(theDialog, theItem), 255);
} // END DisableDlogCtl

// ---------------------------------------------------------------------------

void SelectRadioBtn(DialogPtr theDialog, short selectedBtn, short beginRange, short endRange) {
	for (short i = beginRange; i <= endRange; i++) {
		SetCtlValue((ControlHandle)GetDItemHdl(theDialog, i), 0);
	}
	SetCtlValue((ControlHandle)GetDItemHdl(theDialog, selectedBtn), 1);
} // END SelectRadioBtn

// ---------------------------------------------------------------------------

short GetRadioBtn(DialogPtr theDialog, short beginRange, short endRange)
/*
	Given a range of radio buttons, this routines returns the first
	radio button that's selected (i.e. has a value of 1).

	Note: Apple User Interface Guidelines state that in a given
	range of radio buttons, one and only one radio button should
	be selected. If you have no radio button selected (although you
	should have one preselected) this routine will return 0. If you
	have more than 1 radio button selected, this routine will return
	the first radio button in the group.
*/
{
	short itemSelected = 0;
	
	for (short i = beginRange; i <= endRange; i++) {
		if (GetCtlValue(GetCtlHdl(theDialog, i))) {
			itemSelected = i;
			break;
		}
	}
	
	return(itemSelected);
} // END GetRadioBtn

// ---------------------------------------------------------------------------

void PushButton(DialogPtr theDialog, short theItem) {
	PushButtonControl(GetCtlHdl(theDialog, theItem));
} // END PushButton

// ---------------------------------------------------------------------------

void PushButtonControl(ControlHandle theControl) {
	long dummy;

	HiliteControl(theControl, inButton);
	Delay(7, &dummy);
	HiliteControl(theControl, 0);
} // END PushButtonControl

// ---------------------------------------------------------------------------

void SetDItemText(DialogPtr theDialog, short theItem, Str255 iText) {
	Handle itemH; short itemT; Rect itemR;
	GetDItem(theDialog, theItem, &itemT, &itemH, &itemR);
	SetIText(itemH, iText);
} // END SetDItemText

// ---------------------------------------------------------------------------

// ChangeDITL.
// Adds a new dialog item list to the dialog.
// origCount is the ditl count before you added any extra ditl items to it.
// It uses this as a reference on how many items to remove (shorten).
// newCount returns the new total number of ditl items.

void ChangeDITL(DialogPtr theDialog, short DITLid, short origCount, short *newCount, short eraseItem) {
	Rect eraseRect;
	short shortenCount = CountDITL(theDialog) - origCount;
	Handle curDITL = GetResource('DITL', DITLid);

	HLock(curDITL);

	if (shortenCount > 0)
		ShortenDITL(theDialog, shortenCount);

	if (eraseItem != 0) {
		ShowDItem(theDialog, eraseItem);
		GetDItemRect(theDialog, eraseItem, &eraseRect);
		HideDItem(theDialog, eraseItem);
		SetPort(theDialog);
		EraseRect(&eraseRect);
	}

	AppendDITL(theDialog, curDITL, overlayDITL);
	HUnlock(curDITL);
	ReleaseResource(curDITL);
	*newCount = CountDITL(theDialog);
} // END ChangeDITL

// ---------------------------------------------------------------------------

pascal void FrameBorderBlack(DialogPtr theDialog, short itemNo) {
	GrafPtr savePort;
	Rect frameR;
	PenState savePen;
	
	GetPort(&savePort);
	SetPort(theDialog);
	
	GetPenState(&savePen);
	GetDItemRect(theDialog, itemNo, &frameR);
	PenNormal();
	FrameRect(&frameR);
	
	SetPenState(&savePen);
	SetPort(savePort);
} // END FrameBorderBlack

// ---------------------------------------------------------------------------

pascal void FrameBorderDotted(DialogPtr theDialog, short itemNo) {
	GrafPtr savePort;
	Rect frameR;
	PenState savePen;
	
	GetPort(&savePort);
	SetPort(theDialog);
	
	GetPenState(&savePen);
	GetDItemRect(theDialog, itemNo, &frameR);
	PenNormal();
	PenPat((ConstPatternParam)&qd.gray);
	FrameRect(&frameR);
	
	SetPenState(&savePen);
	SetPort(savePort);
} // END FrameBorderDotted

// -----------------------------------------------------------------------------------

// Unlike SetDItemText, which works on text 255 chars or less, this routine
// will set an edit text item to any length of text, if under 32K...

void SetDItemBigText(DialogPtr theDialog, short theItem, Ptr theText, short textLen) {
	GrafPtr savePort;
	short curSelection = ((DialogPeek)theDialog)->editField + 1;
	Rect tRect;
	
	GetPort(&savePort);
	SetPort(theDialog);

	SelIText(theDialog, theItem, 0, 32767);
	TESetText(theText, textLen, ((DialogPeek)theDialog)->textH);
	if (curSelection > 0)
		SelIText(theDialog, curSelection, 0, 32767);
	else
		SelIText(theDialog, theItem, 0, 32767);
	//DrawDialog(theDialog);
	GetDItemRect(theDialog, theItem, &tRect);
	TEUpdate(&tRect, ((DialogPeek)theDialog)->textH);

	SetPort(savePort);
} // END SetDItemBigText