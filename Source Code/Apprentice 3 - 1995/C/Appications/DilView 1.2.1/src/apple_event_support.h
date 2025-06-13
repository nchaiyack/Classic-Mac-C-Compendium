/* apple_event_support.h */

#include <AppleEvents.h>

void myEventInit( void );
pascal void myHandleHighLevelEvent( EventRecord *theEvent );

pascal OSErr DoOpenApp( AppleEvent *theAppleEvent, AppleEvent *reply, long refCon );
pascal OSErr DoOpenDoc( AppleEvent *theAppleEvent, AppleEvent *reply, long refCon );
pascal OSErr DoPrintDoc( AppleEvent *theAppleEvent, AppleEvent *reply, long refCon );
pascal OSErr DoQuitApp( AppleEvent *theAppleEvent, AppleEvent *reply, long refCon );
