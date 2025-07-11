/*	Player PRO 4.11 PlugIns

	Antoine ROSSET
	16 Tranchees
	1206 GENEVA
	SWITZERLAND
	
	FAX: 022 789 35 03
	Compuserve: 100277,164
*/

/*****************************************************/

/* 	This is an exemple of Player PRO's PlugIns.
	Compiling: 'PLug', ID = 1000
	STR# ID 1000: Menu Name
	
	If you want to reallocate InstrumentPtr:
	
	DisposPtr( InstrumentPtr);			// VERY IMPORTANT
	
	InstrumentPtr = NewPtr( newsize);	// Use NewPtr ONLY !
*/

/********************************************************/

#include "MAD.h"

OSErr main( 	Ptr						*InstrumentPtr,
				struct FileInstrData	*theData,
				long					SelectionStart,
				long					SelectionEnd)
{
long	i;
int		temp;
Ptr		SamplePtr = *InstrumentPtr;

	SamplePtr += SelectionStart;
	
	for( i = 0; i < SelectionEnd - SelectionStart; i++)
	{
		temp = *SamplePtr;
		if( temp >= 0x80) temp -= 0xFF;
			
		temp *= 5;
		temp /= 4;
		if( temp >= 127) temp = 127;
		else if( temp <= -127 ) temp = -127;
			
		*SamplePtr = temp;
		SamplePtr++;
	}
	
	return noErr;
}