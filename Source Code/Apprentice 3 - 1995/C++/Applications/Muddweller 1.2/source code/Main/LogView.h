/* LogView - Implementation of TLogView                                       */

#ifndef __LOGVIEW__
#define __LOGVIEW__

		// � MacApp
#ifndef __UMacApp__
#include "UMacApp.h"
#endif

		// � Implementation use
#ifndef __BIGTEXT__
#include "BigText.h"
#endif


class TLogView: public TView {
public:
	TBigText *fBT;
	TScrollBar *fScrollBar;
	
	virtual pascal void Activate (Boolean entering);
	virtual pascal long AddText (unsigned char *textBuf, long count);
	virtual pascal void ClikLoop (Point pt);
	virtual pascal void DoCopy (void);
	virtual pascal TCommand *DoKeyCommand (short ch, short aKeyCode,
		EventInfo *info);
	virtual pascal TCommand *DoMenuCommand (CmdNumber aCmdNumber);
	virtual pascal struct TCommand *DoMouseCommand (Point *theMouse,
		EventInfo *info, Point *hysteresis);
	virtual pascal Boolean DoSetCursor (Point localPoint, RgnHandle cursorRgn);
	virtual pascal void DoSetupMenus (void);
	virtual pascal void Draw (Rect *area);
	virtual pascal void Free (void);
	virtual pascal void InstallSelection (Boolean wasActive, Boolean beActive);
	virtual pascal void IRes (TDocument *itsDocument, TView *itsSuperView,
		Ptr *itsParams);
	virtual pascal void Open (void);
	virtual pascal void Resize (VCoordinate width, VCoordinate height,
		Boolean invalidate);
	virtual pascal void SetBaseStyle (TextStyle *theStyle, Boolean redraw);
	virtual pascal void SetMaxSize (long logSize);
	virtual pascal void SynchScrollBar (Boolean redraw);
};


void InitLogView (void);

#endif
