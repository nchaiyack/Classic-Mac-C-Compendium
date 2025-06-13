// File "aevents.h" - 

#ifndef ____AEVENTS_HEADER_____
#define ____AEVENTS_HEADER_____

#include <AppleEvents.h>

// * **************************************************************************** * //
// * **************************************************************************** * //
// Function Prototypes

pascal short AEHandlerOAPP(AppleEvent *event, AppleEvent *reply, long refCon);
pascal short AEHandlerODOC(AppleEvent *event, AppleEvent *reply, long refCon);
pascal short AEHandlerPDOC(AppleEvent *event, AppleEvent *reply, long refCon);
pascal short AEHandlerQUIT(AppleEvent *event, AppleEvent *reply, long refCon);
void InitHLEvents(void);

#endif  ____AEVENTS_HEADER_____
