/* mac_event.h */

#pragma once

#include <AppleEvents.h>
#include <Aliases.h>


#define BUG(x)
/* #define BUG(x) DebugStr(x) */

/* #define AskForFile */


#define ControlEventClass ('�Tsp')

#define ControlEvent ('�Ctl')
#define keyKeyPressCharacter ('�Key')

#define ErrorEvent ('�Err')
#define keyErrorIDNum ('�Er#')

/* modifiers for open event */
#define keyAntiAliasing ('�Saa')
#define keyStereoOn ('�Sso')
#define keySamplingRate ('�Ssr')
#define keyNumRepeats ('�Snr')
#define keySpeed ('�Ssp')
#define keyStereoMix ('�Ssm')
#define keyLoudness ('�Svl')
#define keyNumBits ('�Bit')
#define keySenderPSN ('�Psn')


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
