#pragma once

#include <Constants.h>
#include <CPane.h>

#define kSICNWidth  SIZEBOX_LENGTH
#define kSICNHeight SIZEBOX_LENGTH

struct CSICNPane : CPane
{
	short fId;
	short fIndex;

	void ISICNPane
		( CView *anEnclosure
		, CBureaucrat *aSupervisor
		, short aHEncl
		, short aVEncl
		, SizingOption aHSizing
		, SizingOption aVSizing
		, short SICNid
		, short SICNindex );

	virtual void Draw( Rect *area );
};
