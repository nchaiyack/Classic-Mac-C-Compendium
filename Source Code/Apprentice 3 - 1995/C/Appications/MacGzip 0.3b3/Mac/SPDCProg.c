/*
 * Copyright (C) 1993  SPDsoft
 * 
 */

/* 22may95,ejo: added these */
#include <Quickdraw.h>
#include <Resources.h>
#include <ToolUtils.h>
#include <Controls.h>
#include <Windows.h>
#include <string.h>
#ifndef CtoPstr
#include <Strings.h>
#define CtoPstr(a) c2pstr (a)
#endif
#ifdef MPW
#define FALSE false
#define TRUE true
#endif
/* --- ejo --- */

#include "SPDProg.h"
/*			SpinCursors by
 *			America Online: LISPer
 *		 	Internet: tree@uvm.edu
 */
#define HiWrd(aLong)	(((aLong) >> 16) & 0xFFFF)

typedef struct				/* The structure of an 'acur' resource */
{
	short numberOfFrames;		/* number of cursors to animate */
	short whichFrame;		/* current frame number */
	CursHandle frame[1];		/* Pointer to the first cursor */
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

#if 1
	/* 22may95,ejo: this is to avoid warning about possible unintended assignment */
	if ((gFrameList = (acurHandle) GetResource('acur',acurID)) != NULL)
#else	
	if((gFrameList = (acurHandle) GetResource('acur',acurID)))
#endif
	{
		/* got it! */
		noErrFlag = TRUE;
		
		while((i<(*gFrameList)->numberOfFrames) && noErrFlag)
		{
			/*
			 * The id of the cursor is stored in
			 * the high word of the frame handle
			 */
			
			cursID = (int) HiWrd((long) (*gFrameList)->frame[i]);
			
			(*gFrameList)->frame[i] = GetCursor(cursID);
			
			if((*gFrameList)->frame[i])
				i++;			/* get the next one */
			else
				noErrFlag=FALSE;	/* foo! we couldn't find the cursor */
		}
	}
	if(noErrFlag)
	{
		(*gFrameList)->whichFrame = 0;
		UseAnimatedCursor=true;
	}
	return noErrFlag;
}

/* Free up the storage used by the current animated cursor and all
   of its frames */
void ReleaseAnimatedCursors (void)	/* 22may95,ejo: added void */
{
	int i;
	
	if(UseAnimatedCursor)
	{
		for(i=0;i<(*gFrameList)->numberOfFrames;i++)
			ReleaseResource((Handle) (*gFrameList)->frame[i]);
		ReleaseResource((Handle) gFrameList);
	}
	UseAnimatedCursor=false;
	SetCursor(&qd.arrow);

}

/************************************************************************************/

/* Opens the MModal dialog. */
/* Just for grins, I'm also saving and restoring it's position in*/
/* a very simple way */

void DrawMovable(WindowPtr drawIt);

Boolean	UseMModalProg=false;

static Rect		barRect; 
static Rect		greyRect;

static ControlHandle	ButtonHndl;

unsigned long int SPDEnd, SPDNow;

Str255	SPDPstr="\p";
char*	SPDpstr=(char*)SPDPstr;

static	Boolean		gModalUp;
	Boolean		gSavedPos;
	Point		gSavedPoint;

#define	kMyModalKind	1000

