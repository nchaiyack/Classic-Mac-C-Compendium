/* mac_event.h */

#pragma once

#include <AppleEvents.h>
#include <Aliases.h>


#define BUG(x)
/* #define BUG(x) DebugStr(x) */

/* #define AskForFile */


#define ControlEventClass ('ºTsp')

#define ControlEvent ('ºCtl')
#define keyKeyPressCharacter ('ºKey')

#define ErrorEvent ('ºErr')
#define keyErrorIDNum ('ºEr#')

/* modifiers for open event */
#define keyAntiAliasing ('ºSaa')
#define keyStereoOn ('ºSso')
#define keySamplingRate ('ºSsr')
#define keyNumRepeats ('ºSnr')
#define keySpeed ('ºSsp')
#define keyStereoMix ('ºSsm')
#define keyLoudness ('ºSvl')
#define keyNumBits ('ºBit')
#define keySenderPSN ('ºPsn')


#define MAXKEYS (64)


void				WaitForEvent(long SleepTime);

short				NumberPendingBlocks(void);

void				TogglePause(void);

void				FatalError(short ErrorID);
#define FatalErrorUnknown (0)
#define FatalErrorOutOfMemory (1)
#define FatalErrorInternalError (2)
#define FatalErrorCantOpenCompressedFiles (3)
#define FatalErrorCouldntOpenFile (4)
#define FatalErrorCouldntCloseFile (5)
#define FatalErrorNotASong (6)
#define FatalError68020NeededID (7)

int					RegisterEventHandlers(void);
