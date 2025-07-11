#include "CatSearch.h"

#include <stdio.h>

main()
{
	short				loopy;
	OSErr				err;
	long				numberFound;
	FSSpec				MPWSpec;
	unsigned long		startTime, endTime;
	
	#define kMaxMatches 100
	FSSpec				pMatchBuffer[kMaxMatches];

	if (FALSE)	// Test "Set search criteria and search once, and then search repeatedly"
	{
		printf("Searching for all MPW tools.\n");
		GetDateTime(&startTime);
		err = CatSearch(&numberFound,
						csVRefNum, (short) -2,
						csMatchPtr, pMatchBuffer, (long) kMaxMatches,
						csFInfoFDType, 'MPST',
						csFInfoFDCreator, 'MPS ',
						csEndList);
		while ((err == noErr) || (err == eofErr))
		{
			for (loopy = 0; loopy < numberFound; ++loopy)
			{
			//	PtoCstr(pMatchBuffer[loopy].name);
			//	printf("%7ld %s\n", pMatchBuffer[loopy].parID, pMatchBuffer[loopy].name);
			//	CtoPstr((char*) pMatchBuffer[loopy].name);
			}
	
			if (err == noErr)
				err = CatSearch(&numberFound, csContinue);
			else
				break;
		}
		GetDateTime(&endTime);
		printf("Search took %ld seconds.\n", endTime - startTime);
	}

	if (TRUE)	// Test "Set search criteria, and then search repeatedly"
	{
		printf("Searching for all MPW tools.\n");
		GetDateTime(&startTime);
		err = CatSearch(&numberFound,
						csVRefNum, (short) -2,
						csMatchPtr, pMatchBuffer, (long) kMaxMatches,
						csFInfoFDType, 'MPST',
						csFInfoFDCreator, 'MPS ',
						csInitOnly,
						csEndList);
	
		while (err == noErr)
		{
			err = CatSearch(&numberFound, csContinue);
			if ((err == noErr) || (err == eofErr))
			{
				for (loopy = 0; loopy < numberFound; ++loopy)
				{
				//	PtoCstr(pMatchBuffer[loopy].name);
				//	printf("%7ld %s\n", pMatchBuffer[loopy].parID, pMatchBuffer[loopy].name);
				//	CtoPstr((char*) pMatchBuffer[loopy].name);
				}
			}
		}
		GetDateTime(&endTime);
		printf("Search took %ld seconds.\n", endTime - startTime);
	}

	if (TRUE)	// Test "Search for file only within a given subdirectory"
	{
		printf("Searching for all MPW tools in the MPW folder.\n");
		err = CatSearch(&numberFound,
						csVRefNum, (short) -2,
						csMatchPtr, &MPWSpec, (long) 1,
						csFullName, "\pMPW Shell",
						csFInfoFDType, 'APPL',	// or else it will find my alias!
						csEndList);
	
		if (numberFound == 0)
			return;
		
		GetDateTime(&startTime);
		err = CatSearch(&numberFound,
						csVRefNum, MPWSpec.vRefNum,
						csMatchPtr, pMatchBuffer, (long) kMaxMatches,
						csFInfoFDType, 'MPST',
						csFInfoFDCreator, 'MPS ',
						csSearchInDirectory, MPWSpec.parID,
						csInitOnly,
						csEndList);
	
		while (err == noErr)
		{
			err = CatSearch(&numberFound, csContinue);
			if ((err == noErr) || (err == eofErr))
			{
				for (loopy = 0; loopy < numberFound; ++loopy)
				{
				//	PtoCstr(pMatchBuffer[loopy].name);
				//	printf("%7ld %s\n", pMatchBuffer[loopy].parID, pMatchBuffer[loopy].name);
				//	CtoPstr((char*) pMatchBuffer[loopy].name);
				}
			}
		}
		GetDateTime(&endTime);
		printf("Search took %ld seconds.\n", endTime - startTime);
	}
}
