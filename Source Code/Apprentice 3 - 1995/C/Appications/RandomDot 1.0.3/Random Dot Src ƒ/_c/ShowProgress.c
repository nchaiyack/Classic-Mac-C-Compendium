/* ShowProgress.c - 
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */
#include <QDOffscreen.h>
#include "RandomDotMain.h"
#include "RandomDotRes.h"
#include "RandomDotWin.h"

#include "Progress.h"

#include "Error.h"
#include "Utils.h"

typedef struct ProgressRec {
	unsigned long	start;
	unsigned long	last;
	Integer			width;
	DialogPtr		win;
	Rect			rFullBar;
	Rect			rBar;
	Str255			labelText;
}ProgressRec;

static ProgressRec progress;

/* SetProgressLabelText
 */
void SetProgressLabelText(StringPtr s){
	StrMove(s, progress.labelText);
}

/* DrawProgress - draw the progress item.
 */
static pascal void DrawProgress(DialogPtr dp, Integer i){
	GrafPtr	savePort;
	Rect	r;

	GetPort(&savePort);
	SetPort(dp);
	GetDIRect(i, &progress.rFullBar);
	FrameRect(&progress.rFullBar);
	progress.rBar = progress.rFullBar;
	InsetRect(&progress.rBar, 2, 2);
	r = progress.rBar;
	r.right = r.left + progress.width;
	if(progress.width > 0){
		PaintRect(&r);
	}
	SetPort(savePort);
}

/* ProgressWidth - current width
 */
static Integer ProgressWidth(LongInt n, LongInt max){
	return (progress.rBar.right - progress.rBar.left - 2)*n / max;
}

/* UpdateProgress - draw just the changed part
 */
static void UpdateProgress(Integer newWidth){
	GrafPtr	savePort;
	Rect	r;

	r = progress.rBar;
	r.right = r.left + newWidth;
	r.left += progress.width;
	GetPort(&savePort);
	SetPort(progress.win);
	PaintRect(&r);
	SetPort(savePort);

	progress.width = newWidth;
}


/* ShowProgress - 
	caller calls us, with n ranging from 0 to max.
	we measure the rate at which n goes from 0 to max/10, and do nothing for
	1/2 second. if whole thing will take less than 3 seconds, we do nothing,
	else we put up progress meter with cancel button.
	
	Call this with n == max to reset it.
 */
OSErr	ShowProgress(LongInt n, LongInt max){
	OSErr		errCode;
	unsigned long now;
	static UserItemUPP	drawprogressUPP = NIL;
	Rect		**rh;
	Integer	newWidth;
	EventRecord e;
	Integer		item;
	DialogPtr	dp;
	CGrafPtr	savePort;
	GDHandle	saveGD;

	errCode = noErr;
	now = TickCount();
	if(0 == n){
		progress.start = TickCount();
	}else if(n == max){
		if(NIL != progress.win){
			HideWindow(progress.win);
			progress.width = 0;
		}
	}else if(now == progress.last){
		/* done */
	}else{
		GetGWorld(&savePort, &saveGD);
		progress.last = now;
		if((NIL == progress.win || NOT ((WindowPeek) progress.win)->visible) && 
			now - progress.start > 20 && (n*100L)/max < 10){

			if(NIL == progress.win){
				progress.win = GetNewDialog(rProgress, NIL, (WindowPtr) -1L);
				SetPort(progress.win);
				if(NIL == drawprogressUPP){
					drawprogressUPP = NewUserItemProc(DrawProgress);
				}
				SetDIHandle(1, (Handle) drawprogressUPP);
				if(NIL != (rh = (Rect **) GetPreferencesHandle('Wind', 128))){
					if(TitleBarOnScreen(*rh)){	
						MoveWindow(progress.win, (**rh).left, (**rh).top, FALSE);
					}
					DisposeHandle((Handle) rh);
				}
			}
			SetIText(GetWDIHandle(progress.win, 3), progress.labelText);
			ShowWindow(progress.win);
			SelectWindow(progress.win);
			DrawDialog(progress.win);
		}else if(NIL != progress.win && ((WindowPeek) progress.win)->visible){
			newWidth = ProgressWidth(n, max);
			if(newWidth != progress.width){
				UpdateProgress(newWidth);
				WaitNextEvent(everyEvent, &e, NIL, 0);
				if(IsDialogEvent(&e) && DialogSelect(&e, &dp, &item) &&
					progress.win == dp){

					errCode = eUserCancel;
				}else{
					DialogOnTopGoEvent(&e);
				}
			}
		}
		SetGWorld(savePort, saveGD);
	}
	return errCode;
}

/* RecordProgress - save position of the progress meter in the preferences
	file.
 */
void RecordProgress(void){
	WindowPtr	savePort;
	Rect		wind;
	Handle		h;
	OSErr		errCode;

	if(NIL == progress.win || -1 == prefResFile){
		return;
	}
	GetPort(&savePort);
	SetPort(progress.win);
	wind = qd.thePort->portRect;
	LocalToGlobal(&topLeft(wind));
	LocalToGlobal(&botRight(wind));
	errCode = PtrToHand(&wind, &h, sizeof(Rect));
	if(noErr == errCode){ 
		SavePreferencesResource(h, 'Wind', 128);
		DisposeHandle(h);
	}
	SetPort(savePort);
}
