
#ifndef __FABCURSORBALLOON__
#include	"CursorBalloon.h"
#endif


/* kMinDocDim is used to limit the minimum dimension of a window when GrowWindow
	is called. */
enum {
kMinDocDim	= 63
};

	
//#define	MIN(a, b)	((a) < (b) ? (a) : (b))
	
typedef struct {
	FabWindowRecord	docWindow;
	TEHandle		docTE;
	ControlHandle	docVScroll;
	ControlHandle	docHScroll;
} DocumentRecord, *DocumentPeek;

void IOHome(void);
void IOEnd(void);
void IOPgUp(void);
void IOPgDn(void);
void DoGrowWindow(WindowPtr window, EventRecord	*event);
void DoZoomWindow(WindowPtr window);
void ResizeWindow(WindowPtr window);
void DoUpdateWindow(WindowPtr window);
void DoActivateWindow(EventRecord *, WindowPtr window, Boolean becomingActive);
void DoContentClick(WindowPtr window, EventRecord *event);
void DoKeyDown(WindowPtr window, unsigned char key, Boolean);
void GetTERect(WindowPtr window, RectPtr teRect);
void AdjustScrollbars(WindowPtr window, Boolean needsResize);
void CloseIO(WindowPtr w);
void RecalcIO(FabWindowPtr w, RgnBalloonCursPtr theObj);
