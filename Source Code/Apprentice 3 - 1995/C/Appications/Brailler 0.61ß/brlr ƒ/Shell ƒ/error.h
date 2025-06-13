#ifndef __ERROR_H__
#define __ERROR_H__

#include "program globals.h"
#include <Notification.h>

typedef struct
{
	Boolean			isFatal;
	Boolean			isSmall;
	NMRec			notificationRec;
	enum ErrorTypes	resultCode;
} ErrorStateRec;

extern	ErrorStateRec		gPendingErrorRec;

void FailNilUPP(UniversalProcPtr theUPP);
void HandleError(enum ErrorTypes resultCode, Boolean isFatal, Boolean isSmall);

#endif
