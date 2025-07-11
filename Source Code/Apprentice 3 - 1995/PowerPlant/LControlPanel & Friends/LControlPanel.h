#pragma once

/*
	LControlPanel
	
	PowerPlant-based control panel.
	�1995 Chris K. Thomas.  All Rights Reserved.
*/

#include "StdControlPanel.h"

class LGrafPortView;

class LControlPanel : public StdControlPanel
{
protected:
	LGrafPortView	*mOverView;
	
public:
			LControlPanel(DialogPtr inCPDialog);
	virtual	~LControlPanel();
	
	virtual void Update();
	virtual void Click();
	virtual void Activate();
	virtual void Deactivate();
	virtual void KeyDown();
	virtual void Idle();
};

