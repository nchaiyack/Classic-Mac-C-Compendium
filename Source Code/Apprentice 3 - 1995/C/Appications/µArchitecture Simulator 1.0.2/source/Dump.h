#ifndef __FABCURSORBALLOON__
#include	"CursorBalloon.h"
#endif

extern ControlHandle	dumpVScroll;
extern short	dumpLineHeight, dumpFromTop, dumpCWidMax;

void DumpHome(void);
void DumpEnd(void);
void DumpPgUp(void);
void DumpPgDn(void);
void Update_Dump(WindowPtr w);
void Do_Dump(WindowPtr w, EventRecord *myEvent);
void Grow_Dump(WindowPtr w, EventRecord *event);
void Activate_Dump(EventRecord *, WindowPtr w, Boolean becomingActive);
void DrawDump(WindowPtr w);
void SetupDumpCtlMax(ControlHandle theControl);
void InvalDump(void);
void CloseDump(WindowPtr w);
void RecalcDump(FabWindowPtr w, RgnBalloonCursPtr theObj);
void getDragRectDump(WindowPtr w, RectPtr r);
