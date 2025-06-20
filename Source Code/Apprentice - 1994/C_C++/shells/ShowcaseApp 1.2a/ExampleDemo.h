#pragma once


#include "CSApplication.h"


struct CExamplePane : CPane
{
	void IExamplePane
		( CView *anEnclosure
		, CBureaucrat *aSupervisor
		, short aWidth
		, short aHeight
		, short aHEncl
		, short aVEncl
		, SizingOption aHSizing
		, SizingOption aVSizing );
	
	virtual void Draw( Rect *area );
};


struct CExampleDemoDir : CShowcaseDemoDir
{
	CExamplePane *itsPane;

	virtual void INewDemo( CDirectorOwner *aSupervisor);
};
