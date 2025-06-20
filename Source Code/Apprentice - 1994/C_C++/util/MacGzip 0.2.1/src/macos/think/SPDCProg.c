/*
 * Copyright (C) 1993  SPDsoft
 * 
 */

#include "SPDProg.h"
/*			SpinCursors by
 *			America Online: LISPer
 *		 	Internet: tree@uvm.edu
*/
#define HiWrd(aLong)	(((aLong) >> 16) & 0xFFFF)
#define MAX_L_STR	54

typedef struct				/* The structure of an 'acur' resource */
{
	short numberOfFrames;		/* number of cursors to animate */
	short whichFrame;		/* current frame number */
	CursHandle frame[];		/* Pointer to the first cursor */
} acur, *acurPtr, **acurHandle;

unsigned long int	SPDBackCycles;

Boolean		UseAnimatedCursor=false;
Boolean		CanBreak=false;
acurHandle	gFrameList;

Boolean InitAnimatedCursors(short acurID)
{
	register short i=0;
	register short cursID;
	Boolean noErrFlag = FALSE;
	SPDBackCycles=0;
	
	if((gFrameList = (acurHandle) GetResource('acur',acurID))) {
		/* got it! */
		noErrFlag = TRUE;
		while((i<(*gFrameList)->numberOfFrames) && noErrFlag) {
			/* The id of the cursor is stored in the high word of the frame handle */
			cursID = (int) HiWrd((long) (*gFrameList)->frame[i]);
			(*gFrameList)->frame[i] = GetCursor(cursID);
			if((*gFrameList)->frame[i])
				i++;			/* get the next one */
			else
				noErrFlag=FALSE;	/* foo! we couldn't find the cursor */
		}
	}
	if(noErrFlag) {

		(*gFrameList)->whichFrame = 0;
		UseAnimatedCursor=true;
	}
	return noErrFlag;
}

/* Free up the storage used by the current animated cursor and all
   of its frames */
void ReleaseAnimatedCursors()
{
	int i;
	
	if(UseAnimatedCursor)
	{
		for(i=0;i<(*gFrameList)->numberOfFrames;i++)
			ReleaseResource((Handle) (*gFrameList)->frame[i]);
		ReleaseResource((Handle) gFrameList);
	}
	UseAnimatedCursor=false;
	SetCursor(&arrow);

}

/************************************************************************************/

/* Opens the MModal dialog. */
/* Just for grins, I'm also saving and restoring it's position in*/
/* a very simple way */

void DrawMovable(WindowPtr drawIt);
void UpdateBar(void);

Boolean	UseMModalProg=false;

static Rect		barRect; 
static Rect		greyRect;
static short int mypat[] = { 0x55AA,0x55AA,0x55AA,0x55AA };	/* Grey */
unsigned long int SPDEnd, SPDNow;

Str255	SPDPstr="\p";
char*	SPDpstr=(char*)SPDPstr;

static	Boolean		gModalUp;
	Boolean		gSavedPos;
	Point		gSavedPoint;

#define	kMyModalKind	1000

void InitMovableModal(unsigned long int theEnd)
{
    WindowPtr temp;

    temp = GetNewWindow(150, nil, (WindowPtr)-1);
    /* install drawing proc */
    SetWRefCon(temp, (long)DrawMovable);
    
    /* set the kind to my MModalwindow */
    ((WindowPeek)temp)->windowKind = kMyModalKind;
    gModalUp = true;
    
    if (gSavedPos) {
        /* move it to the saved position */
        /* move it to 0,0 to avoid any math at all */
        MoveWindow(temp, 0, 0, false);
        MoveWindow(temp, gSavedPoint.h, gSavedPoint.v, false);
    }
    
    SPDNow=0;
    SPDEnd=(theEnd==0?1:theEnd);

    UseMModalProg=true;
    
    	
    CtoPstr(SPDPstr);
    
    if((unsigned char)*SPDPstr > MAX_L_STR)
    {
    	*SPDPstr = (unsigned char)MAX_L_STR;
    	*(SPDPstr+MAX_L_STR) = '�';
    }
    
    barRect.left=temp->portRect.left+20;
    barRect.right=temp->portRect.right-20;
    barRect.bottom=temp->portRect.bottom - 20;
    barRect.top=barRect.bottom-20;
    
    greyRect.left=barRect.left+1;
    greyRect.right=greyRect.left;
    greyRect.bottom=barRect.bottom-1;
    greyRect.top=barRect.top+1;
    
    ShowWindow(temp);

    SetPort(temp);
}