void InitMovableModal(unsigned long int theEnd)
{
    WindowPtr	temp;
    int		newWid, newLen, wid;
    Rect	r;


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
    
    r = temp->portRect;
    
    SPDNow=0;
    SPDEnd=(theEnd==0?1:theEnd);

    UseMModalProg=true;
    
    	
    CtoPstr(SPDpstr);
    
	wid = (r.right - r.left) - 40;
	newWid = StringWidth(SPDPstr);
	if (newWid > wid )
	{
		newLen = (int) SPDPstr[0];
		wid = wid - CharWidth('�');
		do
		{
			newWid = newWid - CharWidth(SPDPstr[newLen]);
			newLen--;
		}while((newWid > wid) && (SPDPstr[0] != 0x00));

		newLen ++;
		SPDPstr[newLen] = '�';
		SPDPstr[0] = (char)newLen;
	}


        
    barRect.left =	r.left+20;
    barRect.right =	r.right-20;
    barRect.bottom =	r.bottom - 20;
    barRect.top =	barRect.bottom-20;
    
    if(CanBreak)
    {
    
    ButtonHndl = GetNewControl(128, temp);
    
    barRect.right  -= (20+ (*ButtonHndl)->contrlRect.right);
    OffsetRect( &((*ButtonHndl)->contrlRect), barRect.right + 20, barRect.top );

    ShowControl(ButtonHndl);
    
    
    }   


    greyRect.left =	barRect.left+1;
    greyRect.right =	greyRect.left;
    greyRect.bottom =	barRect.bottom-1;
    greyRect.top =	barRect.top+1;
    
    ShowWindow(temp);
    DrawMovable(temp);

    SetPort(temp);
}

void ReleaseMovableModal(void)
{
    WindowPtr temp = FrontWindow();

    
    
 	if(UseMModalProg)
	{
	    if(CanBreak)
	    {
		DisposeControl(ButtonHndl);
	    }

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

 
    BeginUpdate(drawIt);
    GetPort(&tempWP);
    SetPort(drawIt);
    
    	MoveTo(20,20);		/* h, v */
    	DrawString(SPDPstr);

	wbar = ((float)SPDNow/SPDEnd)*(barRect.right-barRect.left) + barRect.left; 
	if ( wbar >= barRect.right ) wbar = barRect.right;
	FrameRect(&barRect);
	greyRect.right=wbar-1;
	
	FillRect( &greyRect, &qd.gray );
	/* ejo: this is how it was */
	/* FillRect( &greyRect, qd.gray );*/

    DrawControls(drawIt);
    EndUpdate(drawIt);
    SetPort(tempWP);
}

/************************************************************************************/

Boolean	SPDSystemTask()
{
	/*typedef long (*MyProcPtr)(); 22may95,ejo: SC dont like this */
	typedef long (*MyProcPtr) (WindowPtr);
	
	EventRecord	theEvent;
	MyProcPtr	drawProc;
	WindowPtr	twindow;
	static unsigned long int TheLastTime;
	ControlHandle	WhichCtl;
	
	if(UseMModalProg)
	{
		InvalRect(&barRect);
	}
	while(GetNextEvent(everyEvent, &theEvent))
	{
	if ((theEvent.what == keyDown)&&(CanBreak))
	{
		if (((theEvent.modifiers & cmdKey) != 0)
		&& ((theEvent.message & charCodeMask) == '.' ))
		{
			SetCursor(&qd.arrow);
			return(true);
		}
	}else if ((theEvent.what == updateEvt)&&(UseMModalProg))
	{
                /* Make sure it's my window before I jump through the refCon */
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
                        
                        case inContent:
                        
                       		GlobalToLocal(&theEvent.where);
                       		
                       		if(inButton == FindControl(theEvent.where, twindow, &WhichCtl))
                       		{
                       			if(0!=TrackControl(WhichCtl, theEvent.where, nil))
                       			{
                         			SetCursor(&qd.arrow);
                         			return(true);
                         		}
                        	}			
                        	break;
	                       
                        
                        default:
                                               				
                        	break;

        		
        	}
	}
	
	if(( theEvent.when-TheLastTime > kTicksCursor )&&(UseAnimatedCursor))
	{
		SetCursor(*((*gFrameList)->frame[(*gFrameList)->whichFrame++]));
		if((*gFrameList)->whichFrame == (*gFrameList)->numberOfFrames)
			(*gFrameList)->whichFrame = 0;
			
		TheLastTime=theEvent.when;
		/* This will be the last time... */
	}
	}
	return(false);
}

/* 20may95,ejo: tell (), what the heck is that, BSD? Have to implement something similar. */

#include "unistd.h"

long tell (int fn);	/* this is avoid missing prototype warning (Metrowerks) */

long tell (int fn)
	{
	return lseek (fn, 0, SEEK_CUR);
	}

