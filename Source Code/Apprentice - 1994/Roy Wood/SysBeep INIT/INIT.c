#include <Traps.h>
#include <SetUpA4.h>

#define goodICON	128
#define badICON		129
#define procID		256
#define patchName	"\pSysBeep Patch"
#define EQSTR		0



void ShowInit(int ID)
{
Handle		procH;
        
	if ((procH = GetResource('PROC', procID)) != 0L)
	{
		HLock(procH);
		CallPascal(ID, -1, *procH);
		HUnlock(procH);
		ReleaseResource(procH);
	}
}



static int	ShiftKey()
{
char	theKeyMap[16];
	
	GetKeys(theKeyMap);
	return(theKeyMap[7] & 0x01);
}



int strcmp(char *string1, char *string2)
{
register char	*s1,*s2;

	s1 = string1;
	s2 = string2;
	
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


main()
{
char 		*moof,installed;
Handle 		myCodeRes;
long		theSize;
long		oldTrap;

	RememberA0();
	SetUpA4();
	
	installed = FALSE;
	
	if (!ShiftKey())
	{
		myCodeRes = GetNamedResource('RoyW',patchName);
				
		if(myCodeRes == nil)
		{
			SysBeep(1);
		}
		
		else
		{
			theSize = GetHandleSize(myCodeRes);
			
			for(moof=*myCodeRes;theSize>0 && strcmp(moof,"moof")!=EQSTR;moof++,theSize--);
			
			if (theSize <= 0L || strcmp(moof,"moof") != EQSTR)
			{
				SysBeep(1);
				SysBeep(1);
			}
			
			else
			{
				oldTrap = NGetTrapAddress((int) (_SysBeep - 0xA000),ToolTrap);
				
				moof += 4L;
				*((long *) moof) = oldTrap;
				
				DetachResource(myCodeRes);
				HUnlock(myCodeRes);
				MoveHHi(myCodeRes);
				HLock(myCodeRes);
				
				NSetTrapAddress((long) StripAddress(*myCodeRes),(_SysBeep - 0xA000),ToolTrap);
					
				installed = TRUE;
			}
		}
	}	
	
	if (!installed)
	{
		ShowInit(badICON);
	}
	
	else
		ShowInit(goodICON);
	
	RestoreA4();
}