/* CModalDialog.c */

#define COMPILING_CMODALDIALOG_C
#include "CModalDialog.h"
#include "EventLoop.h"
#include "MenuController.h"
#include "CScrap.h"
#include "CApplication.h"


/* */		CModalDialog::~CModalDialog()
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CModalDialog::~CModalDialog called on uninitialized object."));
		*GoAwayLocation = True; /* makes the event loop quit as soon as possible */
	}


/* after calling this from your initialization routine, call DoEventLoop */
void		CModalDialog::IModalDialog(LongPoint Start, LongPoint Extent,
					MyBoolean AllowMenus)
	{
		ERROR(Initialized == True,PRERR(ForceAbort,
			"CModalDialog::IModalDialog called on already initialized object."));
		EXECUTE(Initialized = True);
		IWindow(Start,Extent,ModalWindow,NoGrowable,NoZoomable);
		SelectWindow(MyGrafPtr);
	}


void		CModalDialog::MakeMyGrafPtr(LongPoint Start, LongPoint Extent)
	{
		Rect		BoundsRect;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CModalDialog::MakeMyGrafPtr called on uninitialized object."));
		ERROR(MyGrafPtr!=NIL,PRERR(ForceAbort,
			"CModalDialog::MakeMyGrafPtr MyGrafPtr is not NIL."));
		BoundsRect = RectOf(Start.x,Start.y,Start.x+Extent.x,Start.y+Extent.y);
		MyGrafPtr = NewWindow(NIL,&BoundsRect,"\p",True,dBoxProc,(void*)-1,True,(long)this);
		if (MyGrafPtr == NIL)
			{
				PRERR(ForceAbort,"NewWindow failed to allocate memory.");
			}
	}


void		CModalDialog::DoEventLoop(void)
	{
		EventRecord		MyEvent;
		WindowPtr			WhichWindow;
		MyEventRec		SpiffEvent;
		short					MenuCommand;
		MyBoolean			GoAwayNow;
		long					MenuCommandStart;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CModalDialog::DoEventLoop called on uninitialized object."));
		GoAwayLocation = &ModalWindowTerminationFlag;
		TheEventLoop(this);
		ActiveWindow = NIL; /* we end by having delete performed on us */
	}
