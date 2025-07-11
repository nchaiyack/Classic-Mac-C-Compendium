// ===========================================================================
// "Connection Tool Skeleton in C" for the Communications Toolbox
// 
//	Copyright � 1994 Peter J. Creath
//	All Rights Reserved Worldwide
// ===========================================================================

#include "ConnToolCommon.h"

// ===========================================================================
// Function prototypes
// ===========================================================================
extern pascal long	main(ConnHandle hConn, short msg, Ptr pInput, Ptr *pOutput, long language);
extern long	DoTranslate(Ptr inputStr, Ptr *outputStr, long fromLanguage, long toLanguage);

// ===========================================================================
// main()
// 	This function is the entry point for the 'cloc' resource.  It passes control to the appropriate
// 	subroutines, depending on the incoming message.  This can probably remain unchanged.
// ===========================================================================

pascal long main(ConnHandle hConn, short msg, Ptr pInput, Ptr *pOutput, long language)
{
long			rtnValue;

	switch (msg)
		{
			case cmL2English:
				rtnValue = DoTranslate(pInput, pOutput, language, verUS);
				break;
			
			case cmL2Intl:
				rtnValue = DoTranslate(pInput, pOutput, verUS, language);
				break;
		}
}

// ===========================================================================
// DoTranslate()
//	This function translates an input configuration string from one language to another.  It should
//	return 0 if no problem, nonzero if there is a problem.  This routine needs to allocate outputStr.
//	If the language requested is not supported, return 0 but leave outputStr = NIL.
// ===========================================================================
long	DoTranslate(Ptr inputStr, Ptr *outputStr, long fromLanguage, long toLanguage)
{
	// Whatever...
}