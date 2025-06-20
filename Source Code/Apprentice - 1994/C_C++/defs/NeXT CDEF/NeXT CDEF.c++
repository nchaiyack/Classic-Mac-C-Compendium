/*

		NeXT CDEF.c++
		
		Another CDEF implementing NeXT-like features...
		
		by Hiep Dam, 3G Software
		Copyright (C) 1994 by Hiep Dam
		Feel free to use the CDEF in your own projects or to use the source
		code as a starting point for your own cdefs. I only ask you give me
		tiny credits somewhere in your application and/or send me a copy...
		
		Contacting the author:
		America Online: StarLabs
		Delphi:         StarLabs
		Internet:       starlabs@aol.com, starlabs@delphi.com
		
		Last update: May 4, 1994
		
		••• IMPORTANT: Requires COLOR QUICKDRAW and SYSTEM 7!!! •••
		               Does NO error-checking for the above whatsoever....

*/

// ----------------------------------------------------------------------

// Note:
//		1) Whenever you see ">> 1" in the code, that means "/ 2". It's a quick
//		   shortcut to divide by 2. I use it so I don't have to include the
//		   math library.

// NeXT CDEF variation codes:
// • stdBtnProc, sysFontBtnProc, geneva9BtnProc:
//		All text-only btn procs. No special handling required.
//		Control value, max, & min ignored.
// • cicnBtnProc:
//		draws simple color icon. No text drawn (it's ignored).
//		Control value holds the rsrc id of the color icon to draw.
//		Control max & min ignored.
// • inRectProc, outRectProc:
//		Just frames, similar to the button frames. Doesn't respond to clicks.
//		Use mainly as dressing...

// Some important things I learned (the HARD way!):
// • In the ctrlMin and ctrlMax fields, always make sure ctrlMin is less
//   than ctrlMax!! This is less obvious when you use these fields to hold
//   private data (such as id's of cicn's) but the Control Manager DOES
//   do range-checking, and you will get strangely-working buttons if
//   ctrlMin is greater than ctrlMax.
// • Similarly, make sure that ctrlValue is within the range specified by
//   ctrlMin and ctrlMax. Again, the Control Manager does do range-checking,
//   and will coerce the ctrlValue field to the appropriate value if it's
//   greater than ctrlMax or less than ctrlMin.
//   I had wanted the ctrlValue to hold either 0 or 1, and if it was 0 the
//   CDEF was to use the value in the ctrlMin field (let's say 128) to draw
//   the icon. If the value was 1, the CDEF was to use the value in the
//   ctrlMax field (129, for example). The problem, however, was that 0 and 1
//   was out of range specified by the ctrlMin & ctrlMax fields, which were
//   128 and 129. So this didn't work. A fix: just put the id itself in the
//   ctrlValue field, instead of 0 or 1. So it might either contain 128 or 129.

// Version Control:
//	Version 1.1,	Feb 18 1993
//		Added inRectProc, outRectProc

// ----------------------------------------------------------------------

typedef enum CDEFconstants {
	// NeXT CDEF variation codes - yeah there's a lot!
	stdBtnProc		= 0,			// Std btn variation: use window font
	sysFontBtnProc	= 1,			// Uses System font
	geneva9BtnProc	= 2,			// Uses Geneva 9
	cicnBtnProc		= 3,			// Simple CICN btn
	checkBxBtnProc  = 4,
	radioBxBtnProc  = 5,
	popupBtnProc    = 6,			// Uses window font
	geneva9PopupProc= 7,			// Use geneva 9
	inRectProc		= 10,
	outRectProc		= 11,
	sysFontCheckBxBtnProc = 12,
	geneva9CheckBxBtnProc = 13,
	sysFontRadioBxBtnProc = 14,
	geneva9RadioBxBtnProc = 15,

	// NeXT CDEF part codes
	unhilite     = 0,
	btnHilite    = inButton,
	btnDisabled  = 255,
	btnCheckBox  = inCheckBox,

	btnOff		 = 0,
	btnOn		 = 1,

	checkBxHt    = 12,			// Height of check box/radio button
	checkBxWd    = 12,			// Width

	centerJustification = 0,
	leftJustification   = 1,
	rightJustification  = 2,

	leftPadding  = 4,			// Offset to left when drawing text in checkboxes
	rightPadding = 4,
	vertOffset   = -1,			// Pretty-printing for drawing text in buttons

	kBlack		 = 0,			// Some RGBColor constants
	kWhite		 = 65535,
	kLtGray		 = 56797,       // light (56797) or darker (48059)
	kDkGray		 = 17467		// 17467 (lighter) or darker (8738)
};

