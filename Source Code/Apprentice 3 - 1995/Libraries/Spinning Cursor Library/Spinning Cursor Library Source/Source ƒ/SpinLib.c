/*
	SpinLib.c
	
	Code for spinning a beachball cursor.
	
	History:
		06/01/95 - dn - Modified to use the Universal Headers.
*/

#include "SpinLibInterns.h"

#include <Traps.h>
#include <GestaltEqu.h>
#include <Think.h>

/*
	To build a shared library from this project, you must do two things:
	
	1. Add the interfacelib.xcoff file to the project
	2. move the next line out of this comment:
		
		#pragma lib_export list SpinInit, SpinStart, SpinStop, SpinSpinning, SpinCleanup
*/

#if USESROUTINEDESCRIPTORS
#define UniversalHdrs
#else
#define NotUniversalHdrs
#endif

#ifdef NotUniversalHdrs
#ifndef CrsrBusy
#define CrsrBusy  0x8CD				/*[GLOBAL VAR]  Cursor locked out? [byte]*/
#endif
#endif /* NotUniversalHdrs */

#define kBaseSpinCurs 29000

#ifdef NotUniversalHdrs
pascal long __GetVBLRec (void);
pascal long __GetVBLRec (void)= 0x2E88;
#endif /* NotUniversalHdrs */

CursorTask			gCursTask;						/* Global structure */
Boolean				gCursInstalled = false;				/* installation global */
Boolean				gCursInited=false;

/*
	SpinInit
	
	Loads the cursors and sets up the spinning stuff...
*/
OSErr SpinInit(void){
	Handle h;
	short i;
	OSErr err;
	
	if (gCursInited)
		return noErr;
		
	gCursInstalled = false;							/* we are not installed yet */
	gCursInited=false;
	
	/* allocate cursor memory */
	gCursTask.cursors=(CursHandle *)NewPtr (8*sizeof (CursHandle));
	
	gCursTask.value=0;
	
	for (i=0; i < 8; i++) {							/* cycle through the rest */
			
		h = (Handle)GetCursor (kBaseSpinCurs+i);			/* black and white */
			
		if (h == nil)								/* did we get the cursor */
			return ResError();						/* if no, return error */
		
		DetachResource(h);
		HNoPurge (h);								/* force it unpurgeable */
		HLockHi (h);								/* and lock it high */
		
		gCursTask.cursors[i]=(CursHandle)h;
	}
	
	gCursInited=true;
	
	return noErr;
}

/*
	SpinStart
	
	Starts the spinning of the beachball.
*/
OSErr SpinStart(short direction){
	OSErr		err;									/* returnable error */
	short		i;									/* dummy counter */
	Handle		h;
	
	if (!gCursInited)
		return paramErr;
		
	if (gCursInstalled)									/* if we are already installed, */
		return noErr; // do nothing...
			
	if (direction==0)
		direction=1;
	
	if (direction>1)
		direction=1;
	if (direction<-1)
		direction=-1;
	
	gCursTask.direction=direction;
	
	gCursTask.vblTask.qType = vType;						/* set up the VBL task record */
	
#ifdef NotUniversalHdrs
	gCursTask.vblTask.vblAddr=(ProcPtr)SpinTask;
#else
	{
		VBLUPP spinUPP;
		spinUPP=NewVBLProc(SpinTask); // get the Universal Proc Ptr for a VBL task
		
		gCursTask.vblTask.vblAddr = spinUPP;
	}
#endif /* NotUniversalHdrs */
	
	gCursTask.vblTask.vblCount = 20;
	gCursTask.vblTask.vblPhase = 0;
	gCursTask.vblA5 = SetCurrentA5();
	gCursTask.vblKeyTag=(long)('NebS');
	
	err = VInstall ((QElemPtr)&gCursTask.vblTask);			/* install the VBL task */
		
	if (err)
		return err;									/* if errors, return them, else */
	else {
		gCursInstalled = true;							/* declare the task installed */
		return noErr;									/* and return no error */
	}
}

