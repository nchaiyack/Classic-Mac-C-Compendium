/* CommandView - Implementation of TCommandView                               */

#ifndef __COMMANDVIEW__
#define __COMMANDVIEW__

		// ¥ MacApp
#ifndef __UMacApp__
#include "UMacApp.h"
#endif

#ifndef __UDialog__
#include "UDialog.h"
#endif


class TCommandView: public TTEView {
public:
	int fMaxLines, fCurIndex, fFirstIndex, fShowIndex, fSelStart, fSelEnd;
	Handle **fHandList;
	TDocument *fMyDocument;
	
	virtual pascal void CalcMinSize (VPoint *minSize);
	virtual pascal Boolean ClikLoop (void);
	virtual pascal TCommand *DoKeyCommand (short ch, short aKeyCode,
		EventInfo *info);
	virtual pascal TCommand *DoMouseCommand (Point *theMouse, EventInfo *info,
		Point *hysteresis);
	virtual pascal void Free (void);
	virtual pascal long GrowZone (long needed);
	virtual pascal void IRes (TDocument *itsDocument, TView *itsSuperView,
		Ptr *itsParams);
	virtual pascal void RevealRect (VRect *rectToReveal, Point minToSee,
		Boolean redraw);
	virtual pascal void ScrollHist (Boolean scrollUp);
	virtual pascal void SendLine (void);
	virtual pascal void SetBaseStyle (TextStyle *theStyle, Boolean redraw);
	virtual pascal void SetHistSize (long lines);
};


void InitCommandView (void);

#endif
