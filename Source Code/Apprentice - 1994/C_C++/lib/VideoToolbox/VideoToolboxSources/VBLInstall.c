/*
VBLInstall.c

This is the Apple-recommended way of synchronizing your program to a video
display. In the Macintosh, interrupt service routines run at a high processor
priority, i.e. they block interrupts while they run, so Apple advises that they
be very quick, to avoid missing interrupts. That is the approach taken here,
using the interrupt service routines solely to bump a frame counter and set a
newFrame flag to true. Typically your main program will iteratively test the
newFrame flag and, when it's true, clear it and do some action that you want to
do once per frame, in synch with the video frames.

OSErr VBLInstall(VBLTaskAndA5 *vblData,GDHandle device,int frames);

The first argument is a pointer to a user-supplied data structure. "device" is a
handle to the video screen that you want to synchronize to, or NULL if you want
to synchronize to the System VBL rate (usually 60.15 Hz). The last argument,
"frames", specifies how many times you want the interrupt to occur before the
routine disables itself. If frames<0 then the interrupt will recur indefinitely.

VBLInstall sets noiseVBL.framesDesired and noiseVBL.framesLeft equal to
"frames". While noiseVBL.framesLeft!=0, at each end-of-frame interrupt
noiseVBL.newFrame will be set to 1. noiseVBL.framesLeft will be decremented if
it's >0, but left alone if it's negative. If noiseVBL.framesLeft is decremented
to zero then the interrupt service routine is done, and won't reenable the
interrupt.

Here's a minimal program, extracted from NoiseVBL.c, that uses these routines to
show a 100-frame movie, with a new image on each video frame:

	VBLTaskAndA5 noiseVBL;
	GDHandle device=GetMainDevice();
	int frames=100,error;

	noiseVBL.subroutine=NULL;				// request default subroutine
	error=VBLInstall(&noiseVBL,device,frames);
	if(error)PrintfExit("VBLInstall error %d\n",error);
	noiseVBL.vbl.vblCount=1;	// enable the interrupt service routine
	while(noiseVBL.framesLeft){
		if(noiseVBL.newFrame){
			noiseVBL.newFrame=0;
			CopyBitsQuickly((BitMap *)&(movie[noiseVBL.framesLeft])
				,(BitMap *)*((CGrafPtr)window)->portPixMap
				,&noiseImage[0].bounds,&window->portRect,srcCopy,NULL);
		}
	}
	VBLRemove(&noiseVBL);

Apple warns that interrupt service routines and any data that they access must
be locked into memory, not swapped out, e.g. due to operation of the Memory
Manager or Virtual Memory. It is dangerous to allow your compiler to install
debugging code into interrupt service routines. It is VERY important that you
call VBLRemove() before quitting. Once installed, slot-based interrupt tasks
keep going forever. (Turning off vblData->vbl.vblCount disables the routine, but
doesn't remove it.) The tasks are not removed by the Finder or System when your
application finishes, even though the interrupt service routine's code and data
will probably be overwritten.

Actually, things aren't that bad, because I've added a call to _atexit()
requesting that all the VBL tasks installed by VBLInstall() be removed whenever
the program terminates, whether normally or abnormally. This prevents the
otherwise very annoying crash that would accompany premature termination, e.g.
by typing command-., while the VBL task is active.

Writing an interrupt service routine, to be called once per video frame, is
slightly tricky. VBLInterruptServiceRoutine does all the dirty work, and then
calls a user-supplied subroutine that does whatever you want. If all you want to
do is advance a frame counter until you reach the desired number of frames then
you may wish to use the default FrameSubroutine() instead of writing your own.
Put the address of whatever routine you want to use into the "subroutine"
element of the VBLTaskAndA5 structure, or, to request use of FrameSubroutine,
supply the address NULL.

FrameSubroutine() uses Timer.c. If this program runs on an old System (pre
6.05?) lacking the Revised Time Manager, which Timer.c requires, then
SimpleVBLSubroutine() will be used instead. The Timer is used to discard
spurious interrupts that occur within 5 ms of the previous. This is necessary
because some of the Apple video cards generate several interrupts during the
vertical blanking interval, even though Apple's documentation clearly indicates
that they should only generate one. The fix (holding off for 5 ms) was suggested
by Raynald Comtois.

The 1992 Apple "Inside Macintosh: Processes" book explains how to code a task
that is to be performed each time your video device produces a vertical blanking
level (i.e. between video frames). It's quite tricky. Therefore I wrote a
generic one, that in turn, calls your custom one, after all the tricky bits have
been taken care of.

There are some subtleties to the VBL interrupt service routine. The main one is
that all the Macintosh compilers reference global variables relative to register
A5, but register A5 may have the wrong value (corresponding to a different
application) at interrupt time. So we save the right value of A5 inside our
structure and restore A5 before calling our Task().

A further subtlety is that the new generation of optimizing compilers might
notice that A5 is being modified, so it would be dangerous to access globals at
all in InterruptServiceRoutine(), even though it's safe to do so in Task(). In
fact Task() doesn't use any globals, but it could.

The fact that the address of our structure is in register A0 when the interrupt
service routine is called results from the fact that the low level hardware VBL
interrupt is intercepted by the operating system, which then calls our routine.

The VBLTaskAndA5 struct extends Apple's VBLTask struct by adding some useful
information at the end. You may choose to copy this, or define your own, adding
more stuff at the end. However, you may not want to bother, considering that
your interrupt service routine can freely access global variables.
Alternatively, I added a generic pointer at the end, which you may use to pass
the address of any stuff that you want to access within your subroutine.

Macintosh Technical Note 180 ("Multifinder Miscellanea"), p. 5 says:
"GetVBLRec() returns the address of the VBLRec associated with our VBL task.
This works because on entry into the VBL task, A0 points to the theVBLTask field
in the VBLRec record, which is the first field in the record and that is the
address we return. Note that this method works whether the VBLRec is allocated
globally, in the heap...or...on the stack. ... This trick allows us to get to
the saved A5, but it could also be used to get to anything we wanted to store in
the record." (quoted by Jamie McCarthy in comp.sys.mac.prog 10/15/92.)

HISTORY:
8/22/92 dgp wrote it, based on code extracted from my NoiseVBL.c
8/26/92	dgp	added VBLRemoveAll(), which is automatically placed in the _atexit()
			queue, so it all your VBL tasks will be removed from the queue when
			your program exits, whether normal or abnormally.
9/9/92	dgp	fixed erroneous attempt to use slot zero when NULL device was passed.
9/10/92	dgp	added calls to VM to HoldMemory() and UnHoldMemory(). According to Apple's
			Memory book this isn't strictly necessary, since VBL tasks will 
			be called only when it's safe.
9/17/92	dgp	Transferred FrameSubroutine() here from GDFrameRate.c. Now use 
			FrameSubroutine() instead of SimpleVBLSubroutine() as the default,
			provided we can use Timer.c, otherwise fall back to using 
			SimpleVBLSubroutine.
10/9/92	dgp	Automatically set up and dispose of the timer used by FrameSubroutine.
			Added a field to the VBLTaskAndA5 structure to hold the timer pointer,
			instead of using up the generic ptr.
			WARNING: old programs (written during 9/92) that explicitly request use of 
			FrameSubroutine must be changed, because at that time it was the user's 
			responsibility to set up and dispose of the timer, whereas it's now done 
			for you. To remind you to make this change "FrameSubroutine" is no longer 
			in VideoToolbox.h, it's treated as a private routine here. To obtain the 
			services of FrameSubroutine, just specify a NULL in the VBLTaskAndA5 
			subroutine field.
11/17/92 dgp In VBLRemove(), first disable the interrupt, then clean up.
			Fixed error that could cause bus error in VBLRemoveAll.
			VBLInstall() now installs VBLRemoveAll() only once in the _atexit()
			queue, no matter how many times you call it.
11/24/92 dgp Minor updating of comments.
7/9/93	dgp	Test MATLAB in if() instead of #if. 
2/28/94	dgp	In response to query by Mike Tarr (tarr-michael@CS.YALE.EDU), changed 
			frames argument from int to long, and now allow frames==-1 to request 
			that the interrupt service routine continue working indefinitely.
3/5/94	dgp	In response to a bug report by Mike Tarr, finished the 2/28/94 change,
			which I'd foolishly only done to SimpleVBLSubroutine and not to 
			FrameSubroutine.
*/
#include "VideoToolbox.h"
#include <Traps.h>
void VBLRemoveAll(void);
static void FrameSubroutine(VBLTaskAndA5 *vblData);

