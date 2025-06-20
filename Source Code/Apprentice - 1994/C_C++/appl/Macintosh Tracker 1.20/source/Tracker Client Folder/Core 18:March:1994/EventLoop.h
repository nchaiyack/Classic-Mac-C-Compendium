/* EventLoop.h */

#pragma once

#include "CObject.h"

class CWindow;
class CViewRect;

#define MENUMINFLASH (5)

#define DefaultSleepTime (15)

typedef struct
	{
		LongPoint		Where;
		long				When;
		short				Modifiers;
		long				Message;
	} MyEventRec;

struct	CIdle	:	CObject
	{
		ulong		MinSleepTime;
		EXECUTE(MyBoolean Installed;)

		EXECUTE(/* */	CIdle();)
		EXECUTE(/* */ ~CIdle();)
		void		DoIdle(long TimeSinceLastEvent);
	};

void					InitMyEventLoop(void);
void					ShutDownMyEventLoop(void);
void					TheEventLoop(CWindow* TheModalWindow);
void					RegisterIdler(CIdle* TheIdler, long MinSleepTime);
void					DeregisterIdler(CIdle* TheIdler);
void					SendIdleToAll(void);
void					SendIdleToAllJudiciously(void);
MyBoolean			RelinquishCPU(void);
MyBoolean			RelinquishCPUJudiciously(void);

#ifndef COMPILING_EVENTLOOP_C
	extern CViewRect*	LastMouseDownViewRect;
	extern CViewRect*	KeyReceiverViewRect;
	extern CViewRect*	LastKeyDownViewRect;
	extern MyBoolean	QuitPending;
	extern MyBoolean	WeAreActiveApplication;
	extern MyBoolean	ModalWindowTerminationFlag;
	extern short			LastModifiers;
	extern ulong			LastModifiersTime;
#endif
