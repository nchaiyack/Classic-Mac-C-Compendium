//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEnhancedWindow.h
//|
//| This file contains the interface to an enhanced window.  This window
//| is more aware of multiple screens that a normal window, and can place
//| and zoom itself intelligently depending on the screen setup.
//|___________________________________________________________________________



class CWindow;

class CEnhancedWindow : public CWindow
	{

private:

	virtual void FindIdealSize(Rect *rect);

protected:

	short title_bar_height;

public:
	
	void IEnhancedWindow(short WINDid, Boolean aFloating,
						CDesktop *anEnclosure, CDirector *aSupervisor);

	virtual void Zoom(short zoomState);
	virtual void PlaceWithVerify(Rect *bounds);
	virtual void Place(Rect *bounds);
	virtual void GetRect(Rect *window_rect);
	virtual void MakeFullScreen(void);
	
	};


