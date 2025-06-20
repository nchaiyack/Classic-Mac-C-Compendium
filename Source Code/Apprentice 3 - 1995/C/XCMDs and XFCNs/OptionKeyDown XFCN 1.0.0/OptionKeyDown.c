/* ----------------------------------------------------------------------

	OptionKeyDown XFCN
	version 1.0
	27 September 1993
	
	Written by: Paul Celestin
	
	Copyright ��1993 Celestin Company
	All Rights Reserved
	
	This XFCN returns true if the option key is down.
	
	930927 - 1.0.0 - initial release

---------------------------------------------------------------------- */

# include	<Events.h>
# include	<HyperXCMD.h>
# include	<OSUtils.h>
# include	<QuickDraw.h>
# include	<Traps.h>
# include	<SetUpA4.h>

Boolean bit_test(Ptr p, int n)
{
	return(!!(p[n/8] & 1L<<(n%8)));
}

pascal void main(XCmdPtr paramPtr)
{
	long			it;
	StringPtr		myString;
	KeyMap			myKeys;
	Boolean			isTrue;
 
 	RememberA0();
 	SetUpA4();
 	
	if (paramPtr->paramCount == 0)
	{
		GetKeys(myKeys);
		if (bit_test((Ptr)myKeys, 0x3a)) /* 0x3a is the option key */
			paramPtr->returnValue = PasToZero(paramPtr,"\ptrue");
		else
			paramPtr->returnValue = PasToZero(paramPtr,"\pfalse");
	}
	
	RestoreA4();
}

