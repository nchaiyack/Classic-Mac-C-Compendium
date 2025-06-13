/* ----------------------------------------------------------------------

	ScreenWidth XFCN
	version 1.0
	25 September
	
	Written by: Paul Celestin
	
	Copyright ©Ê1993 Celestin Company
	All Rights Reserved
	
	This XFCN returns the width of the main screen in pixels.
	
	930925 - 1.0.0 - initial release

---------------------------------------------------------------------- */

# include	<HyperXCMD.h>
# include	<QuickDraw.h>
# include	<SetUpA4.h>

pascal void main(XCmdPtr paramPtr)
{
	long		width;
	StringPtr	myString;
	GDHandle	curDev;
	Rect		bounds;
 
 	RememberA0();
 	SetUpA4();
 	
	if (paramPtr->paramCount == 0)
	{

		curDev = GetMainDevice();
		bounds = (**curDev).gdRect;
		width = bounds.right - bounds.left;

		NumToStr(paramPtr,width,myString);
		paramPtr->returnValue = PasToZero(paramPtr,myString);
	}
	
	RestoreA4();
}
