/* acurs.c--  animated cursor handling routines used in Oscilloscope.  These are   */
/* from the article by Malcolm H. Teas in MacTechQuarterly, vol2 number 2 and are  */
/* therefore subject to his copyright.                                             */

/* Call InitACurs(theCursorsResourceID) to use an animated cursor.  Then call 		 */
/* SpinACur(countNumber) over and over to make it spin.  If you pass a zero, it will */
/* increment its count.  ShowACurs and HideACurs do as they say.                     */


#include <Types.h>
#include <Resources.h>
#include <Memory.h>
#include <Quickdraw.h>
#include <ToolUtils.h>
#include <Events.h>
#include <Desk.h>
#include "acurs.h"

#define FALSE 0
#define TRUE !FALSE


/*---------------------------------- static typedefs --------------------------------*/
typedef struct Acur
	{
	short totalFrames;	/* number of cursor frames */
	short currentFrame;	/* the currently shown frame */
	long frame[32];		/* high word has the ID */
	}Acur;
	
/*---------------------------------- static variables -------------------------------*/

static int SpinCount=10;
static long count;
static Acur aCurs;
static Acur *aCursR=&aCurs;
static int calledBefore=FALSE;
static int showACurs=FALSE;
static int aCursGoing=FALSE;

/*--------------------------- Externally visible subroutines ------------------------*/
int InitACurs(int id)
	{
	int i;
	Handle cHandle;
	
	/* release any acurs handles we already got */
	if (calledBefore)
		{
		for (i=0;i<aCurs.totalFrames;i++)
			ReleaseResource((Handle)(aCurs.frame[i]));
		}
	
	/* get the acur resource */
	cHandle=GetResource('acur',id);
	if (cHandle==0) return(-1);	/* @#!$$@^*& no resource there!! */
	
	/* load it into the record for this code */
	HLock(cHandle);
	aCursR=(Acur*)*cHandle;
	aCurs.totalFrames=aCursR->totalFrames;
	if (aCurs.totalFrames>32) return(-2); /* something wrong with this resource */
	for (i=0;i<aCursR->totalFrames;i++)
		aCurs.frame[i]=aCursR->frame[i];
	HUnlock(cHandle);
	ReleaseResource(cHandle);
	
	/* now we gotta pick up each of the cursors */
	for (i=0;i<aCurs.totalFrames;i++)
		{
		aCurs.frame[i]=(long)GetCursor(HiWord(aCurs.frame[i]));
		if (aCurs.frame[i]==0) return(-3);
		}
	calledBefore=TRUE;
	aCurs.currentFrame=0;
	aCursGoing=TRUE;
	ShowACurs();
	}
	
int SpinACurs(int increment)
	{
	Cursor *curs;

	if (increment!=0)
		count+=increment;
	else
		count++;
	if (count<SpinCount) return(FALSE);
	minimain(); 	/* take care of update and suspend/resume events */
	
	/* if showing it, unlock the old cursor */
	if (showACurs)
		HUnlock((Handle)(aCurs.frame[aCurs.currentFrame]));
	/* move cursor to the next frame */
	aCurs.currentFrame++;
	if (aCurs.currentFrame>=aCurs.totalFrames) aCurs.currentFrame=0;
	
	/* and show it if necessary */
	if (showACurs)
		{
		HLock((Handle)(aCurs.frame[aCurs.currentFrame]));
		curs=*((CursHandle)(aCurs.frame[aCurs.currentFrame]));
		SetCursor(curs);
		}
	count=0;
	return(TRUE);
	}
	
void ShowACurs(void)
	{
	Cursor *curs;
	
	showACurs=TRUE;
	HLock((Handle)aCurs.frame[aCurs.currentFrame]);
	curs=*((CursHandle)(aCurs.frame[aCurs.currentFrame]));
	SetCursor(curs);
	}

void HideACurs(void)
	{
	showACurs=FALSE;
	HUnlock((Handle)(aCurs.frame[aCurs.currentFrame]));
	aCursGoing=FALSE;
	}

int ACursGoing(void)
	{
	return(aCursGoing);
	}

void SuspendACurs(void)
	{
	showACurs=FALSE;
	HUnlock((Handle)(aCurs.frame[aCurs.currentFrame]));
	}
			
	
	
/*-------------------------------- minimain ------------------------------*/
/*   For handling events while processing, especially in the background   */

#ifndef NOMMAIN

void minimain()
{
	return;
}

#if 0

extern void DoUpdates();
extern void DoActivates();
extern void DoSuspend();
extern void DoResume();

short gWNEisImplemented=TRUE;
short gInBackground=FALSE;
unsigned long gSleepTime=0;	/* this is for foreground, we'll use more in background */

 
void minimain()
   {
	EventRecord minievent;
	
	if (gWNEisImplemented)
		{
		while (WaitNextEvent(activMask+updateMask+app4Mask+mDownMask,&minievent,gSleepTime,0)) 
		/*while (WaitNextEvent(everyEvent,&minievent,gSleepTime,NIL)) */
			{	
			switch (minievent.what)
				{
				case updateEvt:
					{
					DoUpdates(&minievent);
					break;
					}
				case activateEvt:
					{
					DoActivates(&minievent,TRUE);
					break;
					}
				case osEvt:
					{
					if (!BitTst((Ptr)minievent.message,suspendResumeMessage))
						DoSuspend(&minievent);		/* suspend event*/
					else if (BitTst((Ptr)minievent.message,suspendResumeMessage))
						DoResume(&minievent);		/* resume event */
					break;
					}
				default:
					SysBeep(2);
				}
			}
		}
	else while (GetNextEvent(activMask+updateMask,&minievent))
		{
		SystemTask();

		switch (minievent.what)
			{
			case updateEvt:
				{
				DoUpdates(&minievent);
				break;
				}
			case activateEvt:
				{
				DoActivates(&minievent,TRUE);
				break;
				}
			}
		}

	}
  
#endif
#endif