// MDEFColors struct.
// Used for keeping track of menu colors, from function to function,
// since we aren't allowed to have globals (though we can if we want
// to, using the A4 register, but that's too skanky for me).

typedef struct CDEFcolors {
	struct RGBColor black;
	struct RGBColor white;
	struct RGBColor ltGray;
	struct RGBColor dkGray;
};

// ----------------------------------------------------------------------

pascal OSErr PlotCIconHandle(Rect *theRect, short alignment, short transform,
		CIconHandle theIcon) = { 0x303C, 0x061F, 0xABC9 };

void FillInColor(RGBColor *theColor, unsigned short r, unsigned short g, unsigned short b);
void InitCDEFColors(CDEFcolors *theColors);
void DrawControl(ControlHandle theControl, short variation, long part);
short TestControl(ControlHandle theControl, short variation, long location);
void CalcRgnControl(ControlHandle theControl, long ref);
void CalcRgnControl32Bit(ControlHandle theControl, long ref);

// ----------------------------------------------------------------------

pascal long main(short variation, ControlHandle theControl, short msg, long parameter) {
	long returnVal = 0;

	switch(msg) {
		case initCntl: {
			// For this control def, we don't need to allocate any private
			// data or do any special initialization, so don't do anything
			// here.
		} break;
		
		case drawCntl: {
			DrawControl(theControl, variation, parameter);
		} break;
		
		case testCntl: {
			returnVal = TestControl(theControl, variation, parameter);
		} break;
		
		case calcCRgns: {
			// We receive this msg if running in Sys 6 or Sys 7 with 24-bit
			// addressing on.
			CalcRgnControl(theControl, parameter);
		} break;
		
		case calcCntlRgn: {
			// We receive this msg if running on Sys 7 with 32-bit
			// addressing on. Gotta be 32-bit clean!
			CalcRgnControl32Bit(theControl, parameter);
		} break;
		
		case dispCntl: {
			// If you allocated any private data previously, this is
			// where you de-allocate it.
			// Since we didn't allocate anything, so do nothing.
		} break;
	} // END switch
	
	return(returnVal);
} // END main

// ----------------------------------------------------------------------

void CenterRect(Rect *innerRect, Rect *outerRect) {
	short hDiff = ((outerRect->right - outerRect->left) - (innerRect->right - innerRect->left)) >> 1;
	short vDiff = ((outerRect->bottom - outerRect->top) - (innerRect->bottom - innerRect->top)) >> 1;
	innerRect->left = outerRect->left + hDiff;
	innerRect->right = outerRect->right - hDiff;
	innerRect->top = outerRect->top + vDiff;
	innerRect->bottom = outerRect->bottom - vDiff;
}

// ----------------------------------------------------------------------

void DrawCICN(short iconID, short hiliteStatus, Rect *cdefRect) {
	CIconHandle controlIcon = GetCIcon(iconID);
	if (controlIcon == nil)
		return;

	Rect iconRect = (*controlIcon)->iconPMap.bounds;
	CenterRect(&iconRect, cdefRect);

	if (hiliteStatus == btnHilite)
		OffsetRect(&iconRect, 1, 1);

	if (hiliteStatus == btnDisabled)
		// atNone == 0x00
		// ttDisabled == 0x0001
		PlotCIconHandle(&iconRect, 0x00, 0x0001, controlIcon);
	else
		PlotCIcon(&iconRect, controlIcon);
	DisposeCIcon(controlIcon);
} // END DrawCICN

// ----------------------------------------------------------------------

void DrawColorIcon(ControlHandle theControl, short hiliteStatus, Rect *cdefRect) {
	short iconID = (*theControl)->contrlValue;
	DrawCICN(iconID, hiliteStatus, cdefRect);
} // END DrawColorIcon

// ----------------------------------------------------------------------

