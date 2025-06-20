// simple time manager deadman task by Chuck Pliske {chuckp@u.washington.edu}
// this code is creditware.  If you sell it, give me some credit.

#include <Timer.h>
#include "AppleEvents.h"
#include "GestaltEqu.h"


#define TMTIME 1000 // 1000 ms execution frequency
// note: define counts in number of seconds to wait before abort (reboot)
// mainline will try once each second to reset counter.
#define COUNTS 300 // amount of time to wait before reset (300=5min)
#define kSleepMax 60  // long sleep time to avoid stealing cycles

long temp=COUNTS;
ProcPtr ResetVector;

typedef struct {
	long myA5;
	TMTask tmTask;
} myStuff;

myStuff x;

Boolean gAppleEventsFlag, gQuitFlag;
long gSleepVal;

void TM_Routine ()
{
   asm{	move.l a5,-(sp)
   		move.l -4(a1),a5 } // the globals, please (from a1 for time manager)
   		
        //x.tmTask.tmCount = 30;  /* reset the count so we run next time */
        PrimeTime(&x.tmTask,TMTIME); // run every 100 ms or so...

		if(temp-- <= 0) { asm { movea.l  ResetVector,a3
								jmp (a3) } }
		
        asm{ move.l (sp)+,a5 }         /* reset the globals stuff */
        
}

/* This routine installs the given TMTask */

Set_Interrupts (TMTask *tmTask)
{
        OSErr err;

        tmTask->qType          = vType;
        tmTask->tmAddr        = (ProcPtr) TM_Routine;
        tmTask->tmWakeUp       = 0;
        tmTask->tmReserved       = 0;
        InsTime ( (QElemPtr)tmTask );
        PrimeTime(tmTask,TMTIME);
}

/*
main()
{
	x.myA5 = (long)CurrentA5;
	Set_Interrupts(&x.tmTask);
	Debugger();
	temp = 0;
	while (!Button());
}
*/
  
pascal OSErr DoAEOpenApplication(AppleEvent * theAppleEvent,
                                 AppleEvent * replyAppleEvent, 
                                 long refCon)
{
#pragma unused (theAppleEvent, replyAppleEvent, refCon)
  return noErr;
}
 
pascal OSErr DoAEOpenDocuments(AppleEvent * theAppleEvent,
                               AppleEvent * replyAppleEvent, 
                               long refCon)
{
#pragma unused (theAppleEvent, replyAppleEvent, refCon)
  return errAEEventNotHandled;
}
 
pascal OSErr DoAEPrintDocuments(AppleEvent * theAppleEvent,
                                AppleEvent * replyAppleEvent, 
                                long refCon)
{
#pragma unused (theAppleEvent, replyAppleEvent, refCon)
  return errAEEventNotHandled;
}
 
pascal OSErr DoAEQuitApplication(AppleEvent * theAppleEvent,
                                 AppleEvent * replyAppleEvent, 
                                 long refCon)
{
#pragma unused (theAppleEvent, replyAppleEvent, refCon)
  gQuitFlag = true;
  return noErr;
}
 
void InitAppleEventsStuff(void)
// install Apple event handlers
{
  OSErr retCode;
  
  if (gAppleEventsFlag) {
    
    retCode = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication,
                (EventHandlerProcPtr) DoAEOpenApplication, 0, false);
    if (retCode == noErr)
      retCode = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
              (EventHandlerProcPtr) DoAEOpenDocuments, 0, false);
    if (retCode == noErr)
      retCode = AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,
              (EventHandlerProcPtr) DoAEPrintDocuments, 0, false);
    if (retCode == noErr)
      retCode = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
              (EventHandlerProcPtr) DoAEQuitApplication, 0, false);
    if (retCode != noErr) DebugStr("\pInstall event handler failed");
  }
}
 
void DoHighLevelEvent(EventRecord * theEventRecPtr)
// high-level event dispatching
{
  (void) AEProcessAppleEvent(theEventRecPtr);
}
 
main()
{
  OSErr retCode;
  long gestResponse;
  long tickWait;
  
  EventRecord mainEventRec;
  Boolean eventFlag;
  
  // initialize globals
  
  gQuitFlag = false;
  gSleepVal = kSleepMax;
  
  // wait about a minute for things to settle down...
  tickWait = TickCount();
  while ((TickCount() - tickWait) < 3600) WaitNextEvent(everyEvent, &mainEventRec, gSleepVal,nil);
  
  // is the Apple Event Manager available?
  retCode = Gestalt(gestaltAppleEventsAttr, &gestResponse);
  if (retCode == noErr &&
      (gestResponse & (1 << gestaltAppleEventsPresent)) != 0)
    gAppleEventsFlag = true;
  else gAppleEventsFlag = false;
 
  // install Apple event handlers
  InitAppleEventsStuff();
  
  // install tm task
  x.myA5 = (long)CurrentA5;
  ResetVector = ROMBase + 10;

  Set_Interrupts(&x.tmTask);
    //Debugger();

  while (!gQuitFlag) {
    eventFlag = WaitNextEvent(everyEvent, &mainEventRec, gSleepVal, nil);
    
    temp = COUNTS; // reset the deadman

    if (mainEventRec.what == kHighLevelEvent)
      DoHighLevelEvent(&mainEventRec);
  }
  RmvTime((QElemPtr)&x.tmTask);
}