void ReleaseMovableModal(void)
{
    WindowPtr temp = FrontWindow();

	if(UseMModalProg)
	{
	    /* the front window really should be my modal window */
	    /* if it isn't, I'm bailing.*/
	    gSavedPos = true;
	    gSavedPoint.h = temp->portRect.left;
	    gSavedPoint.v = temp->portRect.top;
	    LocalToGlobal(&gSavedPoint);
	    if (((WindowPeek)temp)->windowKind == kMyModalKind)
	        CloseWindow(temp);
	
	    gModalUp = false;
	    UseMModalProg=false;
	}
}

static void DrawMovable(WindowPtr drawIt)
{
    WindowPtr tempWP;
    unsigned long int wbar;
    Str255 stemp;
 
    BeginUpdate(drawIt);
    GetPort(&tempWP);
    SetPort(drawIt);
    
    	MoveTo(20,20);/*h,v*/
    	DrawString(SPDPstr);

	wbar = SPDNow*(barRect.right-barRect.left)/SPDEnd+barRect.left; 
	if ( wbar >= barRect.right ) wbar = barRect.right;
	FrameRect(&barRect);
	greyRect.right=wbar-1;
	FillRect(&greyRect,mypat);

    DrawControls(drawIt);
    EndUpdate(drawIt);
    SetPort(tempWP);
}

static void UpdateBar(void)
{
    WindowPtr tempWP,drawIt;
    unsigned long int wbar;

    drawIt = FrontWindow();
    GetPort(&tempWP);
    SetPort(drawIt);
    

	wbar = SPDNow*(barRect.right-barRect.left)/SPDEnd+barRect.left; 
	if ( wbar >= barRect.right ) wbar = barRect.right;
	FrameRect(&barRect);
	greyRect.right=wbar-1;
	FillRect(&greyRect,mypat);

    DrawControls(drawIt);
    EndUpdate(drawIt);
    SetPort(tempWP);
}

/************************************************************************************/

Boolean	SPDSystemTask()
{
	typedef long (*MyProcPtr)();
	
	EventRecord	theEvent;
	MyProcPtr	drawProc;
	WindowPtr	twindow;
	static unsigned long int TheLastTime;
	
	if(UseMModalProg)
	{
		SPDNow+=10;
		UpdateBar();
	}
	GetNextEvent(everyEvent, &theEvent);
	{
	if ((theEvent.what == keyDown)&&(CanBreak))
	{
		if (((theEvent.modifiers & cmdKey) != 0)
		&& ((theEvent.message & charCodeMask) == '.' ))
		{
			SetCursor(&arrow);
			return(true);
		}
	}else if ((theEvent.what == updateEvt)&&(UseMModalProg))
	{
                /* Mkae sure it's my window before I jump through the refCon */
                /* Why, since DA's have they're own layer in 7.0? */
                /* BECAUSE there are other people in the universe who will */
                /* add things to your windowList.BalloonWriter, for example, */
                /* so you still need to be careful */
                if (((WindowPeek)theEvent.message)->windowKind == kMyModalKind)
                {
                    /* get the drawing proc from the refCon */
                    drawProc = (MyProcPtr)GetWRefCon((WindowPtr)theEvent.message);
                    /* jump to it */

                    drawProc((WindowPtr)theEvent.message);

                }
        }else if (theEvent.what == mouseDown)
        {
        	switch (FindWindow(theEvent.where, &twindow))
        	{
        		case inSysWindow:
                        		/* pass to the system */
                        		SystemClick(&theEvent, twindow);
                        		break;
                        case inDrag:
                        	if (twindow != FrontWindow() && gModalUp)
                        	{
                            /* don't do anything, can't drag a back window */
                           	SysBeep(1);
                        	} else
                        	{
                            DragWindow(twindow, theEvent.where, &qd.screenBits.bounds);
                        	}
                        	break;
                        default:	break;

        		
        	}
	}
	
	if(( theEvent.when-TheLastTime > kTicksCursor )&&(UseAnimatedCursor))
	{
		SetCursor(*((*gFrameList)->frame[(*gFrameList)->whichFrame++]));
		if((*gFrameList)->whichFrame == (*gFrameList)->numberOfFrames)
			(*gFrameList)->whichFrame = 0;
			
		TheLastTime=theEvent.when;
	}
	}
	return(false);
}