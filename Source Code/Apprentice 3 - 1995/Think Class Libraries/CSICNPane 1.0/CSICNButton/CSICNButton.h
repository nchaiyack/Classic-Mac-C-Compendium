#pragma once

#include "CSICNPane.h"

struct CSICNButton : CSICNPane
{
	long  fCommand;

	void ISICNButton
		( CView *anEnclosure
		, CBureaucrat *aSupervisor
		, short aHEncl
		, short aVEncl
		, SizingOption aHSizing
		, SizingOption aVSizing
		, short SICNid
		, short SICNindex );

	virtual void DoClick( Point hitPt, short modifierKeys, long when );
	virtual void DoGoodClick( short whichPart );
	virtual void SetClickCmd( long command );
	virtual long GetClickCmd( void );
};
