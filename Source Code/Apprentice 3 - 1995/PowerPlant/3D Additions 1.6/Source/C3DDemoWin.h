// ===========================================================================
//	C3DDemoWin.h
// ===========================================================================

#pragma once

#include <LWindow.h>

class C3DDemoWin : public LWindow {

public:
	enum { class_ID = '3Dwn' };
	
	static C3DDemoWin*	CreateFromStream(LStream *inStream);

					C3DDemoWin(LStream *inStream);
					C3DDemoWin();
					~C3DDemoWin();

protected:
	virtual void	FinishCreateSelf();
	virtual void	DrawSelf();
};