void DrawControlText(ControlHandle theControl, short variation, short hiliteStatus,
						Rect *cdefRect, CDEFcolors *cdefColors) {
	short offsetH, offsetV, halfFontDiff, halfCdefDiff;
	short justification = centerJustification;

	short saveFont = (*theControl)->contrlOwner->txFont;
	short saveSize = (*theControl)->contrlOwner->txSize;
	short saveFace = (*theControl)->contrlOwner->txFace;
	short saveMode = (*theControl)->contrlOwner->txMode;

	switch(variation) {
		case sysFontBtnProc:
		case sysFontCheckBxBtnProc:
		case sysFontRadioBxBtnProc:
			TextFont(systemFont);
			TextSize(12);
			TextFace(0);	// plain
			if (variation != sysFontBtnProc)
				justification = leftJustification;
		break;
		
		case geneva9BtnProc:
		case geneva9CheckBxBtnProc:
		case geneva9RadioBxBtnProc:
			TextFont(geneva);
			TextSize(9);
			TextFace(0);
			if (variation != geneva9BtnProc)
				justification = leftJustification;
		break;
		
		case stdBtnProc:
		case checkBxBtnProc:
		case radioBxBtnProc:
			// Use window font & size, so do nothing
			if (variation != stdBtnProc)
				justification = leftJustification;
		break;

		case popupBtnProc:
			justification = leftJustification;
		break;
		
		case geneva9PopupProc:
			TextFont(geneva);
			TextSize(9);
			TextFace(0);
			justification = leftJustification;
		break;

		case inRectProc:
		case outRectProc:
			justification = leftJustification;
		break;
		
		default:
			justification = leftJustification;
		break;
	} // END switch

	FontInfo theInfo;
	GetFontInfo(&theInfo);

	halfFontDiff = theInfo.ascent >> 1;
	halfCdefDiff = (cdefRect->bottom - cdefRect->top) >> 1;
	offsetV = halfCdefDiff - halfFontDiff;

	if (justification == centerJustification) {
		short strWidth = StringWidth((*theControl)->contrlTitle);
		halfFontDiff = strWidth >> 1;
		halfCdefDiff = (cdefRect->right - cdefRect->left) >> 1;
		offsetH = halfCdefDiff - halfFontDiff;
	}
	else {
		offsetH = leftPadding;
	}
	
	RGBForeColor(&cdefColors->black);
	MoveTo(cdefRect->left + offsetH, cdefRect->bottom - offsetV + vertOffset);
	if (hiliteStatus == btnHilite)
		Move(1, 1);
	if (hiliteStatus == btnDisabled)
		TextMode(grayishTextOr);
	DrawString((*theControl)->contrlTitle);
	
	TextFont(saveFont);
	TextSize(saveSize);
	TextFace(saveFace);
	TextMode(saveMode);
} // END DrawControlText

// ----------------------------------------------------------------------

void Draw3DRadio(ControlHandle theControl, short hiliteStatus, Rect *radioRect,
				CDEFcolors *cdefColors) {
	Rect radioR = *radioRect;
	PenSize(2, 2);

	// Draw highlight
	if (hiliteStatus == btnCheckBox)
		RGBForeColor(&cdefColors->white);
	else
		RGBForeColor(&cdefColors->dkGray);
	FrameArc(&radioR, 45, 180);

	// Draw shadow
	if (hiliteStatus == btnCheckBox)
		RGBForeColor(&cdefColors->dkGray);
	else
		RGBForeColor(&cdefColors->white);
	FrameArc(&radioR, 225, 180);

	// Erase circle inside highlight & shadow
	RGBForeColor(&cdefColors->ltGray);
	InsetRect(&radioR, 2, 2);
	PaintOval(&radioR);

	// If button on, show dot
	if ((*theControl)->contrlValue == 1) {
		InsetRect(&radioR, 1, 1);
		PenSize(1, 1);
		RGBForeColor(&cdefColors->dkGray);
		PaintOval(&radioR);
	}
} // END Draw3DRadio

// ----------------------------------------------------------------------

void Draw3DButton(short hiliteStatus, Rect *cdefRect, CDEFcolors *cdefColors) {
	RGBBackColor(&cdefColors->ltGray);
	EraseRect(cdefRect);
	
	if (hiliteStatus == btnHilite || hiliteStatus == btnCheckBox)
		RGBForeColor(&cdefColors->dkGray);
	else
		RGBForeColor(&cdefColors->white);
	// Draw top edge
	MoveTo(cdefRect->left, cdefRect->top);
	LineTo(cdefRect->right - 1, cdefRect->top);
	MoveTo(cdefRect->left, cdefRect->top + 1);
	LineTo(cdefRect->right - 2, cdefRect->top + 1);
	// Draw left edge
	MoveTo(cdefRect->left, cdefRect->top + 2);
	LineTo(cdefRect->left, cdefRect->bottom);
	MoveTo(cdefRect->left + 1, cdefRect->top + 2);
	LineTo(cdefRect->left + 1, cdefRect->bottom - 1);
	if (hiliteStatus == btnHilite || hiliteStatus == btnCheckBox)
		RGBForeColor(&cdefColors->white);
	else
		RGBForeColor(&cdefColors->dkGray);
	// Draw right edge
	MoveTo(cdefRect->right, cdefRect->top);
	LineTo(cdefRect->right, cdefRect->bottom);
	MoveTo(cdefRect->right - 1, cdefRect->top + 1);
	LineTo(cdefRect->right - 1, cdefRect->bottom);
	// Draw bottom edge
	MoveTo(cdefRect->right - 2, cdefRect->bottom);
	LineTo(cdefRect->left + 1, cdefRect->bottom);
	MoveTo(cdefRect->right - 2, cdefRect->bottom - 1);
	LineTo(cdefRect->left + 2, cdefRect->bottom - 1);
} // END Draw3DButton

