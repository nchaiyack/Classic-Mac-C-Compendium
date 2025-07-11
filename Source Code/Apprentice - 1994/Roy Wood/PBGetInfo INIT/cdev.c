#include "PBGetInfo.h"
#include <SetUpA4.h>


#define CHECKBOX	5


char *strcpy(char *s1,char *s2);
int strlen(char *s1);
int strcmp(char *s1, char *s2);


pascal long main(int message,int item, int numItems, int CPanelID, EventRecord *theEvent, long cdevValue, DialogPtr CPDialog)
{
long		retCode;
Handle		itemHandle;
Rect		itemRect;
int			itemType;
CInfoPBRec	myPBRec;
char		tempString[256];
Handle		lockStatusHandle;
int			errCode;


	RememberA0();
	SetUpA4();
	
	retCode = cdevValue;
	
	switch (message)
	{
		case initDev:
			GetDItem(CPDialog,CHECKBOX+numItems,&itemType,&itemHandle,&itemRect);
			
			if (itemHandle)
			{
				lockStatusHandle = GetNamedResource('RoyW',lockStatus);
				
				if (lockStatusHandle)
				{
					if (strcmp((char *) *lockStatusHandle,LOCKMESSAGE) == EQSTR)
						SetCtlValue((ControlHandle) itemHandle,1);
					else
						SetCtlValue((ControlHandle) itemHandle,0);
				}
				
				else
					SetCtlValue((ControlHandle) itemHandle,0);
			}
			
			break;
			
		case hitDev:
			if (item - numItems == CHECKBOX)
			{
				GetDItem(CPDialog,CHECKBOX+numItems,&itemType,&itemHandle,&itemRect);
				
				if (itemHandle)
				{
					SetCtlValue((ControlHandle) itemHandle,!GetCtlValue((ControlHandle) itemHandle));
					
					strcpy(tempString,LOCKMESSAGE);
					
					myPBRec.dirInfo.ioCompletion = 0L;
					myPBRec.dirInfo.ioNamePtr = (StringPtr) tempString;
					myPBRec.dirInfo.ioVRefNum = 0;
					myPBRec.dirInfo.ioFDirIndex = 0;
					myPBRec.dirInfo.ioDrDirID = 0L;
					
					lockStatusHandle = GetNamedResource('RoyW',lockStatus);
			
					if (lockStatusHandle)
					{
						if (GetCtlValue((ControlHandle) itemHandle))
							strcpy(tempString,LOCKMESSAGE);
						else
							strcpy(tempString,UNLOCKMESSAGE);
						
						SetHandleSize(lockStatusHandle,(long) strlen(tempString) + 1L);
						strcpy((char *) *lockStatusHandle,tempString);
						
						ChangedResource(lockStatusHandle);
						WriteResource(lockStatusHandle);
					}
					
					else
					{
						SysBeep(1);
						SysBeep(1);
						SysBeep(1);
					}
					
					errCode = PBGetCatInfo(&myPBRec,FALSE);
					
					if (errCode != noErr)
					{
						SysBeep(1);
						SysBeep(1);
						SysBeep(1);
						SysBeep(1);
					}
				}
				
				else
				{
					SysBeep(1);
					SysBeep(1);
				}
			}
			
			else
				SysBeep(1);
			
			break;
		
		case closeDev:
			break;
		
		case nulDev:
			break;
		
		case updateDev:
			break;
		
		case activDev:
			break;
		
		case deactivDev:
			break;
		
		case keyEvtDev:
			break;
		
		case macDev:
			retCode = 1;
			break;
		
		case undoDev:
			break;
		
		case cutDev:
			break;
		
		case copyDev:
			break;
		
		case pasteDev:
			break;
		
		case clearDev:
			break;
		
		case cursorDev:
			InitCursor();
			break;
	}
	
	RestoreA4();
	
	return(retCode);
}



char *strcpy(char *s1,char *s2)
{
	do
		*s1 = *s2++;
		
	while (*s1++);
}


int strlen(char *s1)
{
int		length;

	length = 0;
	
	while (*s1++)
		length++;
	
	return(length);
}


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
