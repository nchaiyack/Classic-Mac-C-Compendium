#include <Traps.h>
#include <SetUpA4.h>

#include "PBGetInfo.h"



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


char *strcpy(char *s1,char *s2)
{
	do
		*s1 = *s2++;
		
	while (*s1++);
}


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



main()
{
char 		*moof,installed;
Handle 		myCodeRes;
Handle		lockStatusHandle;
long		theSize;
SysEnvRec	theWorld;
CInfoPBRec	myPBRec;
char		volName[256];
char		tempString[256];
int			errCode;
int			vRefNum;
long		freeBytes;


	RememberA0();
	SetUpA4();
	
	installed = FALSE;
	
	/* get teh startup volume */
	
	errCode = GetVInfo(0,volName,&vRefNum,&freeBytes);
	
	if (errCode != noErr)
		SysBeep(1);
	
	else
	{
		/* get the system folder from the SysEnvirons theWorld.sysVRefNum */
		
		errCode = SysEnvirons(1,&theWorld);
		
		if (errCode != noErr)
		{
			SysBeep(1);
			SysBeep(1);
		}
		
		else
		{
			volName[0] = 0;
			
			myPBRec.dirInfo.ioCompletion = 0L;
			myPBRec.dirInfo.ioNamePtr = (StringPtr) volName;
			myPBRec.dirInfo.ioVRefNum = theWorld.sysVRefNum;
			myPBRec.dirInfo.ioFDirIndex = -1;
			myPBRec.dirInfo.ioDrDirID = 0L;
			
			/* get the dirID and parent dirID of the system folder */
			
			errCode = PBGetCatInfo(&myPBRec,FALSE);
			
			if (errCode != noErr)
			{
				SysBeep(1);
				SysBeep(1);
				SysBeep(1);
			}
			
			else
			{
				/* load the actual patch which is stored in a resource of type 'RoyW' */
				
				myCodeRes = GetNamedResource('RoyW',patchName);
			
				if(myCodeRes == nil)
				{
					SysBeep(1);
					SysBeep(1);
					SysBeep(1);
					SysBeep(1);
				}
				
				else
				{
					/* search for the characters 'Moof' in the trap patch code, since this marks the start of the memory reserved for storing pseudo-globals */
					
					theSize = GetHandleSize(myCodeRes);
					for(moof=*myCodeRes;theSize>0 && strcmp(moof,"Moof")!=EQSTR;moof++,theSize--);
					
					if (theSize <= 0L || strcmp(moof,"Moof")!=EQSTR)
					{
						SysBeep(1);
						SysBeep(1);
						SysBeep(1);
						SysBeep(1);
						SysBeep(1);
					}
					
					else
					{
						/* store the pseudo-globals */
						
						moof += 4L;
						*((long *) moof) = NGetTrapAddress((int) (_HFSDispatch - 0xA000),OSTrap);
						moof += 4L;
						*((int *) moof) = theWorld.sysVRefNum;
						moof += 2L;
						*((int *) moof) = vRefNum;
						moof += 2L;
						*((long *) moof) = myPBRec.dirInfo.ioDrDirID;
						moof += 4L;
						*((long *) moof) = myPBRec.dirInfo.ioDrParID;
						
						DetachResource(myCodeRes);
						HUnlock(myCodeRes);
						MoveHHi(myCodeRes);
						HLock(myCodeRes);
						
						NSetTrapAddress((long) StripAddress(*myCodeRes),(_HFSDispatch - 0xA000),OSTrap);
						
						/* lock the system folder! */
						
						strcpy(tempString,LOCKMESSAGE);
						
						myPBRec.dirInfo.ioCompletion = 0L;
						myPBRec.dirInfo.ioNamePtr = (StringPtr) tempString;
						myPBRec.dirInfo.ioVRefNum = 0;
						myPBRec.dirInfo.ioFDirIndex = 0;
						myPBRec.dirInfo.ioDrDirID = 0L;
						
						lockStatusHandle = GetNamedResource('RoyW',lockStatus);
				
						if (lockStatusHandle)
							strcpy(tempString,(char *) *lockStatusHandle);
						
						errCode = PBGetCatInfo(&myPBRec,FALSE);
						
						if (errCode == noErr)
							installed = TRUE;
					}
				}
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