/* This is just for reference. The original is in VideoToolbox.h
struct VBLTaskAndA5 {
    volatile VBLTask vbl;
    long ourA5;
    void (*subroutine)(struct VBLTaskAndA5 *vblData);
    GDHandle device;
    long slot;
    volatile long newFrame;				// Boolean
    volatile long framesLeft;			// count down to zero
    long framesDesired;
    Timer *frameTimer;					// time ms since last VBL interrupt, see Timer.c
    void *ptr;							// use this for whatever you want
};
typedef struct VBLTaskAndA5 VBLTaskAndA5;
*/
#define TASK_SIZE 1000	// Generous guess for size of routine
static long vmPresent=0;

OSErr VBLInstall(VBLTaskAndA5 *vblData,GDHandle device,long frames)
// trivial, but verbose
{
	static int firstTime=1,slotRoutinesAvailable;
	static long timeManagerVersion=0;
	
	if(firstTime){
		firstTime=0;
		slotRoutinesAvailable=TrapAvailable(_SlotVInstall);
		Gestalt(gestaltVMAttr,&vmPresent);
		vmPresent &= gestaltVMPresent;
		if(!MATLAB)_atexit(VBLRemoveAll);
		Gestalt(gestaltTimeMgrVersion,&timeManagerVersion);
	}
	vblData->device=device;
	if(device!=NULL && slotRoutinesAvailable)vblData->slot=GetDeviceSlot(device);
	else vblData->slot=-1;
	vblData->vbl.vblAddr=(ProcPtr)VBLInterruptServiceRoutine;
	vblData->vbl.qType=vType;
	vblData->vbl.vblCount=0;	/* Initially disable the interrupt service routine */
	vblData->vbl.vblPhase=0;
	vblData->ourA5=SetCurrentA5();
	if(vblData->subroutine==NULL){
		if(timeManagerVersion>=gestaltRevisedTimeMgr){
			vblData->frameTimer=NewTimer();
			StartTimer(vblData->frameTimer);
			vblData->subroutine=FrameSubroutine;
		}
		else vblData->subroutine=SimpleVBLSubroutine;
	}
	vblData->newFrame=0;
	vblData->framesLeft=vblData->framesDesired=frames;
	if(vmPresent){
		HoldMemory(vblData,sizeof(*vblData));
		HoldMemory(vblData->vbl.vblAddr,TASK_SIZE);
		HoldMemory(vblData->subroutine,TASK_SIZE);
	}
	if(vblData->slot>=0)return SlotVInstall((QElemPtr)vblData,vblData->slot);
	else return VInstall((QElemPtr)vblData);
}

