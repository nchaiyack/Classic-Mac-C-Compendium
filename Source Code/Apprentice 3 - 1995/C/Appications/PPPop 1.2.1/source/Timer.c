/* PPPop, verison 1.2  June 6, 1995  Rob Friefeld

   Session Timer
*/


#include "Timer.h"

unsigned long	gCumulativeTime;
unsigned long	gSessionTime;
unsigned long	gStartTime;
unsigned long	gResetTime;
unsigned long	gTimeToReset;

Boolean			gCounting;
Boolean			gTimerOn;
Boolean			gShowCurrent;

OffscreenBitsP	gOffPtr;


void ResetCumulativeTime()
{
	gCumulativeTime = 0;
	GetDateTime(&gResetTime);		// record the time this was done
}


void ResetStartTime()
{
	gStartTime = TickCount();
	gSessionTime = 0;
}

void AdjustTimerWindow()
{
	if (gTimerOn) {
		AdjustWindowPosition(gTimerWindow);
		ShowWindow(gTimerWindow);
	}
	else
		HideWindow(gTimerWindow);
}

void InitTimer()
{
	Rect	bounds;
	
	if (gTimerWindow == NIL) return;
	bounds = gTimerWindow->portRect;
	gOffPtr = SetupOffBit(&bounds);
	ResetStartTime();
}

void SetupTimerWindow()
{
	short	fontNum;

	SetPort(gTimerWindow);
	GetFNum("\pMonaco", &fontNum);		// If this returns system font, fine
	TextFont(fontNum);
	if (fontNum != 0) {
		TextSize(9);
		TextFace(bold);
		SizeWindow(gTimerWindow, kTWindWidth, kTWindHeight - 3, FALSE);
	}
	// else we will automatically use Chicago 12 in a slightly larger window
}
	

void FlipTimeDisplay()
{
		gShowCurrent = !gShowCurrent;
		ShowTime();
}

void ElapsedTime()
{
	unsigned long	oldTime;
	
	if (gCounting) {
		oldTime = gSessionTime;
		gSessionTime = (TickCount() - gStartTime);
		gCumulativeTime += (gSessionTime - oldTime);
	}
	
	AutoReset();	// in case it is time to reset cumulative timer
}



void ShowTime()
{
	gShowCurrent ? 
	DisplayTime(gSessionTime) : DisplayTime(gCumulativeTime);
}


void DisplayTime(long refTime)
{
	char			theTime[] = "000:00:00";
	Str15			timeStr;
	long			timeCount;
	char			a;	
	GrafPtr			savePort;
	
	if (!gTimerOn) return;
	
	timeCount = (refTime/60) % 60;		// seconds
	NumToString(timeCount, timeStr);
	a = timeStr[0];						// this is the length byte of a pascal string
	if (a == 1)
		theTime[8] = timeStr[1];
	else {
		theTime[7] = timeStr[1];
		theTime[8] = timeStr[2];
	}
	
	timeCount = (refTime/3600) % 60;	// minutes
	NumToString(timeCount, timeStr);
	a = timeStr[0];
	if (a == 1)
		theTime[5] = timeStr[1];
	else if (a == 2) {
		theTime[4] = timeStr[1];
		theTime[5] = timeStr[2];
	}

	
	timeCount = refTime / 216000;		// hours
	NumToString(timeCount, timeStr);
	a = timeStr[0];		// length byte
	if (a == 1)
		theTime[2] = timeStr[1];
	else if (a == 2) {
		theTime[1] = timeStr[1];
		theTime[2] = timeStr[2];
	} 
	else if (a == 3) {
		theTime[0] = timeStr[1];
		theTime[1] = timeStr[2];
		theTime[2] = timeStr[3];
	}
	
	GetPort(&savePort);
	SetPort(gTimerWindow);	
	BeginOffBitDraw(gOffPtr);
	if (!gShowCurrent)
		TextBox(theTime, 6, &(gTimerWindow->portRect), teJustCenter);
	else
		TextBox(theTime + 1, 8, &(gTimerWindow->portRect), teJustCenter);
	EndOffBitDraw(gOffPtr, TRUE);
	SetPort(savePort);
}

void AutoReset()
{
	DateTimeRec		last, this;
	unsigned long	now;
	short			d, m, dm;
	
	if (gCurrentRadio != iNever) {
		Secs2Date(gResetTime, &last);		// load DTR for last time reset was done
		GetDateTime(&now);
		Secs2Date(now, &this);				// load DTR for current time
		
		if (gCurrentRadio == iDaily) {		// reset every day
			if (last.day != this.day) {		// it wasn't reset today, so do it
				ResetCumulativeTime();
				ShowTime();
			}
		}
		else if (gCurrentRadio == iMonthly) {	// reset every month on day n 

			d = gResetDay - this.day;
			dm = this.month - last.month;
			if (dm < 0)
				dm += 12;					// e.g. this = Jan, last = Dec

	// If we are past reset day, and reset done this month already, do nothing.
			if ((d < 0) && (dm == 0)) return;
			
	// If not yet reset day,  and reset done this month or last month, do nothing.
			if ((d > 0) && (dm <= 1)) return;
			
	// If we are on the reset day, and reset done this month already,
	// and that was done on the current reset day, do nothing.
			if ((d == 0) && (dm == 0) && (last.day == gResetDay)) return;
		
			ResetCumulativeTime();
			ShowTime();
		}
	}
}

	
	