// ----------------------------------------------------------------------

void DrawPopupButton(short hiliteStatus, Rect *cdefRect, CDEFcolors *cdefColors) {
	// First, draw shadow
	Rect innerRect = *cdefRect;
	OffsetRect(&innerRect, 2, 2);
	RGBBackColor(&cdefColors->dkGray);
	EraseRect(&innerRect);
	OffsetRect(&innerRect, -2, -2);
	
	RGBBackColor(&cdefColors->ltGray);
	EraseRect(cdefRect);

	RGBForeColor(&cdefColors->black);
	FrameRect(cdefRect);

	InsetRect(&innerRect, 1, 1);
	innerRect.right--; innerRect.bottom--;

	RGBForeColor(&cdefColors->white);
	// Draw top edge
	MoveTo(innerRect.left, innerRect.top);
	LineTo(innerRect.right - 1, innerRect.top);
	// Draw left edge
	MoveTo(innerRect.left, innerRect.top);
	LineTo(innerRect.left, innerRect.bottom);

	RGBForeColor(&cdefColors->dkGray);
	// Draw right edge
	MoveTo(innerRect.right, innerRect.top);
	LineTo(innerRect.right, innerRect.bottom);
	// Draw bottom edge
	MoveTo(innerRect.left + 1, innerRect.bottom);
	LineTo(innerRect.right, innerRect.bottom);
} // END DrawPopupButton

// ----------------------------------------------------------------------

void DrawCheckBox(ControlHandle theControl, short variation, short hiliteStatus,
				Rect *cdefRect, CDEFcolors *cdefColors) {
	Rect checkBxRect = *cdefRect;
	checkBxRect.bottom = cdefRect->top + checkBxHt;
	CenterRect(&checkBxRect, cdefRect);
	checkBxRect.right = checkBxRect.left + checkBxWd;
	
	switch(variation) {
		case checkBxBtnProc:
		case sysFontCheckBxBtnProc:
		case geneva9CheckBxBtnProc:
			Draw3DButton(hiliteStatus, &checkBxRect, cdefColors);
			if ((*theControl)->contrlValue == 1) {
				InsetRect(&checkBxRect, 3, 3);
				checkBxRect.right--;
				checkBxRect.bottom--;
				PenNormal();
				RGBForeColor(&cdefColors->black);
				MoveTo(checkBxRect.left, checkBxRect.top + 1);
				LineTo(checkBxRect.right, checkBxRect.bottom + 1);
				MoveTo(checkBxRect.right, checkBxRect.top + 1);
				LineTo(checkBxRect.left, checkBxRect.bottom + 1);
				RGBForeColor(&cdefColors->white);
				MoveTo(checkBxRect.right, checkBxRect.top - 1);
				LineTo(checkBxRect.left, checkBxRect.bottom - 1);
				MoveTo(checkBxRect.left, checkBxRect.top - 1);
				LineTo(checkBxRect.right, checkBxRect.bottom - 1);
				RGBForeColor(&cdefColors->ltGray);
				MoveTo(checkBxRect.left, checkBxRect.top);
				LineTo(checkBxRect.right, checkBxRect.bottom);
				MoveTo(checkBxRect.right, checkBxRect.top);
				LineTo(checkBxRect.left, checkBxRect.bottom);
			}
		break;
		
		case radioBxBtnProc:
		case sysFontRadioBxBtnProc:
		case geneva9RadioBxBtnProc:
			Draw3DRadio(theControl, hiliteStatus, &checkBxRect, cdefColors);
		break;
	} // END switch
} // END DrawCheckBox

// ----------------------------------------------------------------------

void DrawBoxBounds(ControlHandle theControl, short variation, Rect *cdefRect, CDEFcolors *cdefColors) {
	short rectStyle;

	if (variation == inRectProc)
		rectStyle = btnHilite;
	else if (variation == outRectProc)
		rectStyle = unhilite;
		
	Draw3DButton(rectStyle, cdefRect, cdefColors);
} // END DrawBoxBounds