OSErr VBLRemove(VBLTaskAndA5 *vblData)
{
    OSErr error;

	// only remove it if we installed it
	if(vblData->vbl.vblAddr != (ProcPtr)VBLInterruptServiceRoutine)return;
	if(vblData->slot>=0)error=SlotVRemove((QElemPtr)vblData,vblData->slot);
	else error=VRemove((QElemPtr)vblData);
	if(vmPresent){
		UnholdMemory(vblData,sizeof(vblData));
		UnholdMemory(vblData->vbl.vblAddr,TASK_SIZE);
		UnholdMemory(vblData->subroutine,TASK_SIZE);
	}
	if(vblData->subroutine==FrameSubroutine && vblData->frameTimer!=NULL)
		DisposeTimer(vblData->frameTimer);
	return error;
}

void VBLRemoveAll(void)
{
	QHdrPtr qHeader;
	QElemPtr q;
	
	qHeader=GetVBLQHdr();
	q=qHeader->qHead;
	while(q!=NULL){
		VBLRemove((VBLTaskAndA5 *)q);	// only removes ours
		q=q->qLink;
	}
}	

#pragma options(!profile)	// it would be dangerous to call the profiler from here
#pragma options(assign_registers,redundant_loads)
Ptr GetA0(void)=0x2008;	/* MOVE.L A0,D0 */

void VBLInterruptServiceRoutine(void)
{
	register long oldA5;
	register VBLTaskAndA5 *vblData;

	vblData = (VBLTaskAndA5 *)GetA0();
	oldA5 = SetA5(vblData->ourA5);
	(*vblData->subroutine)(vblData);		// call user's task, pass data ptr
	SetA5(oldA5);
}

void SimpleVBLSubroutine(VBLTaskAndA5 *vblData)
{
	vblData->newFrame=1;
	if(vblData->framesLeft>0)vblData->framesLeft--;
	if(vblData->framesLeft!=0)vblData->vbl.vblCount=1;
}

static void FrameSubroutine(VBLTaskAndA5 *vblData)
{
	// The 1991-2 Apple video cards emit several vbl interrupts per frame,
	// which violates Apple's documentation.
	// So we use the Time Manager to ignore any interrupt that occurs within 5 ms
	// of the most recent interrupt, for that video device.
	// Thus this frame counter should work correctly on all video cards.
	// Suggested by Raynald Comtois.
	if(StopTimer(vblData->frameTimer)>5000){
		vblData->newFrame=1;									// set new-frame flag
		if(vblData->framesLeft>0)vblData->framesLeft--;
		if(vblData->framesLeft!=0)vblData->vbl.vblCount=1;		// re-enable interrupt
	}else vblData->vbl.vblCount=1;								// re-enable interrupt
	StartTimer(vblData->frameTimer);
}

