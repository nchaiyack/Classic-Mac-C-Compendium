#include "UGWorld.h"
#include "LWindow.h"

class PixMapWindow : public LWindow {

public:
	static	PixMapWindow* CreatePixMapWindowStream(LStream *inStream);
			PixMapWindow(LGWorld *inGWorld, Rect frame, Boolean inScale);
			PixMapWindow(LGWorld *inGWorld, Boolean inScale);
			PixMapWindow(LStream *inStream);
	virtual	~PixMapWindow();

	void	SetmGWorld(LGWorld *theGWorld);
	inline	LGWorld	*GetmGWorld();

private:
	LGWorld	*mGWorld;
	int		mScaleToWindow;
	virtual void	DrawSelf();
	
};
