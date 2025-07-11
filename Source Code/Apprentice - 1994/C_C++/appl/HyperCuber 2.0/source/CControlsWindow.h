//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CControlsWindow.h
//|
//| This file contains the interface to a controls window.
//|___________________________________________________________________________

#include "CEnhancedWindow.h"

class CControlsWindow : public CEnhancedWindow
	{
	
public:

	short dimension;
	
	void IControlsWindow(short WINDid, Boolean aFloating,
						CDesktop *anEnclosure, CDirector *aSupervisor, short dim);

	virtual void Move(short h, short v);
	virtual void HideFloat(void);
	virtual void ShowFloat(void);
	virtual void Drag(EventRecord *macEvent);
	
	};


