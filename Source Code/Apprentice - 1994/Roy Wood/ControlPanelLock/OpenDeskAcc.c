#include "CPanel Lock.h"

pascal void HiliteDefaultButton(DialogPtr theDPtr,int whichItem);
int GetInput(char *thePrompt,char *theText,int defaultItem);
int CommandKey(void);
int OptionKey(void);
int Pstrcmp(char *s1, char *s2);






pascal int main(char *theDAName)
{
int				i,j;
char			permit;
char			*cPanelNameS6,*cPanelNameS7;

	asm
	{
			lea.l	@cpnmS6,a1
			lea.l	cPanelNameS6,a0
			move.l	a1,(a0)
			
			lea.l	@cpnmS7,a1
			lea.l	cPanelNameS7,a0
			move.l	a1,(a0)
			
			bra.s	@skip
			
			dc.b	magicWord
oldTrap:	dc.b	0,0,0,0
cpnmS6:		dc.b	CPANELNAMES6,0
cpnmS7:		dc.b	CPANELNAMES7,0
			dc.b	0

skip:		nop
	}
	
	DebugStr("\pCalling Elvis");
	
	permit = TRUE;
	
	if (theDAName[0])
	{
		for (j=0,i=theDAName[0];i > 0 && theDAName[j] == cPanelNameS6[j];i--,j++);
		
		if (i <= 0)
		{
			if (!OptionKey() || !CommandKey() || !CheckPassword())
				permit = FALSE;
		}
		
		if (permit)
		{
			for (j=0,i=theDAName[0];i > 0 && theDAName[j] == cPanelNameS7[j];i--,j++);
			
			if (i <= 0)
			{
				if (!OptionKey() || !CommandKey() || !CheckPassword())
					permit = FALSE;
			}
		}
	}
	
	if (!permit)
	{
		SysBeep(1);
		return;
	}
	
	asm 
	{
			move.l	@oldTrap, a0
			unlk	a6
			jmp		(a0)
	}
}




int OptionKey(void)
{
char	theKeyMap[16];

	GetKeys(theKeyMap);
	
	if (theKeyMap[7] & 0x04)
		return(TRUE);
	else
		return(FALSE);
}




int CommandKey(void)
{
char	theKeyMap[16];

	GetKeys(theKeyMap);
	
	if (theKeyMap[6] & 0x80)
		return(TRUE);
	else
		return(FALSE);
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



int CheckPassword()
{
DialogPtr		theDPtr,tempDPtr;
int				itemHit, type, returnCode,i;
int				oldResFRefNum,myResFRefNum;
Handle			theItem,masterPassWord;
Rect			tempRect;
GrafPtr			oldPort;
char			userPassWord[256];
EventRecord		theEvent;

	oldResFRefNum = CurResFile();
	
	if ((myResFRefNum = OpenResFile(myResFileName)) == -1)
	{
		SysBeep(1);
		return(FALSE);
	}
	
	UseResFile(myResFRefNum);
	
	GetPort(&oldPort);

	InitCursor();
	
	theDPtr = GetNewDialog(PASSWDDLOG, 0L,(WindowPtr) -1L);
	SetPort(theDPtr);
	
	/*CenterWindow(theDPtr);*/
	ShowWindow(theDPtr);
	
	((DialogPeek) theDPtr)->aDefItem = 1;
	
	GetDItem(theDPtr, 5, &type, &theItem, &tempRect);
	SetDItem(theDPtr, 5, type, HiliteDefaultButton, &tempRect);
	
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
	
	
	GetDItem(theDPtr, 4, &type, &theItem, &tempRect);
	
	if (itemHit==1)
	{
		masterPassWord = GetResource('STR ',PASSWORDSTR);
		
		if (masterPassWord == nil || Pstrcmp(userPassWord,(char *) *masterPassWord)!=EQSTR)
			returnCode = FALSE;
		
		else
			returnCode = TRUE;
		
		ReleaseResource(masterPassWord);
	}
	else
		returnCode = FALSE;
	
	DisposDialog(theDPtr);
	SetPort(oldPort);
	
	CloseResFile(myResFRefNum);
	UseResFile(oldResFRefNum);
	
	return(returnCode);
}
