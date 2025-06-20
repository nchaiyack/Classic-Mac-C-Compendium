#ifndef __EVENTS__
#include <Events.h>
#endif

#ifndef __DIALOGS__
#include <Dialogs.h>
#endif

#ifndef __DEVICES__
#include <Devices.h>
#endif

#ifndef __OSUTILS__
#include <OSUtils.h>
#endif

#ifndef __CCONTROLPANEL__
#include "CControlPanel.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

pascal long EntryPoint(short message, short item, short numItems, short reserved, EventRecord *theEvent, long refCon, DialogPtr theDialog);

#ifdef __cplusplus
}
#endif

Boolean	CanPanelRun(void);
long	CallControlPanel(short message, short item, EventRecord *theEvent, DialogPtr theDialog, long refCon);

pascal long EntryPoint(short message, short item, short numItems, short reserved, EventRecord *theEvent, long refCon, DialogPtr theDialog)
{
	switch (message)
	{
		case macDev:			// can this run? Return true if possible, false if not.
			refCon = CanPanelRun();
			break;
		case initDev:
		{
			CControlPanel	*thePanel;
			thePanel = new CControlPanel(numItems);
			if (thePanel)
			{
				refCon = (long) thePanel;
				refCon = CallControlPanel(message, item, theEvent, theDialog, refCon);
			}
			else refCon = cdevMemErr;
			break;
		}
		default:
			refCon = CallControlPanel(message, item, theEvent, theDialog, refCon);
			break;
	}
	return refCon;
}

long CallControlPanel(short message, short item, EventRecord *theEvent, DialogPtr theDialog, long refCon)
{
	CControlPanel *thePanel;
	
	if (refCon == cdevUnset) return cdevGenErr;
	
	thePanel = (CControlPanel *)refCon;
	thePanel->SetArguments(item, theEvent, theDialog);
	refCon = thePanel->DispatchMessage(message);
	switch (refCon)
	{
		case cdevGenErr:
		case cdevMemErr:
		case cdevResErr:
			delete thePanel;
		default:
			break;
	}
	return refCon;
}

Boolean CanPanelRun(void)
{
	SysEnvRec	theWorld;
	
	SysEnvirons(curSysEnvVers, &theWorld);
	
	if (theWorld.systemVersion < 0x0700) return false;
	return true;
}