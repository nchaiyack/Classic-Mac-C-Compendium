/* ----------------------------------------------------------------------

	BitDepth XFCN
	version 1.0
	25 September 1993
	
	Written by: Paul Celestin
	
	Copyright ��1993 Celestin Company
	All Rights Reserved
	
	This XFCN returns the bit depth of the main screen.
	
	930925 - 1.0.0 - initial release

---------------------------------------------------------------------- */

# include	<HyperXCMD.h>
# include	<OSUtils.h>
# include	<QuickDraw.h>
# include	<SetUpA4.h>

pascal void main(XCmdPtr paramPtr)
{
	long			depth;
	StringPtr		myString;
	GDHandle		curDev;
	PixMapHandle	myPixMap;
	Rect			bounds;
	SysEnvRec		myComputer;
 
 	RememberA0();
 	SetUpA4();
 	
	if (paramPtr->paramCount == 0)
	{
		SysEnvirons( 2,&myComputer );
		if (myComputer.hasColorQD)
		{
			curDev = GetMainDevice();
			myPixMap = (**curDev).gdPMap;
			depth = (**myPixMap).pixelSize;
		}
		else depth = 1;

		NumToStr(paramPtr,depth,myString);
		paramPtr->returnValue = PasToZero(paramPtr,myString);
	}
	
	RestoreA4();
}
