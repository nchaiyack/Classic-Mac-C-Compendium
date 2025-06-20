#define USE_STYLED_TEXT		0

#include <Drag.h>

void InitTheDragManager(void);
Boolean DragManagerAvailableQQ(void);
short TEIsFrontOfLine(short offset, TEHandle theTE);
short TEGetLine(short offset, TEHandle theTE);
short HitTest(Point theLoc, WindowPtr *theWindow);
void DrawCaret(short offset, TEHandle theTE);
#if USE_STYLED_TEXT
void InsertTextAtOffset(short offset, char *theBuf, long size, StScrpHandle theStyl, TEHandle theTE);
#else
void InsertTextAtOffset(short offset, char *theBuf, long size, TEHandle theTE);
#endif
short GetSelectionSize(WindowPtr theWindow);
Ptr GetSelectedTextPtr(WindowPtr theWindow);
#if USE_STYLED_TEXT
pascal OSErr MySendDataProc(FlavorType theType, void *refCon,
							ItemReference theItem, DragReference theDrag);
#endif
pascal OSErr MyReceiveDropHandler(WindowPtr duhWindow, unsigned long handlerRefCon,
								  DragReference theDrag);
pascal OSErr MyTrackingHandler(short message, WindowPtr duhWindow,
							   void *handlerRefCon, DragReference theDrag);
Boolean DropLocationIsFinderTrash(AEDesc *dropLocation);
Boolean DragText(WindowPtr theWindow, EventRecord *theEvent, RgnHandle hiliteRgn);
Boolean DragInWindow(WindowPtr theWindow, EventRecord *theEvent);
void ResetHiliteRgn(WindowPtr theWindow);
Boolean CursorInDraggableRgn(Point thePoint, WindowPtr theWindow);
