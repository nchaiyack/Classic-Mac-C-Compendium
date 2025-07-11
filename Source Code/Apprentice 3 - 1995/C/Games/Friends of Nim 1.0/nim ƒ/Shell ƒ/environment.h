#ifndef __MY_ENVIRONMENT_H__
#define __MY_ENVIRONMENT_H__

extern	Boolean			gHasColorQD;
extern	Boolean			gHasNotificationManager;

extern	short			gForegroundWaitTime;
extern	short			gBackgroundWaitTime;
extern	Boolean			gIsInBackground;
extern	Boolean			gInProgress;
extern	Boolean			gDone;
extern	short			gFrontWindowIndex;
extern	Boolean			gIsVirgin;
extern	Boolean			gCustomCursor;
extern	short			gKludgeIter;
extern	Boolean			gNeedToOpenWindow;

#include "program globals.h"

#ifdef __cplusplus
extern "C" {
#endif

extern	ErrorTypes InitTheEnvironment(void);

#ifdef __cplusplus
}
#endif

#endif
