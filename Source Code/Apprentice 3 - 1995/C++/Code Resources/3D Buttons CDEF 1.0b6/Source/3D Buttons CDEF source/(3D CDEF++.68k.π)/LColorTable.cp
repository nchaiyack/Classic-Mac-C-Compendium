/*
	Public domain by Zig Zichterman.
*/
/*
	LColorTable
	
	A color table from control's or window's auxilliary 
	color info.
	
	This class "borrows" the color table handle--it does
	not own the color table handle, it does not dispose it.
	The control or window would be very upset if that
	happened.
	
	CCTabHandle and CTabHandle share the same format,
	so this class works fine for control or window
	custom color tables.
	
	12/27/94	zz	h	rename from LGBColorTable
	11/27/94	zz	h	initial write
*/
#include "LColorTable.h"

#include <Windows.h>

/**************************************************************************
	LColorTable()										[public]
	
	Create an empty color table. Use GetControlColors() or
	GetWindowColors() to fill the table
**************************************************************************/
LColorTable::LColorTable(void)
{
	mColorTable = NULL;
}

/**************************************************************************
	GetControlColors()									[public]
	
	Get the custom color table for a control. Does nothing if the
	control does not specify custom colors
**************************************************************************/
void
LColorTable::GetControlColors(
	const ControlHandle	inControlH)
{
	// assume we won't find a color table
	mColorTable	= NULL;
	
	// get the control's auxilliary info
	AuxCtlHandle	auxControlInfoH	= NULL;
	
	// if the aux info is the default aux info, there's no custom colors
	Boolean	isCustom
		= ::GetAuxiliaryControlRecord(inControlH,&auxControlInfoH);
		
	// run through a few tests to make sure we have a truly custom color
	// table, not the default table
	
	// !!! GetAuxiliaryControlRecord() does NOT seem to return FALSE
	// !!! if the CNTL lacks a cctb--it returns TRUE, with an aux
	// !!! info record that contains a reference to the default
	// !!! control color table. Testing "isCustom" is nigh useless.
	if (isCustom == false) {
		return;
	}
	
	// do we have a valid handle to info?
	if ((auxControlInfoH == NULL) || (*auxControlInfoH == NULL)) {
		return;
	}
	
	// get the default aux info
	AuxCtlHandle	defaultInfoH	= NULL;
	::GetAuxiliaryControlRecord(NULL,&defaultInfoH);
	// if we got the default aux info
	if (defaultInfoH && *defaultInfoH) {
		// does our control's aux info point to the same color table
		// as the default control aux info?
		if ((**auxControlInfoH).acCTable == (**defaultInfoH).acCTable) {
			// no custom colors. Refuse to use default
			return;
		}
	}

	// the color table is custom for this control. OK to use
	mColorTable	= (CTabHandle) (**auxControlInfoH).acCTable;
}

/**************************************************************************
	GetWindowColors()									[public]
	
	Get the custom color table for a window. Does nothing if
	the window does not specify custom colors
**************************************************************************/
void
LColorTable::GetWindowColors(
	const WindowPtr	inWindow)
{
	// assume we won't find a color table
	mColorTable	= NULL;
	
	// get the window's auxilliary info
	AuxWinHandle	auxWindowInfoH	= NULL;
	Boolean	isCustom = ::GetAuxWin(inWindow,&auxWindowInfoH);

	// !!! GetAuxWin() does NOT seem to return FALSE
	// !!! if the window lacks a wctb--it returns TRUE, with an aux
	// !!! info record that contains a reference to the default
	// !!! window color table. Testing "isCustom" is nigh useless.
	if (isCustom == false) {
		return;
	}
	
	// do we have a valid handle to info?
	if ((auxWindowInfoH == NULL) || (*auxWindowInfoH == NULL)) {
		return;
	}
	
	// get the default aux info
	AuxWinHandle	defaultInfoH	= NULL;
	::GetAuxWin(NULL,&defaultInfoH);
	// if we got the default aux info
	if (defaultInfoH && *defaultInfoH) {
		// does our window's aux info point to the same color table
		// as the default window aux info?
		if ((**auxWindowInfoH).awCTable == (**defaultInfoH).awCTable) {
			// no custom colors. Refuse to use default
			return;
		}
	}

	// use the color table from the aux info
	mColorTable	= (**auxWindowInfoH).awCTable;
}

/**************************************************************************
	GetColorForPart()									[public]
	
	Scan our list of colors for one whose part code matches what
	the caller seeks. Return FALSE if no match, TRUE (and the color)
	if a match
**************************************************************************/
Boolean								// did we get a color
LColorTable::GetColorForPart(
	PartT		inPart,				// content, frame, etc.
	RGBColor	&outColor) const	// returned color
{
	// assume the worst
	outColor.red	=
	outColor.green	=
	outColor.blue	= 0;	// black
	
	// do we have a color table handle?
	if ((mColorTable == NULL) || (*mColorTable == NULL)) {
		// nope
		return false;
	}
	
	// is the requested value listed? Iterate through all colors
	// until you find a match or run out of parts
	const short	ColorCount	= (**mColorTable).ctSize + 1;
	for (short i = 0; i < ColorCount; i++) {
		// if you find a match
		if ((**mColorTable).ctTable[i].value == inPart) {
			// return the match
			outColor	= (**mColorTable).ctTable[i].rgb;
			return true;
		}
	}
	
	// never found a color for the requested value
	return false;
}

	

