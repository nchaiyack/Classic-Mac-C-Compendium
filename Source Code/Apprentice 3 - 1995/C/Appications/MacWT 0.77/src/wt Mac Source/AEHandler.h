// AEHandler.h

#include <AppleEvents.h>

void	InitAppleEvents(void);
void	DoAppleEvent(EventRecord *);
pascal	OSErr	OpenApplicationEvent(AppleEvent *, AppleEvent *, long);
pascal	OSErr	QuitApplicationEvent(AppleEvent *, AppleEvent *, long);
pascal  OSErr	OpenDocumentEvent(AppleEvent *, AppleEvent *, long);
