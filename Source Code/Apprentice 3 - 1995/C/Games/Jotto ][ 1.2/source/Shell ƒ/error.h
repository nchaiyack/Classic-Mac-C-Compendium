#include "program globals.h"
#include <Notification.h>

extern	NMRec			gMyNotification;
extern	enum ErrorTypes	gPendingResultCode;

void FailNilUPP(UniversalProcPtr theUPP);
void HandleError(enum ErrorTypes resultCode, Boolean exitToShell);
