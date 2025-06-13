/* See the file Distribution for distribution terms.
	(c) Copyright 1994-1995 Ari Halberstadt */

/*	Header for the Thread Library Manager compatability layer. The layer provides
	a very similar functional interface to Apple's Thread Manager.

	You can use the macros THREAD_MANAGER_AS_THREAD_LIBRARY and THREAD_LIBRARY_AS_THREAD_MANAGER
	to replace one functional interface with the other. For instance, if you have an
	application written using Thread Manager, and you want to try using Thread Library,
	you can just include this header file in all source files that make calls to Thread
	Manager.

	Define THREAD_MANAGER_AS_THREAD_LIBRARY as 1 to enable macros that redefine Thread
	Manager function calls to equivalent Thread Library Manager function calls. This
	is the default if you include this file.
	
	Define THREAD_LIBRARY_AS_THREAD_MANAGER as 1 to enable macros that redefine Thread
	Library Manager function calls to equivalent Thread Manager function calls. */
	
#pragma once
#ifndef THREAD_LIBRARY_MANAGER
#define THREAD_LIBRARY_MANAGER

#ifndef __THREADS__
#include <Threads.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

OSErr TLMCreateThreadPool(ThreadStyle style, short count, Size stack);
OSErr TLMGetFreeThreadCount(ThreadStyle style, short *count);
OSErr TLMGetSpecificThreadCount(ThreadStyle style, Size stack, short *count);
OSErr TLMGetDefaultThreadStackSize(ThreadStyle style, Size *size);
OSErr TLMThreadCurrentStackSpace(ThreadID id, long *free);
OSErr TLMNewThread(ThreadStyle style,
	ThreadEntryProcPtr entry,
	void *param,
	Size stack,
	ThreadOptions options,
	void **result,
	ThreadID *id);
OSErr TLMDisposeThread(ThreadID id, void *result, Boolean recycle);
OSErr TLMGetCurrentThread(ThreadID *id);
OSErr TLMYieldToAnyThread(void);
OSErr TLMThreadBeginCritical(void);
OSErr TLMThreadEndCritical(void);
OSErr TLMYieldToThread(ThreadID id);
OSErr TLMGetThreadState(ThreadID id, ThreadState *state);
OSErr TLMSetThreadState(ThreadID id, ThreadState state, ThreadID suggested);
OSErr TLMSetThreadStateEndCritical(ThreadID id, ThreadState state, ThreadID suggested);
OSErr TLMGetThreadCurrentTaskRef(ThreadTaskRef *task);
OSErr TLMGetThreadStateGivenTaskRef(ThreadTaskRef task, ThreadID id, ThreadState *state);
OSErr TLMSetThreadReadyGivenTaskRef(ThreadTaskRef task, ThreadID id);
OSErr TLMSetThreadScheduler(ThreadSchedulerProcPtr scheduler);
OSErr TLMSetThreadSwitcher(ThreadID id, ThreadSwitchProcPtr switcher,
		void *param, Boolean inout);

#if ! defined(THREAD_MANAGER_AS_THREAD_LIBRARY) && ! defined(THREAD_LIBRARY_AS_THREAD_MANAGER)
	#define THREAD_MANAGER_AS_THREAD_LIBRARY (1)
#endif

#if THREAD_MANAGER_AS_THREAD_LIBRARY

	#define CreateThreadPool					TLMCreateThreadPool
	#define GetFreeThreadCount					TLMGetFreeThreadCount
	#define GetSpecificThreadCount			TLMGetSpecificThreadCount	
	#define GetDefaultThreadStackSize		TLMGetDefaultThreadStackSize	
	#define ThreadCurrentStackSpace			TLMThreadCurrentStackSpace
	#define NewThread								TLMNewThread
	#define DisposeThread						TLMDisposeThread	
	#define GetCurrentThread					TLMGetCurrentThread		
	#define YieldToAnyThread					TLMYieldToAnyThread	
	#define ThreadBeginCritical				TLMThreadBeginCritical	
	#define ThreadEndCritical					TLMThreadEndCritical
	#define YieldToThread						TLMYieldToThread
	#define GetThreadState						TLMGetThreadState
	#define SetThreadState						TLMSetThreadState
	#define SetThreadStateEndCritical		TLMSetThreadStateEndCritical			
	#define GetThreadCurrentTaskRef			TLMGetThreadCurrentTaskRef	
	#define GetThreadStateGivenTaskRef		TLMGetThreadStateGivenTaskRef	
	#define SetThreadReadyGivenTaskRef		TLMSetThreadReadyGivenTaskRef		
	#define SetThreadScheduler					TLMSetThreadScheduler	
	#define SetThreadSwitcher					TLMSetThreadSwitcher		

#elif THREAD_LIBRARY_AS_THREAD_MANAGER

	#define TLMCreateThreadPool				CreateThreadPool
	#define TLMGetFreeThreadCount				GetFreeThreadCount
	#define TLMGetSpecificThreadCount		GetSpecificThreadCount	
	#define TLMGetDefaultThreadStackSize	GetDefaultThreadStackSize	
	#define TLMThreadCurrentStackSpace		ThreadCurrentStackSpace
	#define TLMNewThread							NewThread
	#define TLMDisposeThread					DisposeThread	
	#define TLMGetCurrentThread				GetCurrentThread		
	#define TLMYieldToAnyThread				YieldToAnyThread	
	#define TLMThreadBeginCritical			ThreadBeginCritical	
	#define TLMThreadEndCritical				ThreadEndCritical
	#define TLMYieldToThread					YieldToThread
	#define TLMGetThreadState					GetThreadState
	#define TLMSetThreadState					SetThreadState
	#define TLMSetThreadStateEndCritical	SetThreadStateEndCritical			
	#define TLMGetThreadCurrentTaskRef		GetThreadCurrentTaskRef	
	#define TLMGetThreadStateGivenTaskRef	GetThreadStateGivenTaskRef	
	#define TLMSetThreadReadyGivenTaskRef	SetThreadReadyGivenTaskRef		
	#define TLMSetThreadScheduler				SetThreadScheduler	
	#define TLMSetThreadSwitcher				SetThreadSwitcher		

#endif

#ifdef __cplusplus
}
#endif

#endif /* THREAD_LIBRARY_MANAGER */
