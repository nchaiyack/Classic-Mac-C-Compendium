/* ----------------------------------------------------------------------

	CharCount XFCN
	version 1.0
	4 December 1993
	
	Written by: Paul Celestin
	
	This XFCN returns the number of times a specified character
	appears in the source text.
	
	Requires two parameters: search char, source text.
	
	931204 - 1.0.0 - initial release

---------------------------------------------------------------------- */

# include	<HyperXCMD.h>
# include	<SetUpA4.h>

# define	NUMBER_OF_PARAMS	2

# define	NIL					0
# define	FALSE				0
# define	TRUE				1

# define	BYTEMASK			0xFF
# define	CRC_CONSTANT		0x1021
# define	WORDMASK			0xFFFF
# define	WORDBIT				0x10000

/* ----------------------------------------------------------------------
prototypes
---------------------------------------------------------------------- */

pascal void main(XCmdPtr paramPtr);

/* ----------------------------------------------------------------------
main - here is where it all began...
---------------------------------------------------------------------- */
pascal void main(XCmdPtr paramPtr)
{
	char		*p,*c;
	long		count = 0;
	StringPtr	myString;
 
 	RememberA0();
 	SetUpA4();
 	
	if (paramPtr->paramCount == NUMBER_OF_PARAMS)
	{
		MoveHHi(paramPtr->params[0]);
		MoveHHi(paramPtr->params[1]);
		HLock(paramPtr->params[0]);
		HLock(paramPtr->params[1]);

		p = *(paramPtr->params[0]);
		c = *(paramPtr->params[1]);
		
		while (*c)
		{
			if (*c++ == *p)
				++count;
		}
		NumToStr(paramPtr,count,myString);

		paramPtr->returnValue = PasToZero(paramPtr,myString);
		
		HUnlock(paramPtr->params[0]);
		HUnlock(paramPtr->params[1]);
	}
	
	RestoreA4();
}