/*
	SpinTask
	
	This is the meat and bones of the spinning cursor calls.  When the VBL task count (vblCount) goes to zero, the
	Vertical Retrace Manager will call SpinCursor(). Our job will be to set the cursor to the current frame, and then
	to advance the frame. The thing to remember is that we are executed during interrupt, and we are not allowed to
	use Memory Manager calls... as well as a billion other things.
	
	Another cool thing to note is that at the time of our interrupt, a pointer to the installed VBL task will be
	placed in A0. Since we installed with some more globals attached, these will also be available to us; kind of
	a sneaky way around the messed up A5 problem.
	
	Also remember that the vblCount is zero at this time. If we leave this function without returning a nonzero value,
	the task will never return to SpinCursor again. So, we articulately return the vblCount to frequency, which is
	why the task happens so regulary...
*/
pascal void SpinTask(void){
	CursorTaskPtr		taskPtr;
	Boolean			busy;
	CursPtr			cp;
	
#ifdef NotUniversalHdrs
	taskPtr=(CursorTaskPtr)__GetVBLRec();
	/*	
		This is how we used to get the VBL record.
	*/
#else
	short			doNext=1;
	QElemPtr			qep;
	QHdrPtr			qh;
	
	/*
		This is the Universal Header way to get to the queue element...  The queue
		must be searched from the head to the end looking for our queue element.  If the
		element can't be found then taskPtr is set to NULL.
	*/
	
	qh=GetVBLQHdr();	// get the head of the queue...
	qep=qh->qHead;		// get the first element in the queue...
	
	do {
		taskPtr=(CursorTaskPtr)qep; // make our ptr out of the elem ptr...
		
		if ((taskPtr->vblKeyTag)==((long)'NebS')){
			// this is our item...
			doNext=0;
		} else {
			// this is not our item...
			if (qep->qLink!=(QElemPtr)NULL){
				// but not the end of the list...
				qep=qep->qLink;
			} else {
				// this is the end of the list
				// make each of the pointers point to null
				qep=(QElemPtr)NULL;
				taskPtr=(CursorTaskPtr)NULL;
				
				// and flag the end of the loop...
				doNext=0;
			}
		}
	} while (doNext);
	
	if (taskPtr==(CursorTaskPtr)NULL)
		return;	// unable to continue since we don't have a pointer to our task (oops)!
#endif /* NotUniversalHdrs */

#ifdef NotUniversalHdrs
	// this is the old way...
	busy = (*(Boolean *)CrsrBusy);						/* determine the cursor state */
#else
	// this is the universal headers way...
	busy= (Boolean)LMGetCrsrBusy();						/* determine the cursor state */
#endif /* NotUniversalHdrs */
	
	if (!busy) {										/* if it is available to us */
													/* set the cursor and frame advance */
		cp=(CursPtr)(*(taskPtr->cursors[taskPtr->value]));
		
		SetCursor(cp);
		
		taskPtr->value += taskPtr->direction;
			
		if (taskPtr->value>7)
			taskPtr->value=0;
		
		if (taskPtr->value<0)
			taskPtr->value=7;
	}

	taskPtr->vblTask.vblCount = 3;		/* restore the VBL task */
}

/*
	SpinStop
	
	Stops the spinning of the cursor.
*/
OSErr SpinStop(void){
	OSErr			err;
	short			i;
	CursPtr			arrowPtr;

	if (!gCursInstalled)									/* if we were not installed, */
		return noErr;									/* alert them and exit */
	
	err = VRemove ((QElemPtr)&gCursTask.vblTask);			/* remove the VBL task */
	gCursInstalled=false;
	
	
#ifdef NotUniversalHdrs
	arrowPtr=&arrow;
#else
	arrowPtr=&(qd.arrow);	
#endif
	
	SetCursor (arrowPtr);								/* restore the cursor to an arrow */
	
	return err;
}

/*
	SpinSpinning
	
	returns state of the cursor.
*/
Boolean SpinSpinning(void){
	return gCursInstalled;
}

/*
	SpinCleanup
	
	Dispose of the memory being used by the spinning cursors...
*/
OSErr SpinCleanup(void){
	short i;
	OSErr err;
	
	if (gCursInstalled)
		SpinStop();
	
	if (!gCursInited)
		return noErr;
		
	for (i=0;i<8;i++){
		HUnlock((Handle)(gCursTask.cursors[i]));
		HPurge((Handle)(gCursTask.cursors[i]));
		DisposeHandle((Handle)gCursTask.cursors[i]);
	}
	
	DisposePtr((Ptr)(gCursTask.cursors));

#ifndef NotUniversalHdrs
	// need to get rid of the routine descriptor...
	DisposeRoutineDescriptor(gCursTask.vblTask.vblAddr);
#endif /* NotUniversalHdrs */
	
	return noErr;										/* and return no error */
}
