#ifndef __CCONTROLPANEL__
#define __CCONTROLPANEL__
#pragma once

#ifndef __CLISTMANAGER__
#include "CListManager.h"
#endif

#ifndef __EVENTS__
#include <Events.h>
#endif

#ifndef __DIALOGS__
#include <Dialogs.h>
#endif

class CControlPanel : public SingleObject
{
	public:
	
				CControlPanel(short numItems);
				~CControlPanel();
		
		void	SetArguments(short item, EventRecord *theEvent, DialogPtr theDialog);
		long	DispatchMessage(short theMessage);

	protected:
		enum		// dialog items
		{
			kAddButton = 1,
			kListBox,
			kRemoveButton,
			kPrefsName = 0,
			kChooseDialog = -4033
		};
		
		long	DoInit();
		long	DoHit();
		long	DoClose();
		long	DoUpdate();
		long	DoActivate();
		long	DoDeactivate();
		long	DoKey();
		
		long	ReadPreferencesIntoList();
		void	InsertItems();
		void	SetupDialog();
		void	SavePreferences();
		void	RemovePreferences();
		
		CListManager	*fTheList;		// the list of items 2 b masked
		EventRecord		*fTheEvent;
		DialogPtr		fTheDialog;
		short			fItemHit;
		short			fNumItems;
};

#endif