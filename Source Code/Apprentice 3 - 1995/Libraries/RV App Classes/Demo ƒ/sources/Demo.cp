#include <Aliases.h>
#include <AppleEvents.h>
#include <Drag.h>
#include <QuickDraw.h>
#include <QDOffscreen.h>

#include "memobject.h"
#include "handle.h"
#include "qdobject.h"
#include "recordable.h"
#include "rect.h"
#include "region.h"

#include "appleeventhandler.h"
#include "draghandler.h"

#include "grafport.h"
#include "windowrecord.h"
#include "window.h"
#include "dialog.h"
#include "application.h"

#include "demoapp.h"

void main()
{
	//
	// the Apple Event handlers must stay around long
	//
	OAPP_Handler		theOAPP_Handler;
	QUIT_Handler		theQUIT_Handler;
	ODOC_PDOC_Handler	theODOC_Handler( kAEOpenDocuments);
	ODOC_PDOC_Handler	thePDOC_Handler( kAEPrintDocuments);

	demoapp me;

	me.EventLoop();
}
