#ifndef __MY_ERROR_LAYER_H__
#define __MY_ERROR_LAYER_H__

#include "program globals.h"
#include <Notification.h>

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif
struct ErrorStateRec
{
	Boolean			isFatal;
	Boolean			isSmall;
	NMRec			notificationRec;
	ErrorTypes		resultCode;
};
#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

typedef struct ErrorStateRec	ErrorStateRec;

extern	ErrorStateRec		gPendingErrorRec;

#ifdef __cplusplus
extern "C" {
#endif

extern	void FailNilUPP(UniversalProcPtr theUPP);
extern	void HandleError(ErrorTypes resultCode, Boolean isFatal, Boolean isSmall);

#ifdef __cplusplus
}
#endif

#endif
