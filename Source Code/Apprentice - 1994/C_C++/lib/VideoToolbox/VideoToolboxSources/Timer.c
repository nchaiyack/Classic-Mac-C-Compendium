/*
Timer.c
An interval timer based on Apple's Time Manager. It returns the time that
elapsed between calling StartTimer() and StopTimer(). Under System 7, which has
the Extended Time Manager, the timing is very accurate (better than 1 part in
1000) and a precision of better than 200 �s. Under System 6 the precision will
be about 1 ms, and the returned times will tend to be about 10% low due to
documented deficiencies of Apple's old Revised Time Manager. The even older
"Standard" Time Manager is not supported here, and will result in an error message.

One could easily add a PeekTimer() routine to this set, but I haven't
gotten around to it. Send it to me if you write it.

Timer *NewTimer(void);
void DisposeTimer(Timer *t);
void StartTimer(Timer *t);
long StopTimer(Timer *t);					// �s, up to 36 minutes
double StopTimerSecs(Timer *t);				// s, �s precision and no time limit

	Timer *timer;
	long t;
	
	timer=NewTimer();
	StartTimer(timer);
	do(i=0;i<100;i++);
	t=StopTimer(timer);
	DisposeTimer(timer);
	printf("One hundred iterations takes %ld �s\n",t);

The timing result comes in two flavors. StopTimer() returns the time in
microseconds as a long, which can hold a time up to 2,147,483,647 �s, which is
nearly 36 minutes. StopTimerSecs() returns the time in secs as a double, and can
time an interval of essentially unlimited duration with microsecond precision.

You can have many Timers running at once. The only restriction is that you must
create each Timer, by calling NewTimer() before you use it, and, obviously,
should not use it after calling DisposeTimer().

The Time Manager seems to use interrupts at a higher rate than the 10 s interval
I requested here (which would never expire in typical use). I infer this from
the fact that disabling interrupts by SetPriority(7) greatly reduces the timing
value returned by StopTimer(). So don't disable interrupts while you're timing.

StopTimer() adds a small offset (about 62 �s on a Mac IIci) to the raw time in
order to return an unbiased estimate of the time from when StartTime was called
to when StopTime() was called. This is the most useful time measure for
measuring the interval between two events (e.g. video frames). Alternatively, if
you wish to measure processing time then it is more useful to compute the time
from when StartTimer() returns to when StopTimer() is called. For this you
simply subtract the fixed duration of StartTimer(), call-to-return, which is
provided in �s in your Timer structure in the long structure member
"timeToStartTimer".

	betweenTime = StopTimer(t) - t->timeToStartTimer;
	
If you use StopTimerSecs() instead you'll have to divide timeToStartTimer by a
million to convert �s to s. For some purposes this offset is negligibly
small. On my Mac IIci the timeToStartTimer is 240 �s. This offset, and the one
mentioned above are measured automatically the first time you call NewTimer().
Alternatively, instead of having to remember the name of the structure member,
just measure the timeToStartTimer yourself and subtract it from all subsequent
measures:

	StartTimer(t);
	s0=StopTimerSec(t);

The Timer structures are kept in a linked list so that KillEveryTimer(), which
is placed in the _atexit() queue, can find and kill them when the application
exits.

HISTORY:
8/19/92 dgp	based on Time Manager chapter in Inside Mac VI and TimeIt.c, 
	which is now obsolete. I also benefited from examining code by Jonothan Kolodny
	forwarded to me by Thomas Busey.
8/27/92	dgp	Rewrote everything. Made the interrupt service routine reentrant 
	by eliminating all use of global variables, using only the structure pointed
	to by A1. There can now be an unlimited number of timers active at once.
	Added NewTimer() and DisposeTimer() to manage them. 
9/10/92	dgp	added calls to VM to HoldMemory() and UnHoldMemory(). According to Apple's
	Memory book this isn't strictly necessary, since Time Manager tasks will be
	called only when it's safe.
1/11/93 dgp StopTimerSecs() now returns NAN if called with a NULL pointer.
7/9/93	dgp	Test MATLAB in if() instead of #if. 
*/
#include "VideoToolbox.h"
static pascal void TimerTask(void);
void KillEveryTimer(void);
Ptr GetA1(void);

/* This is a copy for reference. Original is in VideoToolbox.h.
struct Timer{
	TMTask time;
	long ourA5;
	long interval,elapsed,elapsedIntervals;
	long timeToStartTimer;			// minimum time in �s
	long stopDelay;					// �s from call to stop, re from call to start
	long timeManagerVersion;
	struct Timer *next,*previous;	// doubly linked list of Timers
};
*/

static Timer defaultTimer,qTimer={0,0,0,0,0,0,0,0,0,0,0};
static long vmPresent=0;
#define TASK_SIZE 1000	// Generous guess for size of routine

