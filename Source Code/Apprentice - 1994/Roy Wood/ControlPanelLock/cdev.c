#include <SetUpA4.h>
#include "CPanel Lock.h"

#define SETPSWDBTN	6


void Pstrcpy(char *s1,char *s2);
int Pstrlen(char *s1);
int GetPassword(char *thePrompt,char *userPassWord);
pascal void HiliteDefaultButton(DialogPtr theDPtr,int whichItem);
int Pstrcmp(char *s1, char *s2);



pascal long main(int message,int item, int numItems, int CPanelID, EventRecord *theEvent, long cdevValue, DialogPtr CPDialog)
{
long		retCode;
Handle		itemHandle;
Rect		itemRect;
int			itemType;
CInfoPBRec	myPBRec;
char		passWord[256],passWord2[256];
Handle		passWordHandle;
int			errCode;


	RememberA0();
	SetUpA4();
	
	retCode = cdevValue;
	
	switch (message)
	{
		case initDev:
			break;
			
		case hitDev:
			item -= numItems;
			
			if (item == SETPSWDBTN)
			{
				passWordHandle = GetResource('STR ',PASSWORDSTR);
			
				if (passWordHandle && 
					GetPassword("Enter Old Password:",passWord) == 1 && 
					Pstrcmp(passWord,(char *) *passWordHandle) == EQSTR &&
					GetPassword("Enter New Password:",passWord) == 1 &&
					GetPassword("Confirm New Password:",passWord2) == 1 &&
					Pstrcmp(passWord,passWord2) == EQSTR)
				{
					SetHandleSize(passWordHandle,(long) Pstrlen(passWord) + 1L);
					Pstrcpy((char *) *passWordHandle,passWord);
					
					ChangedResource(passWordHandle);
					WriteResource(passWordHandle);
				}
				
				else
					SysBeep(1);
			}
			
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



void Pstrcpy(char *s1,char *s2)
{
register long	length;

	length = *s2;
	
	do
		*s1++ = *s2++;
		
	while (length--);
}


int Pstrlen(char *s1)
{
	return((int) *((unsigned char *) s1));
}


pascal void HiliteDefaultButton(DialogPtr theDPtr,int whichItem)
{
int			type;
Handle		theItem;
Rect		theRect;

	GetDItem(theDPtr, ((DialogPeek) theDPtr)->aDefItem, &type, &theItem, &theRect);
	PenNormal();
	PenSize(3,3);
	InsetRect(&theRect,-4,-4);
	FrameRoundRect(&theRect,16,16);
	PenSize(1,1);
}


int Pstrcmp(char *s1, char *s2)
{
int		i;

	i = *s1 + 1;
	
	while (i > 0 && *s1 == *s2)
	{
		s1++; 
		s2++;
		i--;
	}
	
	if (i <= 0)
		return(EQSTR);
	else
		return(!EQSTR);
}



int GetPassword(char * thePrompt,char *userPassWord)
{
DialogPtr		theDPtr,tempDPtr;
int				itemHit, type, returnCode,i;
Handle			theItem;
Rect			tempRect;
GrafPtr			oldPort;
EventRecord		theEvent;

	GetPort(&oldPort);

	InitCursor();
	
	theDPtr = GetNewDialog(PASSWDDLOG, 0L,(WindowPtr) -1L);
	SetPort(theDPtr);
	
	/*CenterWindow(theDPtr);*/
	ShowWindow(theDPtr);
	
	((DialogPeek) theDPtr)->aDefItem = 1;
	
	GetDItem(theDPtr, 5, &type, &theItem, &tempRect);
	SetDItem(theDPtr, 5, type, HiliteDefaultButton, &tempRect);
	
	GetDItem(theDPtr, 3, &type, &theItem, &tempRect);
	CtoPstr(thePrompt);
	SetIText(theItem, thePrompt);
	PtoCstr(thePrompt);
		
	i = 1;
	userPassWord[0] = 0;
	
	do 
	{
		itemHit = 0;
		
		while (!GetNextEvent(everyEvent,&theEvent))
		{
			SystemTask();
		
			if (((DialogPeek) theDPtr)->editField>=0 && ((DialogPeek) theDPtr)->textH)
				TEIdle(((DialogPeek) theDPtr)->textH);
		}
		
		if (theEvent.what==keyDown || theEvent.what==autoKey)
		{
			if ((theEvent.message & charCodeMask)=='\r' || (theEvent.message & charCodeMask)==0x03)
				itemHit = ((DialogPeek) theDPtr)->aDefItem;
			
			else if (i < 255)
			{
				userPassWord[i++] = theEvent.message & charCodeMask;
				userPassWord[0] += 1;
				theEvent.message &= ~charCodeMask;
				theEvent.message |= '¥';
				
				tempDPtr = 0L;
				itemHit = 0;
				
				DialogSelect(&theEvent,&tempDPtr,&itemHit);
			}
			
			else
				SysBeep(1);
		}
		
		else
		{
			tempDPtr = (DialogPtr) 0L;
				
			if (!IsDialogEvent(&theEvent) || !DialogSelect(&theEvent,&tempDPtr,&itemHit) || tempDPtr!=theDPtr)
				itemHit = 0;
		}
		
	} while (itemHit!=1 && itemHit!=2);
	
	DisposDialog(theDPtr);
	SetPort(oldPort);
	
	return(itemHit);
}
