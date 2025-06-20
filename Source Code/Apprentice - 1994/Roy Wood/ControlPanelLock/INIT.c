#include <Traps.h>
#include <SetUpA4.h>
#include "CPanel Lock.h"


int strcmp(char *s1, char *s2)
{
	while(*s1 == *s2 && *s2 && *s1)
	{
		s1++; 
		s2++;
	}
	
	if (*s1 == *s2) 
		return(EQSTR);
	else
		return(!EQSTR);
}



void ShowInit(int ID)
{
Handle		procH;
        
	if ((procH = GetResource('PROC', ShowIconID)) != 0L)
	{
		HLock(procH);
		CallPascal(ID, -1, *procH);
		HUnlock(procH);
		ReleaseResource(procH);
	}
}



main()
{
char 		*moof,installed;
Handle 		myCodeRes;
long		theSize;

	RememberA0();
	SetUpA4();
	
	installed = FALSE;
	
	myCodeRes = GetNamedResource('RoyW',patchName);

	if(myCodeRes != nil)
	{
		theSize = GetHandleSize(myCodeRes);
		for(moof=*myCodeRes;theSize>0 && strcmp(moof,"Moof")!=EQSTR;moof++,theSize--);
		
		if (theSize>0L && strcmp(moof,"Moof")==EQSTR)
		{
			moof += 4L;
			
			*((long *) moof) = NGetTrapAddress((_OpenDeskAcc - 0xA000),ToolTrap);
			
			DetachResource(myCodeRes);
			HUnlock(myCodeRes);
			MoveHHi(myCodeRes);
			HLock(myCodeRes);
			
			NSetTrapAddress((long) StripAddress(*myCodeRes),(_OpenDeskAcc - 0xA000),ToolTrap);
				
			installed = TRUE;
		}
	}
	
	if (!installed)
	{
		ShowInit(badICON);
		SysBeep(1);
	}
	
	else
		ShowInit(goodICON);
	
	RestoreA4();
}