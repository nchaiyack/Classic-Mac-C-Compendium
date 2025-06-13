#pragma once

#include "PPPop.h"
#include "Offscreen.h"
#include "TimerDlg.h"

#define	kTWindWidth		66
#define	kTWindHeight	14

extern unsigned long	gCumulativeTime;
extern unsigned long	gSessionTime;
extern unsigned long	gOldTime;
extern unsigned long	gStartTime;
extern unsigned long	gResetTime;
extern unsigned long	gTimeToRest;

extern Boolean			gCounting;
extern Boolean			gTimerOn;
extern Boolean			gShowCurrent;
extern OffscreenBitsP	gOffPtr;


void ResetCumulativeTime(void);
void ResetStartTime(void);
void ElapsedTime(void);
void ShowTime(void);
void AdjustTimerWindow(void);
void SetupTimerWindow(void);
void InitTimer(void);
void FlipTimeDisplay(void);
void DisplayTime(long refTime);
void AutoReset(void);

