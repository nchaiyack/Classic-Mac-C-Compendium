// AsyncPeriodical.cp
// Copyright ©1994 David Kreindler. All rights reserved.

// definitions for class AsyncPeriodical, VBLPeriodical and TMPeriodical

// revision history
//	940212 DK: initial version
//	940219 DK: added class TMPeriodical
//	940226 DK: added code for A4-based globals

#ifdef __MWERKS__
#	pragma options align=mac68k
#endif

#include <osutils.h>		// for LMGetCurrentA5

#ifdef __MWERKS__
#	pragma options align=reset
#endif

#include "asyncperiodical.h"

#ifdef OUTLINE_INLINES
#	define inline
#	include "asyncperiodical.i"
#	undef inline
#endif

// applications use the 68K A5 register for global (static data member) access; other code uses A4

#ifndef __a5__
#	define __a5__ 1
#endif

#if __a5__
#else
#	include <a4stuff.h>
#	define SetCurrentA5 GetCurrentA4
#	define SetA5 SetA4
#endif

#ifndef ABS
#	define ABS(x) ((x) >= 0 ? (x) : (-(x)))
#endif

//
// class AsyncPeriodical
//

AsyncPeriodical::AsyncPeriodical(): mTaskIsRunning(false) {
}

AsyncPeriodical::~AsyncPeriodical() {
}

void
AsyncPeriodical::Start(Ticks taskInterval) {
	mInterval = taskInterval;
}

void
AsyncPeriodical::Stop() {
}

//
// class VBLPeriodical
//

VBLUPP VBLPeriodical::sProc = nil;

VBLPeriodical::VBLPeriodical() {

	// initialize the constant parts of the VBLTask
	// note that mTask.task.vblCount is set in Start
	
	if (sProc == nil) {
		sProc = NewVBLProc(HandleVBLTask);
	}
	
	mTask.task.qType = vType;
	mTask.task.vblAddr = sProc;	
	mTask.task.vblPhase = 0;
	mTask.thisP = this;
	mTask.a5 = SetCurrentA5();
}

VBLPeriodical::~VBLPeriodical() {

	// make sure that we have removed the VBLTask before we go away
	
	Stop();
}

void
VBLPeriodical::Start(Ticks taskInterval) {

	if (!IsRunning()) {

	// let the superclass do its thing (set mInterval)
	
		AsyncPeriodical::Start(taskInterval);
	
	// install the VBL task

		mTask.task.vblCount = ABS(taskInterval);	// taskInterval could be negative
		IsRunning() = VInstall((QElemPtr)&mTask) == noErr;
	}
}

void
VBLPeriodical::Stop() {
	
	if (IsRunning()) {

	// remove the VBL task

		IsRunning() = !(VRemove((QElemPtr)&mTask) == noErr);
	
	// let the superclass do its thing (nothing)
	
		AsyncPeriodical::Stop();
	}
}

#ifdef __powerc
pascal void
VBLPeriodical::HandleVBLTask(VBLTaskPtr taskP) {
#else
// this definition uses the Metrowerks function parameter 68K register declaration syntax;
// other compilers might require a #pragma parameter directive instead
pascal void
VBLPeriodical::HandleVBLTask(VBLTaskPtr taskP: __A0) {
#endif

	// set up the globals register
	
	long prevGlobals = SetA5(taskP->a5);
	
	// call the virtual member function to do the work
	
	taskP->thisP->DoTask();	// this->DoTask()

	// update the VBLTask

	taskP->task.vblCount = ABS(taskP->thisP->Interval());
	
	// restore the globals register

	SetA5(prevGlobals);
}

//
// class TMPeriodical
//

TimerUPP TMPeriodical::sProc = nil;

TMPeriodical::TMPeriodical() {

	// initialize and install the TMTask
	
	if (sProc == nil) {
		sProc = NewTimerProc(HandleTimerTask);
	}
	
	mTask.task.tmAddr = sProc;
	mTask.thisP = this;
	mTask.a5 = SetCurrentA5();
	InsTime((QElemPtr)&mTask);
}

TMPeriodical::~TMPeriodical() {
	
	// remove the TMTask
	
	RmvTime((QElemPtr)&mTask);
}
	
void
TMPeriodical::Start(Ticks taskInterval) {
	
	// let the superclass do its thing (set mInterval)
	
	AsyncPeriodical::Start(taskInterval);
	
	// start the TMTask
	
	IsRunning() = true;
	PrimeTime((QElemPtr)&mTask, ABS(taskInterval));
}

void
TMPeriodical::Stop() {

	IsRunning() = false;
	
	// let the superclass do its thing (nothing)
	
	AsyncPeriodical::Stop();
}
	
#ifdef __powerc
pascal void
TMPeriodical::HandleTimerTask(TMTaskPtr taskP) {
#else
// this declaration uses the Metrowerks function parameter 68K register declaration syntax;
// other compilers might require a #pragma parameter directive instead
pascal void
TMPeriodical::HandleTimerTask(TMTaskPtr taskP: __A1) {
#	endif

	// set up the globals register
	
	long prevGlobals = SetA5(taskP->a5);
	
	if (taskP->thisP->IsRunning()) {	// make sure that we are supposed to be executing
	
	// call the virtual member function to do the work
	
		taskP->thisP->DoTask();			// this->DoTask()

	// update the TMTask's queue entry

		PrimeTime((QElemPtr)taskP, ABS(taskP->thisP->Interval()));
	}
	
	// restore the globals register

	SetA5(prevGlobals);
}