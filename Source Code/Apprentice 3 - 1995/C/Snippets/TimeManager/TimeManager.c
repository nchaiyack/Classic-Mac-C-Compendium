/* =============================================================

   Unfortunately, this Time Manager stuff isn't in a nice, neat
   library that you can add to your project for instant
   functionality.  To use it, you must copy all the functions,
   declarations, etc, into your own code.  (Or, you could turn
   it into your own library.)

   Read through the comments to figure out how it works.

   =============================================================*/

typedef struct {
  TMTask	atmTask;
  long		tmWakeUp;
  long		tmReserved;
  long		tmRefCon;
} TMInfo, *TMInfoPtr;

TMInfo	gTaskInfo;
long	gTime;        // gTime is time in milliseconds

pascal TMInfoPtr GetTMInfo(void) ONEWORDINLINE(0x2E89);

void InstallTask(void);
void RemoveTask(void);
pascal void SomeTask(void);

// before calling InstallTask, you must put an appropriate time in gTime.
void InstallTask(void)
{
  gTaskInfo.atmTask.tmAddr = NewTimerProc(SomeTask);
  gTaskInfo.tmWakeUp = 0;
  gTaskInfo.tmReserved = 0;
  gTaskInfo.tmRefCon = SetCurrentA5();    // so our task can access it's globals
  InsXTime((QElemPtr) &gTaskInfo);
  PrimeTime((QElemPtr) &gTaskInfo, gTime);
}

void RemoveTask(void)
{
  RmvTime((QElemPtr) &gTaskInfo);
}

pascal void SomeTask(void)
{
  TMInfoPtr			recPtr;
  long				oldA5;
  Boolean			reactivate;

// When looking at the following lines, remember that PowerMac code
// fragments have automatic access to their globals.
#ifndef powerc    // if we're not compiling for the PowerMac
  recPtr = GetTMInfo();
  oldA5 = SetA5(recPtr->tmRefCon);
#else             // if we are compiling for the PowerMac
  recPtr = &gTaskInfo;
#endif

  // do something that does not move memory.
  // Drawing pictures, playing sounds, etc. all move memory
  // If you want to do something that moves memory when SomeTask
  // is called, set a global flag and keep watch for it in your
  // event loop.

  PrimeTime((QElemPtr) recPtr, gTime);   // only if the task is repeating

#ifndef powerc     // if we're not compiling for the PowerMac
  SetA5(oldA5);
#endif
}



