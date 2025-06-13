#pragma once

/*
	StdControlPanel
	©1995 Chris K. Thomas.  All Rights Reserved.
	
	Safe Control Panel class.
*/


class StdControlPanel
{
protected:
	EventRecord	mLastEvent;
	DialogPtr	mOurDialog;
	
public:
				StdControlPanel(DialogPtr inCPDialog);
	virtual		~StdControlPanel();
	
	// * events
	
	virtual void Update() {}
	virtual void Click() {}
	virtual void Activate() {}
	virtual void Deactivate() {}
	virtual void KeyDown() {}
	virtual void Idle() {}
	
	// * menu events
	
	virtual void Undo() {}
	virtual void Cut() {}
	virtual void Copy() {}
	virtual void Paste() {}
	virtual void Clear() {}
	
	void SetLastEvent(EventRecord *inEvent)
	{
		if(inEvent)
			mLastEvent = *inEvent;
	}
};

StdControlPanel *CreateControlPanel(DialogPtr inDialog);	//user must provide!