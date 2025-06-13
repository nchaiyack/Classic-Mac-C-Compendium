/* ----------------------------------------------------------------------

	CRCValue XFCN
	version 1.0
	4 December 1993
	
	Written by: Paul Celestin
	
	This XFCN returns a 16 bit CRC of a specific piece of text.
	
	Requires one parameter: source text.
	
	931204 - 1.0.0 - initial release

---------------------------------------------------------------------- */

# include	<HyperXCMD.h>
# include	<SetUpA4.h>

# define	NUMBER_OF_PARAMS	1

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
void CalcCRC(unsigned short v);

/* ----------------------------------------------------------------------
globals
---------------------------------------------------------------------- */

unsigned short CRC = 0;

/* ----------------------------------------------------------------------
main - here is where it all began...
---------------------------------------------------------------------- */
pascal void main(XCmdPtr paramPtr)
{
	char		*p;
	StringPtr	myString;
 
 	RememberA0();
 	SetUpA4();
 	
	if (paramPtr->paramCount == NUMBER_OF_PARAMS)
	{
		MoveHHi(paramPtr->params[0]);
		p = *(paramPtr->params[0]);
		while (*p)
		{
			CalcCRC(*p++);
		}
		NumToStr(paramPtr,CRC,myString);
		paramPtr->returnValue = PasToZero(paramPtr,myString);
	}
	
	RestoreA4();
}

/* ----------------------------------------------------------------------
CalcCRC
---------------------------------------------------------------------- */
void CalcCRC(unsigned short v)
{
	register int i;
	register unsigned short temp = CRC;

	for (i = 0; i < 8; i++)
	{
		v <<= 1;
		if ((temp <<= 1) & WORDBIT)
			temp = (temp & WORDMASK) ^ CRC_CONSTANT;
		temp ^= (v >> 8);
		v &= BYTEMASK;
	}
	CRC = temp;
}