// ----------------------------------------------------------------------

void DrawControl(ControlHandle theControl, short variation, long part) {
	short hiliteStatus = (*theControl)->contrlHilite;
	Rect cdefRect = (*theControl)->contrlRect;
	PenState savePen;
	RGBColor foreSave, backSave;
	GetForeColor(&foreSave);
	GetBackColor(&backSave);
	CDEFcolors cdefColors;
	InitCDEFColors(&cdefColors);
	GetPenState(&savePen);
	PenNormal();
	
	switch(variation) {
		case stdBtnProc:
		case sysFontBtnProc:
		case geneva9BtnProc:
			Draw3DButton(hiliteStatus, &cdefRect, &cdefColors);
			DrawControlText(theControl, variation, hiliteStatus, &cdefRect, &cdefColors);
		break;
		
		case cicnBtnProc:
			Draw3DButton(hiliteStatus, &cdefRect, &cdefColors);
			DrawColorIcon(theControl, hiliteStatus, &cdefRect);
		break;
		
		case checkBxBtnProc:
		case radioBxBtnProc:
		case sysFontCheckBxBtnProc:
		case sysFontRadioBxBtnProc:
		case geneva9CheckBxBtnProc:
		case geneva9RadioBxBtnProc:
			DrawCheckBox(theControl, variation, hiliteStatus, &cdefRect, &cdefColors);
			Rect checkBxRect = cdefRect;
			checkBxRect.left += checkBxWd;
			DrawControlText(theControl, variation, hiliteStatus, &checkBxRect, &cdefColors);
		break;
		
		case inRectProc:
		case outRectProc:
			DrawBoxBounds(theControl, variation, &cdefRect, &cdefColors);
		break;

		case popupBtnProc:
		case geneva9PopupProc:
			DrawPopupButton(hiliteStatus, &cdefRect, &cdefColors);
			Rect textRect = cdefRect;
			InsetRect(&textRect, 2, 2);
			DrawControlText(theControl, variation, hiliteStatus, &textRect, &cdefColors);
	} // END switch

	RGBForeColor(&foreSave);
	RGBBackColor(&backSave);
	SetPenState(&savePen);
} // END DrawControl

// ----------------------------------------------------------------------

short TestControl(ControlHandle theControl, short variation, long location) {
	Point pt;

	pt.v = HiWord(location); 
	pt.h = LoWord(location);

	if (PtInRect(pt, &((*theControl)->contrlRect))) {
		switch(variation) {
			case checkBxBtnProc:
			case radioBxBtnProc:
			case sysFontCheckBxBtnProc:
			case sysFontRadioBxBtnProc:
			case geneva9CheckBxBtnProc:
			case geneva9RadioBxBtnProc:
				return(btnCheckBox);
			break;
			
			case inRectProc:
			case outRectProc:
			case geneva9PopupProc:
				return(unhilite);
			break;

			case popupBtnProc:
				return(unhilite);
			break;

			default:
				return(btnHilite);
			break;
		} // END switch
	}
	else {
		return(unhilite);
	}
} // END TestControl

// ----------------------------------------------------------------------

void CalcRgnControl(ControlHandle theControl, long ref) {
	RgnHandle theRgn;
	theRgn = (RgnHandle)ref;
	theRgn = (RgnHandle)((long)theRgn & 0x7FFFFFFF);	// Clear high bit
	RectRgn(theRgn, &((*theControl)->contrlRect));
} // END CalcRgnControl


// Let's be 32-bit clean, now!
void CalcRgnControl32Bit(ControlHandle theControl, long ref) {
	RectRgn((RgnHandle)ref, &((*theControl)->contrlRect));
} // END CalcRgnControl32Bit

// ----------------------------------------------------------------------

// FillInColor.
// If you use RGBColors, no doubt you'll be using some function similar to this.
// Now why didn't Apple include a function like this? Less overhead, I guess.

void FillInColor(RGBColor *theColor, unsigned short r, unsigned short g, unsigned short b) {
	theColor->red   = r;
	theColor->green = g;
	theColor->blue  = b;
} // END FillInColor

// ----------------------------------------------------------------------

void InitCDEFColors(CDEFcolors *theColors) {
	FillInColor(&theColors->white, kWhite, kWhite, kWhite);
	FillInColor(&theColors->black, kBlack, kBlack, kBlack);
	FillInColor(&theColors->ltGray, kLtGray, kLtGray, kLtGray);
	FillInColor(&theColors->dkGray, kDkGray, kDkGray, kDkGray);
} // END InitCDEFColors

// ----------------------------------------------------------------------