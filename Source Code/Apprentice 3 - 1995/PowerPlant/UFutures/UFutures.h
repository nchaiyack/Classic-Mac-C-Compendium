// =================================================================================
//	UFutures.h
// =================================================================================
//	Copyright:
//	� 1993 by Apple Computer, Inc., all rights reserved.
//	� 1993-1994 by Steve Sisak (sgs@gnu.ai.mit.edu), all rights reserved.
//	� 1994 by Harold Ekstrom (ekstrom@aggroup.com), all rights reserved.

#pragma once

#ifndef __APPLEEVENTS__
#include <AppleEvents.h>
#endif

#ifndef __THREADS__
#include <Threads.h>
#endif

#include <LThread.h>

#define __FUTURES__

typedef pascal OSErr (*AESpecialHandlerProcPtr)(AppleEvent *theAppleEvent);

enum {
	uppAESpecialHandlerProcInfo = kPascalStackBased
		 | RESULT_SIZE(SIZE_CODE(sizeof(OSErr)))
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(AppleEvent*)))
};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr AESpecialHandlerUPP;
#define CallAESpecialHandlerProc(userRoutine, theAppleEvent, reply, handlerRefcon)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppAESpecialHandlerProcInfo, (theAppleEvent), (reply), (handlerRefcon))
#define NewAESpecialHandlerProc(userRoutine)		\
		(AESpecialHandlerUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppAESpecialHandlerProcInfo, GetCurrentISA())
#else
typedef ProcPtr AESpecialHandlerUPP;
#define CallAESpecialHandlerProc(userRoutine, theAppleEvent, reply, handlerRefcon)		\
		(*(userRoutine))((theAppleEvent), (reply), (handlerRefcon))
#define NewAESpecialHandlerProc(userRoutine)		\
		(AESpecialHandlerUPP)(userRoutine)
#endif

// List of blocked threads.
typedef struct SThreadList {
	short		numThreads;
	LThread		*threads[1];
}	SThreadList, *SThreadListPtr, **SThreadListHdl;

// Macro for the size of the thread list.
#define sizeofThreadList(n) (sizeof(SThreadList) + ((n)-1)*sizeof(LThread *))


class UFutures {
public:

	static pascal OSErr			InitFutures();
	static pascal OSErr			Ask( AppleEvent *inEvent, AppleEvent *outReply );
	static pascal Boolean		IsFuture( const AppleEvent *inEvent );
	static pascal OSErr			BlockUntilReal( const AppleEvent *inEvent );

private:

	enum {
		kAERefconAttribute =		'refc',
		kAENonexistantAttribute =	'gag!',
		kImmediateTimeout =			0,
		keyBlockFunctionClass =		'blck',
		keyUnblockFunctionClass =	'unbk'
	};
		
	static pascal OSErr			IsFutureBlock( AppleEvent *inEvent );
	static pascal OSErr			DoThreadBlock( AppleEvent *inEvent );
	static pascal OSErr			DoThreadUnblock( AppleEvent *inEvent );
	
	static AESpecialHandlerUPP	sDoThreadBlockUPP;
	static AESpecialHandlerUPP	sDoThreadUnblockUPP;
	static AESpecialHandlerUPP	sIsFutureBlockUPP;

};
