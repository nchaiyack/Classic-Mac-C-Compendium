#include <stdio.h>
#include <oops.h>
#include <Movies.h>

#include "myQuickTime.h"

#include "Dlog.h"
#include "Alert_Class.h"

#include "vcr.h"
#include "mycolors.h"

extern EventRecord gEvent;
extern Alert_Class *myAlert;
extern QT *myQT;

Rect myRects[4]; // This global saves calls to GetDItem over and over
				// these rects are user Items in the resource file
				// used to place the buttons where I want them.

Boolean blink = false;
long oldTick, newTick;
int currentrect = 0;
Boolean clockSet = false;

extern Boolean opened;


/*....................................................................*/
VCR::VCR()
{
	Buttons myButtons;
	int i;
	int rsrcNum;
	
	myButtons = (*this).myButtons;
	
	
	for(i = 0,rsrcNum = BASE_ICON-1; i < LAST_BUTTON; i++,rsrcNum++) {
		myButtons.button[i] = false;
		myButtons.buttonRNum[i] = rsrcNum;
	}
	(*this).myButtons = myButtons;
	vcr_mode = stopped;
}
/*....................................................................*/
VCR::~VCR()
{
	Buttons myButtons;
	int i;
	int rsrcNum;
	
	myButtons = (*this).myButtons;
	
	
	for(i = 0,rsrcNum = BASE_ICON; i < LAST_BUTTON; i++,rsrcNum++) {
		myButtons.button[i] = false;
		myButtons.buttonRNum[i] = rsrcNum;
	}
	(*this).myButtons = myButtons;
}
/*....................................................................*/
void VCR::loadResource( int number)	// load resource from resource file
{
	DialogPtr myDlog;
	
	/*-------------------------
	Shadow for purging problems
	-------------------------*/
	myDlog = this->myDlog;
	rsrcNumber = number;
	myDlog = GetNewDialog(number,0L,(WindowPtr)-1L);
	/*-------------------------
	Copy back from shadow
	-------------------------*/
	this->myDlog = myDlog;

	if ( myDlog == 0L  ) {
		SysBeep(7);
		(*myAlert).AlertStop("\pFatal Error",
							"\pUnable to allocate new dialog",
							"\pDlog::loadResource",NULL);
	} /* end if */
	
	CouldDialog(rsrcNumber);	// could be purged so lock it down



	
	GetDItem(myDlog,PROGRESS_1,&itemType,&itemHandle,&itemRect);
	myRects[0] = itemRect;
	
	GetDItem(myDlog,PROGRESS_2,&itemType,&itemHandle,&itemRect);
	myRects[1] = itemRect;

	GetDItem(myDlog,PROGRESS_3,&itemType,&itemHandle,&itemRect);
	myRects[2] = itemRect;

	GetDItem(myDlog,PROGRESS_4,&itemType,&itemHandle,&itemRect);
	myRects[3] = itemRect;

	
}
/*....................................................................*/
int VCR::HandleDialogItem(int itemHit, Point thePoint, int thePart)
{
	DialogPtr myDlog;
	Buttons myButtons;
	
	myDlog = this->myDlog;
	myButtons = (*this).myButtons;
	
	switch ( itemHit  ) {
		case PLAY_BUTTON:
			if (!opened) { SysBeep(7); break;}
			if (myButtons.button[PLAY_BUTTON] == false) {
				(*this).myButtons = myButtons;
				ButtonsAndValues(PLAY_BUTTON);
				(*this).vcr_mode = playing;
				(*myQT).PlayMovie();
			}
		break;
		
		case STOP_BUTTON:
			if (!opened) { SysBeep(7); break;}

			if (myButtons.button[STOP_BUTTON] == false) {
				(*this).vcr_mode = stopped;
				(*this).myButtons = myButtons;
				ButtonsAndValues(STOP_BUTTON);
				myButtons = (*this).myButtons;
				myButtons.button[STOP_BUTTON] = false;
				(*this).myButtons = myButtons;
				RedrawButtons();
				DrawAllProgress(sl_darkGray);
				(*myQT).StopMyMovie();
			}
		break;
		
		case FF_BUTTON:
			if (!opened) { SysBeep(7); break;}
			if (myButtons.button[FF_BUTTON] == false) {
				(*this).vcr_mode = fastforward;
				(*this).myButtons = myButtons;
				ButtonsAndValues(FF_BUTTON);
				(*myQT).FFMyMovie();
			}
		break;
		
		case GO_START_BUTTON:
			if (!opened) { SysBeep(7); break;}
			if (myButtons.button[GO_START_BUTTON] == false) {
				(*this).myButtons = myButtons;
				(*this).vcr_mode = gotostart;
				ButtonsAndValues(GO_START_BUTTON);
				myButtons = (*this).myButtons;
				myButtons.button[GO_START_BUTTON] = false;
				(*this).myButtons = myButtons;
				RedrawButtons();
				(*myQT).GoStartMyMovie();
			}
			break;
			
		case GO_END_BUTTON:
			if (!opened) { SysBeep(7); break;}
			if (myButtons.button[GO_END_BUTTON] == false) {
				(*this).vcr_mode = gotoend;
				(*this).myButtons = myButtons;
				ButtonsAndValues(GO_END_BUTTON);
				myButtons = (*this).myButtons;
				myButtons.button[GO_END_BUTTON] = false;
				(*this).myButtons = myButtons;
				RedrawButtons();
				(*myQT).GoEndMyMovie();
			}
			break;
			
		case RW_BUTTON:
			if (!opened) { SysBeep(7); break;}
			if (myButtons.button[RW_BUTTON] == false) {
				(*this).vcr_mode = re_wind;
				(*this).myButtons = myButtons;
				ButtonsAndValues(RW_BUTTON);
				(*myQT).RWMyMovie();
			}
			break;
		case CLOCK_BOX:
			clockSet = true;
			break;
			
		case VCR_BOX:{
				Rect bRect;
				SelectWindow(myDlog);
				bRect = screenBits.bounds;
				DragWindow(myDlog,gEvent.where, &bRect);
			}	
			break;
		
		case TAPE_SLOT:
			if (opened) {
				(*myQT).CloseMovie();
				opened = false;
			}
			else {
				opened = (*myQT).OpenMovie();
			}	
		
			break;
			
		case VOLUME_UP:
			if (!opened) {SysBeep(7);break;}
			(*myQT).HandleVolume(UP);
			FlashButton(VOLUME_UP);
			break;
			
		case VOLUME_DOWN:
			if (!opened) {SysBeep(7);break;}
			(*myQT).HandleVolume(DOWN);
			FlashButton(VOLUME_DOWN);
			break;
			
		case CLOSE_VCR_BOX:
			FlashButton(CLOSE_VCR_BOX);
			HideWindow(myDlog);
			break;	
		default: break;
	} /* end switch */
	//(*this).myButtons = myButtons;
}
/*....................................................................*/
void VCR::RedrawDialog()
{
	DialogPtr myDlog;
	GrafPtr oldPort;
	int i;
	
	GetPort(&oldPort);
	myDlog = (*this).myDlog;
	
	SetPort(myDlog);
	DrawDialog(myDlog);
	RedrawButtons();
	RedrawStatus();
	RedrawTime();
	SetPort(oldPort);
}
/*....................................................................*/
void VCR::ButtonsAndValues(int itemHit)
{
	DialogPtr myDlog;
	int i;
	Buttons myButtons;
	
	
	myDlog = (*this).myDlog;
	myButtons = (*this).myButtons;

	for(i = 1; i < LAST_BUTTON; i++) {
		if (i == itemHit) {
			myButtons.button[i] = true;
		}
		else {
			myButtons.button[i] = false;
		}
	}

	(*this).myButtons = myButtons;
	RedrawButtons();
}
/*....................................................................*/
void VCR::RedrawButtons()
{
	DialogPtr myDlog;
	int itemType;
	Handle itemHandle;
	Rect itemRect;
	CIconHandle	icon;
	int i;
	GrafPtr oldPort;
	Buttons myButtons;
	RGBColor fc,bc;
	RGBColor temp;
	
	GetForeColor(&fc);
	GetBackColor(&bc);
	
	temp.red = temp.green = temp.blue = 0xffff;
	RGBBackColor(&temp);
	temp.red = temp.green = temp.blue = 0x0000;
	RGBForeColor(&temp);
	
	GetPort(&oldPort);
	myDlog = (*this).myDlog;
	myButtons = (*this).myButtons;
	
	SetPort(myDlog);
	
	for(i = 1; i < LAST_BUTTON; i++) {
		if (myButtons.button[i] == true) {
			GetDItem(myDlog,i,&itemType,&itemHandle,&itemRect);
			icon = GetCIcon(myButtons.buttonRNum[i]);
			if (icon == NULL) {
				SysBeep(7);
				(*myAlert).AlertStop("\pFatal Error",
							"\pUnable to get cicn resource",
							"\pVCR::RedrawButtons",NULL);
			}
			
			HLock( icon );
			HLock( (**icon).iconData );
		
			(**icon).iconPMap.baseAddr = *(**icon).iconData;
			(**icon).iconMask.baseAddr = (Ptr)&(**icon).iconMaskData;
			
			CopyBits( &(**icon).iconPMap, &myDlog->portBits,
						&(**icon).iconPMap.bounds, &itemRect, notSrcCopy, 0 );
		
			HUnlock( (**icon).iconData );
			HUnlock( icon );
			DisposCIcon(icon);
		}
		else {
			GetDItem(myDlog,i,&itemType,&itemHandle,&itemRect);
			icon = GetCIcon(myButtons.buttonRNum[i]);
			if (icon == NULL) {
				SysBeep(7);
				(*myAlert).AlertStop("\pFatal Error",
							"\pUnable to get cicn resource",
							"\pVCR::RedrawButtons",NULL);
			}
			HLock(icon);
			PlotCIcon(&itemRect, icon);
			HUnlock(icon);
			DisposCIcon(icon);
		}
	}
	
	//(*this).myButtons = myButtons;
	
	/* draw volume controls */
	GetDItem(myDlog,VOLUME_UP,&itemType,&itemHandle,&itemRect);
	icon = GetCIcon(VOLUME_UP_CICN);
	if (icon == NULL) {
		SysBeep(7);
		(*myAlert).AlertStop("\pFatal Error",
					"\pUnable to get cicn resource",
					"\pVCR::RedrawButtons",NULL);
	}
	HLock(icon);
	PlotCIcon(&itemRect, icon);
	HUnlock(icon);
	DisposCIcon(icon);
	
	GetDItem(myDlog,VOLUME_DOWN,&itemType,&itemHandle,&itemRect);
	icon = GetCIcon(VOLUME_DOWN_CICN);
	if (icon == NULL) {
		SysBeep(7);
		(*myAlert).AlertStop("\pFatal Error",
					"\pUnable to get cicn resource",
					"\pVCR::RedrawButtons",NULL);
	}
	HLock(icon);
	PlotCIcon(&itemRect, icon);
	HUnlock(icon);
	DisposCIcon(icon);
	
	GetDItem(myDlog,CLOSE_VCR_BOX,&itemType,&itemHandle,&itemRect);
	icon = GetCIcon(CLOSE_CICN);
	if (icon == NULL) {
		SysBeep(7);
		(*myAlert).AlertStop("\pFatal Error",
					"\pUnable to get cicn resource",
					"\pVCR::RedrawButtons",NULL);
	}
	HLock(icon);
	PlotCIcon(&itemRect, icon);
	HUnlock(icon);
	DisposCIcon(icon);
	
	RGBForeColor(&fc);
	RGBBackColor(&bc);
	
	SetPort(oldPort);
}
/*....................................................................*/
void VCR::RedrawStatus()
{
	DialogPtr myDlog;
	int itemType;
	Handle itemHandle;
	Rect itemRect;
	int i;
	GrafPtr oldPort;
	RGBColor fc,bc;
	Buttons myButtons;

	myDlog = (*this).myDlog;
	
	
	GetForeColor(&fc);
	GetBackColor(&bc);
	
	
	GetPort(&oldPort);
	myButtons = (*this).myButtons;
	
	GetDItem(myDlog,STATUS_BOX,&itemType,&itemHandle,&itemRect);
	SetColor(sl_black);
	PaintRect(&itemRect);
	
	SetColor(sl_darkGray);
	
	// draw border	
	MoveTo(itemRect.left,itemRect.bottom + 1);
	LineTo(itemRect.right + 1,itemRect.bottom + 1);
	
	MoveTo(itemRect.right + 1, itemRect.top);
	LineTo(itemRect.right + 1, itemRect.bottom + 1);
	
	UpdateProgress();
	SetPort(oldPort);
	
	RGBForeColor(&fc);
	RGBBackColor(&bc);
	
}
/*....................................................................*/
void VCR::RedrawTime()
{
	DialogPtr myDlog;
	int itemType;
	Handle itemHandle;
	Rect itemRect;
	int i;
	GrafPtr oldPort;
	RGBColor fc,bc;
	DateTimeRec myTime;
	char time[80];
	char day[6];
	
	
	GetForeColor(&fc);
	GetBackColor(&bc);
	
	GetPort(&oldPort);
	myDlog = (*this).myDlog;
	myButtons = (*this).myButtons;
	
	SetPort(myDlog);
	
	GetDItem(myDlog,CLOCK_BOX,&itemType,&itemHandle,&itemRect);
	SetColor(sl_black);
	PaintRect(&itemRect);
	
	SetColor(sl_darkGray);
	// draw border	
	MoveTo(itemRect.left,itemRect.bottom + 1);
	LineTo(itemRect.right + 1,itemRect.bottom + 1);
	
	MoveTo(itemRect.right + 1, itemRect.top);
	LineTo(itemRect.right + 1, itemRect.bottom + 1);
	
	GetTime(&myTime);
	
	switch(myTime.dayOfWeek) {
		case 1:
			sprintf(day,"SUN");
			break;
		case 2:
			sprintf(day,"MON");
			break;
		case 3:
			sprintf(day,"TUE");
			break;
		case 4:
			sprintf(day,"WED");
			break;
		case 5:
			sprintf(day,"THU");
		break;
		case 6:
			sprintf(day,"FRI");
			break;
		case 7:
			sprintf(day,"SAT");
			break;
		default: break;
	}
	
	if (!clockSet) {
		if (blink) {
			SetColor(sl_white);
			sprintf(time,"12:00");
			MoveTo(itemRect.left+12,itemRect.bottom-12);
			DrawString(CtoPstr(time));
			
			//sprintf(day,"???");
			//MoveTo(itemRect.right - 30, itemRect.bottom-12);
			//DrawString(CtoPstr(day));
		}
		
	}
	else {	
		
		if (blink) {
			SetColor(sl_white);
			//sprintf(time,"%02d:%02d %02d",myTime.hour,myTime.minute,myTime.second);
			sprintf(time,"%02d:%02d",myTime.hour,myTime.minute);
			MoveTo(itemRect.left+12,itemRect.bottom-12);
			DrawString(CtoPstr(time));
			
			MoveTo(itemRect.right - 30, itemRect.bottom-12);
			DrawString(CtoPstr(day));
		}
		else {
			SetColor(sl_white);
			//sprintf(time,"%02d %02d %02d",myTime.hour,myTime.minute,myTime.second);
			sprintf(time,"%02d %02d",myTime.hour,myTime.minute);
			MoveTo(itemRect.left+12,itemRect.bottom-12);
			DrawString(CtoPstr(time));
			
			MoveTo(itemRect.right - 30, itemRect.bottom-12);
			DrawString(CtoPstr(day));
		}
	}
	
	
	SetPort(oldPort);
	
	RGBForeColor(&fc);
	RGBBackColor(&bc);
}
/*....................................................................*/
void VCR::UpdateProgress()
{
	DialogPtr myDlog;
	GrafPtr oldPort;
	RGBColor fc,bc;
	Buttons myButtons;
	
	GetForeColor(&fc);
	GetBackColor(&bc);
	
	GetPort(&oldPort);
	myDlog = (*this).myDlog;
	myButtons = (*this).myButtons;
	
	SetPort(myDlog);
	
	newTick = TickCount();
	if (myButtons.button[PLAY_BUTTON] == true) {
		if ( (newTick - oldTick) > 25) {
			int i,j;
			
			i = currentrect;
			currentrect++;
			
			if (currentrect > 3) currentrect = 0;			
			j = currentrect;
			Draw2Rects2Colors(myRects[i],sl_darkGray,myRects[j],sl_white);
			oldTick = newTick;
		}
	}
	else if (myButtons.button[FF_BUTTON] == true) {
		if ( (newTick - oldTick) > 10) {
			int i,j;
			
			i = currentrect;
			currentrect++;
			
			if (currentrect > 3) currentrect = 0;			
			j = currentrect;
			Draw2Rects2Colors(myRects[i],sl_darkGray,myRects[j],sl_white);
			oldTick = newTick;
		}
	}
	else if (myButtons.button[RW_BUTTON] == true) {
		if ( (newTick - oldTick) > 10) {
			int i,j;
			
			i = currentrect;
			
			currentrect--;
				
			if (currentrect < 0) currentrect = 3;
			j = currentrect;
			
			Draw2Rects2Colors(myRects[i],sl_darkGray,myRects[j],sl_white);
		
			oldTick = newTick;
		}
	}
	else {
		
		if ( (newTick - oldTick) > 120) {	
			DrawAllProgress(sl_darkGray);
			oldTick = newTick;
			currentrect = 0;

		}

	}
	
	
	
	SetPort(oldPort);
	
	RGBForeColor(&fc);
	RGBBackColor(&bc);
}	
/*....................................................................*/
void VCR::DrawAllProgress(int color)
{
	int i;
	GrafPtr oldPort;
	RGBColor fc,bc;
	
	
	GetForeColor(&fc);
	GetBackColor(&bc);
	
	GetPort(&oldPort);
	myDlog = (*this).myDlog;
	SetPort(myDlog);
	
	SetColor(color);
	for(i = 0; i < 4; i++) {
		PaintRect(&myRects[i]);
	}
	
	SetPort(oldPort);
	
	RGBForeColor(&fc);
	RGBBackColor(&bc);
}
/*....................................................................*/
void VCR::Draw2Rects2Colors(Rect r1, int color1, Rect r2, int color2)
{
	RGBColor fc,bc;
	
	GetForeColor(&fc);
	GetBackColor(&bc);
	
	SetColor(color1);
	PaintRect(&r1);
	SetColor(color2);
	PaintRect(&r2);
	
	RGBForeColor(&fc);
	RGBBackColor(&bc);
}
/*....................................................................*/
void VCR::ClearAllButtons()
{
	Buttons myButtons;
	int i;
	Boolean toDo = false;
	
	myButtons = (*this).myButtons;
	
		
	for(i = 1; i < LAST_BUTTON; i++) {
		if ( (toDo == false) && (myButtons.button[i] == true) ) {
			toDo = true;
		}
		myButtons.button[i] = false;
	}
	
	(*this).myButtons = myButtons;
	
	if (toDo) RedrawButtons();
}
/*....................................................................*/
void VCR::FlashButton(int id)
{
	int itemType;
	Handle itemHandle;
	Rect itemRect;
	CIconHandle	icon;
	int CICN_NUM;
	GrafPtr oldPort;
	DialogPtr myDlog = (*this).myDlog;
	RGBColor temp;
	
	GetPort(&oldPort);
	SetPort(myDlog);
	
	temp.red = temp.green = temp.blue = 0xffff;
	RGBBackColor(&temp);
	temp.red = temp.green = temp.blue = 0x0000;
	RGBForeColor(&temp);
	
	switch (id) {
		case VOLUME_UP:
			CICN_NUM = VOLUME_UP_CICN;
			break;
		case VOLUME_DOWN:
			CICN_NUM = VOLUME_DOWN_CICN;
			break;
		default: return; break;
	}
	GetDItem(myDlog,id,&itemType,&itemHandle,&itemRect);
	icon = GetCIcon(CICN_NUM);
	if (icon == NULL) {
		SysBeep(7);
		(*myAlert).AlertStop("\pFatal Error",
					"\pUnable to get cicn resource",
					"\pVCR::FlashButton",NULL);
	}
	
	HLock( icon );
	HLock( (**icon).iconData );
	(**icon).iconPMap.baseAddr = *(**icon).iconData;
	(**icon).iconMask.baseAddr = (Ptr)&(**icon).iconMaskData;
	
	CopyBits( &(**icon).iconPMap, &myDlog->portBits,
				&(**icon).iconPMap.bounds, &itemRect, notSrcCopy, 0 );

	SystemTask();
	(*myQT).MyMoviesTask();
	
	PlotCIcon(&itemRect, icon);
	HUnlock( (**icon).iconData );
	HUnlock(icon);
	DisposCIcon(icon);

	SetPort(oldPort);

}