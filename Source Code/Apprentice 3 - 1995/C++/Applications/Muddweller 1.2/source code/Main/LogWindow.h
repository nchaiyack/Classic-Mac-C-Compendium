/* LogWindow - Implementation of TLogWindow                                   */

#ifndef __LOGWINDOW__
#define __LOGWINDOW__

		// ¥ MacApp
#ifndef __UMacApp__
#include "UMacApp.h"
#endif

#ifndef __UDialog__
#include "UDialog.h"
#endif

		// ¥ Implementation use
#ifndef __COMMANDVIEW__
#include "CommandView.h"
#endif

#ifndef __LOGVIEW__
#include "LogView.h"
#endif


class TLogWindow: public TWindow {
public:
	TLogView *fLogView;
	TCommandView *fCommandView;
	
	virtual pascal void Activate (Boolean entering);
	virtual pascal Boolean DoIdle (IdlePhase phase);
	virtual pascal void IRes (TDocument *itsDocument, TView *itsSuperView,
		Ptr *itsParams);
	virtual pascal void Open (void);
};


void InitLogWindow (void);

#endif
