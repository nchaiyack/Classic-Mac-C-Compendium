/* See the file Distribution for distribution terms.
	(c) Copyright 1994-1995 Ari Halberstadt */

#pragma once
#ifndef THREAD_LIBRARY
#define THREAD_LIBRARY

#include <limits.h>
#include <stddef.h>
#include <Types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define THREAD_VERSION		(0x0100)		/* version of thread library */
#define THREAD_SIZE_MAX		(LONG_MAX)	/* maximum size of a pointer */
#define THREAD_TICKS_MAX	(LONG_MAX)	/* maximum number of ticks */
#define THREAD_TICKS_SEC	(60L)			/* number of ticks in a second */
#define THREAD_NONE			(0)			/* serial number of an invalid thread */

/* memory types */
typedef unsigned long ThreadTypeType;
enum {
	THREAD_TYPE_NONE			= 0,
	THREAD_TYPE_FREE			= 'free',	/* free memory */
	THREAD_TYPE_ALLOC			= 'aloc',	/* allocated memory */
	THREAD_TYPE_UNSPECIFIED	= 'unsp',	/* unspecified memory type */
	THREAD_TYPE_THREAD		= 'thrd',	/* memory for private thread structure */
	THREAD_TYPE_STACK			= 'stak',	/* memory for stack */
	THREAD_TYPE_LAST
};

/* error numbers */
typedef OSErr ThreadErrorType;
enum {
	THREAD_ERROR_NONE			= 0,			/* no error */
	THREAD_ERROR_NOT_FOUND	= -618,		/* thread not found (threadNotFoundErr) */
	THREAD_ERROR_PROTOCOL	= -619,		/* error in protocol (threadProtocolErr) */
	THREAD_ERROR_LAST
};

/* thread types */

typedef Ptr ThreadStackPtr;				/* pointer into a thread's stack */
typedef long ThreadType;					/* thread reference */
typedef long ThreadTicksType;				/* clock ticks */
typedef long ThreadSizeType;				/* size of a pointer */
typedef void *ThreadDataType;				/* application defined data */

/* scheduler callback function */
typedef ThreadType (*ThreadProcScheduleType)(ThreadType suggested);

/* memory allocation callback function */
typedef void *(*ThreadProcAllocateType)(ThreadSizeType size, ThreadTypeType type);

/* memory disposal callback function */
typedef void (*ThreadProcDisposeType)(void *p,  ThreadSizeType size,
	ThreadTypeType type);

/* thread resume, suspend, and entry callback functions */
typedef void (*ThreadProcType)(ThreadDataType data);

/* thread begin/end callback functions */
typedef void (*ThreadProcBeginEndType)(ThreadType thread, ThreadDataType data);

#if THREAD_OBSOLETE
	/* The type ThreadSNType is a synonym for the type ThreadType.
		Applications should refer to threads using variables of type
		ThreadType. The type ThreadSNType is included for compatability
		with versions 1.0d2.2 through 1.0d3. New applications should no
		longer use the type ThreadSNType. The same applies to
		THREAD_SN_NONE. */
	#define THREAD_SN_NONE THREAD_NONE
	typedef ThreadType ThreadSNType;
#endif /* THREAD_OBSOLETE */

ThreadErrorType ThreadError(void);
void ThreadErrorSet(ThreadErrorType error);

long ThreadCount(void);
ThreadType ThreadMain(void);
ThreadType ThreadActive(void);
ThreadType ThreadFirst(void);
ThreadType ThreadNext(ThreadType thread);

Boolean ThreadExists(ThreadType thread);

ThreadDataType ThreadData(ThreadType thread);
void ThreadDataSet(ThreadType thread, ThreadDataType data);

ThreadProcScheduleType ThreadProcSchedule(void);
void ThreadProcScheduleSet(ThreadProcScheduleType schedule);

ThreadProcAllocateType ThreadProcAllocate(void);
void ThreadProcAllocateSet(ThreadProcAllocateType allocate);

ThreadProcDisposeType ThreadProcDispose(void);
void ThreadProcDisposeSet(ThreadProcDisposeType dispose);

ThreadProcBeginEndType ThreadProcBegin(ThreadType thread);
void ThreadProcBeginSet(ThreadType thread, ThreadProcBeginEndType begin);

ThreadProcBeginEndType ThreadProcEnd(ThreadType thread);
void ThreadProcEndSet(ThreadType thread, ThreadProcBeginEndType end);

ThreadProcType ThreadProcResume(ThreadType thread);
void ThreadProcResumeSet(ThreadType thread, ThreadProcType resume);

ThreadProcType ThreadProcSuspend(ThreadType thread);
void ThreadProcSuspendSet(ThreadType thread, ThreadProcType suspend);

ThreadProcType ThreadProcEntry(ThreadType thread);
void ThreadProcEntrySet(ThreadType thread, ThreadProcType entry);

ThreadSizeType ThreadStackMinimum(void);
ThreadSizeType ThreadStackDefault(void);
ThreadSizeType ThreadStackSize(ThreadType thread);
ThreadSizeType ThreadStackSpace(ThreadType thread);

ThreadStackPtr ThreadStackFramePointer(ThreadType thread);

Boolean ThreadEnabled(ThreadType thread);
void ThreadEnabledSet(ThreadType thread, Boolean enabled);

ThreadTicksType ThreadWakeTime(ThreadType thread);
void ThreadWakeTimeSet(ThreadType thread, ThreadTicksType wake);

void ThreadSleepSet(ThreadType thread, ThreadTicksType sleep);
void ThreadSleepIntervalSet(ThreadType thread, ThreadTicksType sleep);

ThreadType ThreadSchedule(void);

void ThreadActivate(ThreadType thread);
void ThreadYield(ThreadTicksType sleep);
ThreadTicksType ThreadYieldInterval(void);

ThreadType ThreadBeginMain(
	ThreadProcType suspend,
	ThreadProcType resume,
	ThreadDataType data);

ThreadType ThreadBegin(
	ThreadProcType entry,
	ThreadProcType suspend,
	ThreadProcType resume,
	ThreadDataType data,
	ThreadSizeType stack_size);

void ThreadEnd(ThreadType thread);
void ThreadEndAll(void);

#ifdef __cplusplus
}
#endif

#endif /* THREAD_LIBRARY */