Timer *NewTimer(void)
{
	static short firstTime=1;
	extern Timer defaultTimer,qTimer;
	Timer *t,*tt;
	long j;
	
	if(firstTime){
		firstTime=0;
		qTimer.next=qTimer.previous=NULL;
		if(!MATLAB)_atexit(KillEveryTimer);
		Gestalt(gestaltVMAttr,&vmPresent);
		vmPresent &= gestaltVMPresent;
		t=&defaultTimer;
		t->ourA5 = SetCurrentA5();
		t->time.tmAddr = (TimerProcPtr)TimerTask;
		t->time.tmCount=t->time.tmWakeUp=t->time.tmReserved=0;
		t->elapsedIntervals=t->elapsed=0;							
		t->timeManagerVersion=0;
		Gestalt(gestaltTimeMgrVersion,&t->timeManagerVersion);
		switch(t->timeManagerVersion){
		case 0:
		case gestaltStandardTimeMgr:
			printf("NewTimer: old System lacks the Revised Time Manager.\n");
			return NULL;
		case gestaltRevisedTimeMgr:
			t->interval = 1L;				// Set the timer interval to 1 ms
			break;
		case gestaltExtendedTimeMgr:
		default:
			t->interval = -10000000L;		// Set the timer interval to 10 s
		}
		t->next=NULL;
		t->previous=&qTimer;
		t->timeToStartTimer=t->stopDelay=0;
		
		// Measure timeToStartTimer and stopDelay offsets.
		t=NewTimer();
		StartTimer(t);
		t->timeToStartTimer=StopTimer(t);
		tt=NewTimer();
		StartTimer(t);
		StartTimer(tt);
		j=StopTimer(t);
		defaultTimer.stopDelay=t->stopDelay=2*t->timeToStartTimer-j;
		// The computed "cycle" interval will have stopDelay removed.
		// The user wishing to compute the "between" interval will be
		// subtracting the timeToStartTimer, so we should subtract 
		// the stopDelay from that
		// so the stopDelay cancels out when "between" time is computed.
		defaultTimer.timeToStartTimer=t->timeToStartTimer-=t->stopDelay;
		DisposeTimer(tt);
		return t;
	}
	t=(Timer *)NewPtr(sizeof(Timer));
	if(t!=NULL){
		*t=defaultTimer;
		t->next=qTimer.next;
		if(t->next!=NULL)t->next->previous=t;
		qTimer.next=t;
		if(vmPresent){
			HoldMemory(t,sizeof(*t));
			HoldMemory(t->time.tmAddr,TASK_SIZE);
		}
		if(t->timeManagerVersion==gestaltRevisedTimeMgr)InsTime((QElemPtr)t);
		else InsXTime((QElemPtr)t);
	}
	return t;
}

void DisposeTimer(Timer *t)
{
	Timer *tt;
	
	if(t==NULL)return;
	RmvTime((QElemPtr)t);
	if(vmPresent){
		UnholdMemory(t,sizeof(*t));
		UnholdMemory(t->time.tmAddr,TASK_SIZE);
	}
	t->previous->next=t->next;
	if(t->next!=NULL)t->next->previous=t->previous;
	DisposPtr((Ptr)t);
}

void StartTimer(Timer *t)
{
	if(t==NULL)return;
	PrimeTime((QElemPtr)t,t->interval);
}

long StopTimer(Timer *t)							// Returns �s
{
	register long microSeconds;
	extern Timer defaultTimer;

	if(t==NULL)return 0;
	RmvTime((QElemPtr)t);
	// add up the elapsed intervals plus the one we're in, minus the time left
	microSeconds=t->elapsed + t->interval;
	if(microSeconds>0) microSeconds*=1000;			// convert ms to �s
	else microSeconds=-microSeconds;
	if(t->time.tmCount>0)t->time.tmCount*=-1000;	// convert ms to -�s
	microSeconds+=t->time.tmCount;					// -�s until end of interval
	microSeconds-=t->stopDelay;						// compute "cycle" time
	
	// Reinstall the Timer, to be ready for for another call to StartTimer()
	t->time.tmWakeUp=t->time.tmReserved=t->elapsed=t->elapsedIntervals=0;							
	if(t->timeManagerVersion==gestaltRevisedTimeMgr)InsTime((QElemPtr)t);
	else InsXTime((QElemPtr)t);
	
	return microSeconds;
}

double StopTimerSecs(Timer *t)						// Returns secs
{
	double s;
	extern Timer defaultTimer;

	if(t==NULL)return NAN;
	RmvTime((QElemPtr)t);
	// add up the elapsed intervals plus the one we're in, minus the time left
	t->elapsedIntervals++;
	if(t->interval>0) s=t->elapsedIntervals*1000.0*t->interval;		// ms
	else s=(double)-t->elapsedIntervals*t->interval;				// �s
	if(t->time.tmCount>0) s-=t->time.tmCount*1000.0;	// -ms until end of interval
	else s+=t->time.tmCount;							// -�s until end of interval
	s-=t->stopDelay;									// compute "cycle" time
	s*=0.000001;
	
	// Reinstall the Timer, to be ready for for another call to StartTimer()
	t->time.tmWakeUp=t->time.tmReserved=t->elapsed=t->elapsedIntervals=0;							
	if(t->timeManagerVersion==gestaltRevisedTimeMgr)InsTime((QElemPtr)t);
	else InsXTime((QElemPtr)t);
	
	return s;
}

// KillEveryTimer turns off all our Timers before we quit. There is no need to
// free the space since the system will do that automatically.

void KillEveryTimer(void)
{
	Timer *t;
	extern Timer qTimer;

	t=qTimer.next;
	while(t!=NULL){
		RmvTime((QElemPtr)t);
		if(vmPresent){
			UnholdMemory(t,sizeof(*t));
			UnholdMemory(t->time.tmAddr,TASK_SIZE);
		}
		t=t->next;
	}
}

// The Revised & Extended Time managers set A1=&task.time before calling TimerTask
// The code allowing access to globals is commented out because it is not needed here.

#pragma options(!profile)	// it would be dangerous to call the profiler from here
Ptr GetA1(void)=0x2009;		// MOVE.L A1,D0

static pascal void TimerTask(void)				// Called at interrupt time
{
	long oldA5;
	Timer *t;

	t=(Timer *)GetA1();
//	oldA5 = SetA5(t->ourA5);					// Reestablish A5 for global variables
	PrimeTime((QElemPtr)t,t->interval);			// Repeat the interval
	t->elapsed += t->interval;					// Increment the time count
	t->elapsedIntervals++;
//	SetA5(oldA5);	 							// Restore A5
}
