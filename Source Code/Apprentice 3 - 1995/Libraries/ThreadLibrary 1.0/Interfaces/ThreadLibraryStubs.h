/* See the file Distribution for distribution terms.
	(c) Copyright 1994-1995 Ari Halberstadt */

/*
	Following are stubs that you can include in your source files.
	These stubs will remove any threaded behavior from your program.
	When you call ThreadBegin, the thread's entry point will be
	called immediately. Control will resume after the call to
	ThreadBegin when the thread's entry point has exited.
	
	You must include this file either instead of "ThreadLib.h",
	or before you include "ThreadLib.h" in your source files.
	
	With these stubs, you can't:
	
	- use ThreadDataSet to store a data pointer
	the thread
	
	- have the full complement of thread callbacks executed
	
	- iterate over threads, since there are no threads
	
	- use threads that run in an infinite loop, since they'll
	never return control to the remainder of the program
	
	Warning: these stubs violate the conditions maintained by
	Thread Library and expressed in its assertions and documentation.
	These stubs also use macros that may discard their parameters without
	evaluating them. These stubs are merely meant to enable you to test
	your application without threads, in the event you suspect that Thread
	Library is causing errors in your software.
	
	950214 aih updated for latest interface
	941003 aih created
*/

#pragma once
#ifndef THREAD_LIBRARY_STUBS
#define THREAD_LIBRARY_STUBS

#include "ThreadLibrary.h"

/*	If this file is included, then stubs are enabled
	by default. */
#ifndef THREAD_STUB_ENABLED
	#define THREAD_STUB_ENABLED (1)
#endif

#if THREAD_STUB_ENABLED

	#define THREAD_STUB_SN				((ThreadType) 1)

	#define ThreadError()				((ThreadErrorType) 0)
	#define ThreadErrorSet()			((void) 0)
	#define ThreadCount()				(THREAD_STUB_SN)
	#define ThreadMain()				(THREAD_STUB_SN)
	#define ThreadActive()				(THREAD_STUB_SN)
	#define ThreadFirst()				(THREAD_STUB_SN)
	#define ThreadNext(t)				(t)
	#define ThreadExists(t)				(true)
	#define ThreadSaveFPU(t)			(false)
	#define ThreadSaveFPUSet(t,f)		((void) 0)
	#define ThreadData(t)				(NULL)
	#define ThreadDataSet(t,d)			((void) 0)
	#define ThreadProcAllocate()		(NULL)
	#define ThreadProcAllocateSet(f)	((void) 0)
	#define ThreadProcDispose()			(NULL)	
	#define ThreadProcDisposeSet(f)		((void) 0)
	#define ThreadProcBegin(t)			(NULL)	
	#define ThreadProcBeginSet(t,f)		((void) 0)
	#define ThreadProcEnd(t)			(NULL)	
	#define ThreadProcEndSet(t,f)		((void) 0)
	#define ThreadProcResume(t)			(NULL)	
	#define ThreadProcResumeSet(t,f)	((void) 0)
	#define ThreadProcSuspend(t)		(NULL)	
	#define ThreadProcSuspendSet(t,f)	((void) 0)
	#define ThreadProcEntry(t)			(NULL)	
	#define ThreadProcEntrySet(t,f)		((void) 0)
	#define ThreadStackMinimum()		((size_t) 0)
	#define ThreadStackDefault()		((size_t) 0)
	#define ThreadStackSpace(t)			((size_t) StackSpace())
	#define ThreadStackFrame(t,f)		((void) 0)
	#define ThreadEnabled(t)			(false)
	#define ThreadEnabledSet(t,f)		((void) 0)
	#define ThreadSleepSet(t,s)			((void) 0)
	#define ThreadSchedule()			(THREAD_STUB_SN)
	#define ThreadActivate(t)			((void) 0)
	#define ThreadYield(s)				((void) 0)
	#define ThreadYieldInterval()		(0)
	#define ThreadBeginMain(s,r,d)		(THREAD_STUB_SN)
	#define ThreadBegin(e,s,r,d,ss)		(THREAD_STUB_SN)
	#define ThreadEnd(t)				((void) 0)
	#define ThreadEndAll(t)				((void) 0)

#endif /* THREAD_STUB_ENABLED */

#endif /* THREAD_LIBRARY_STUBS */
